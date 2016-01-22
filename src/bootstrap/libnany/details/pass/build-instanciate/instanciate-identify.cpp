#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	namespace // anonymous
	{

		static inline bool debugResolveListOnlyContainsFunc(const std::vector<std::reference_wrapper<Atom>>& list)
		{
			for (auto& atom: list)
			{
				if (not atom.get().isFunction())
					return true;
			}
			return false;
		}

	} // anonymous namespace




	inline bool SequenceBuilder::identify(const IR::ISA::Operand<IR::ISA::Op::identify>& operands)
	{
		auto& frame = atomStack.back();
		AnyString name = currentSequence.stringrefs[operands.text];

		// keeping traces of the code logic
		frame.lvids[operands.lvid].resolvedName = name;
		frame.lvids[operands.lvid].referer = operands.self;


		if (name == '=') // it is an assignment, not a real method call
		{
			// remember this special case
			frame.lvids[operands.lvid].isAssignment = true;
			// for consistency checks, after transformations on the AST, '=' should be a method call
			// we should have something like: 'foo.=(rhs)'
			if (unlikely(0 == operands.self))
				return complainInvalidSelfRefForVariableAssignment(operands.lvid);
			return true;
		}

		if (operands.self != 0)
		{
			if (not frame.verify(operands.self))
				return false;

			if (frame.lvids[operands.self].isAssignment)
			{
				// since self was marked as an 'assignment', we're trying to resolve here '^()'
				if (unlikely(name != "^()"))
				{
					ICE() << "invalid resolve name for assignment (got '" << name << "')";
					return false;
				}

				// remember this special case
				frame.lvids[operands.lvid].isAssignment = true;
				return true;
			}
		}

		auto& cdef = cdeftable.classdef(CLID{frame.atomid, operands.lvid});

		// checking if the lvid does not map to a parameter, which  must
		// have already be resolved when instanciating the function
		assert(cdef.clid.lvid() >= 2 + frame.atom.parameters.size());
		if (unlikely(cdef.clid.lvid() < 2 + frame.atom.parameters.size()))
		{
			String errmsg;
			errmsg << CLID{frame.atomid, operands.lvid} << ": should be alreayd resolved";
			return complainOperand(reinterpret_cast<const IR::Instruction&>(operands), errmsg);
		}

		// list of all possible atoms when resolving 'name'
		multipleResults.clear();
		// Self, if any
		Atom* selfAtom = nullptr;
		// local variable ?
		bool isLocalVariable = false;

		if (0 == operands.self)
		{
			// simple variable, function, namespace...

			// checking first for builtin identifiers (void, any...)
			switch (name[0])
			{
				case 'a':
				{
					if (name == "any") // any - nothing to resolve
					{
						multipleResults.clear();
						frame.lvids[operands.lvid].markedAsAny = true;
						frame.resolvePerCLID[cdef.clid].clear();
						cdeftable.substitute(operands.lvid).mutateToAny();
						return true;
					}
					break;
				}
				case 'n':
				{
					if (name == "null")
					{
						multipleResults.clear();
						frame.resolvePerCLID[cdef.clid].clear(); // just in case

						auto& opc = cdeftable.substitute(operands.lvid);
						opc.mutateToBuiltin(nyt_pointer);
						opc.qualifiers.ref = false;
						out.emitStore_u64(operands.lvid, 0);
						return true;
					}
					break;
				}
				case 'v':
				{
					if (name == "void")
					{
						multipleResults.clear();
						frame.resolvePerCLID[cdef.clid].clear();
						cdeftable.substitute(operands.lvid).mutateToVoid();
						return true;
					}
					break;
				}
				case '_':
				{
					if (name[1] == '_')
					{
						multipleResults.clear();
						frame.resolvePerCLID[cdef.clid].clear(); // just in case

						if (name == "__false")
						{
							auto& opc = cdeftable.substitute(operands.lvid);
							opc.mutateToBuiltin(nyt_bool);
							opc.qualifiers.ref = false;
							out.emitStore_u64(operands.lvid, 0);
							return true;
						}
						if (name == "__true")
						{
							auto& opc = cdeftable.substitute(operands.lvid);
							opc.mutateToBuiltin(nyt_bool);
							opc.qualifiers.ref = false;
							out.emitStore_u64(operands.lvid, 1);
							return true;
						}

						nytype_t type = nany_cstring_to_type_n(name.c_str(), name.size());
						if (unlikely(type == nyt_void))
							return complainUnknownBuiltinType(name);

						cdeftable.substitute(operands.lvid).mutateToBuiltin(type);
						return true;
					}
					break;
				}
			}

			// trying for local variables first
			LVID lvidVar = frame.findLocalVariable(name);
			if (lvidVar != 0)
			{
				// the variable is used, whatever it is (error or not)
				frame.lvids[lvidVar].hasBeenUsed = true;
				frame.lvids[operands.lvid].alias = lvidVar;

				if (not frame.verify(lvidVar)) // suppress spurious errors from previous ones
					return false;

				// acquire the variable
				if (canGenerateCode())
					out.emitStore(operands.lvid, lvidVar);

				auto& varcdef = cdeftable.classdef(CLID{frame.atomid, lvidVar});
				if (not varcdef.isBuiltin())
				{
					auto* varAtom = cdeftable.findClassdefAtom(varcdef);
					if (unlikely(varAtom == nullptr))
					{
						ICE() << "invalid atom for local scope variable. clid: " << CLID{frame.atomid, lvidVar}
							<< ", " << (uint32_t) varcdef.kind;
						return false;
					}
					multipleResults.emplace_back(std::ref(*varAtom));
					isLocalVariable = true;
				}
				else
				{
					// special case - not an atom
					auto& spare = cdeftable.substitute(cdef.clid.lvid());
					spare.mutateToBuiltin(varcdef.kind);
					spare.qualifiers.ref = false;
					return true;
				}
			}
			else
			{
				if (not frame.atom.performNameLookupOnChildren(multipleResults, name))
				{
					if (frame.atom.parent)
						frame.atom.parent->performNameLookupFromParent(multipleResults, name);
				}
			}
		}
		else
		{
			// self.<something to identify>
			if (unlikely(frame.lvids[operands.lvid].markedAsAny))
			{
				ICE() << "can not perform member lookup on 'any'";
				return false;
			}

			auto& self = cdeftable.classdef(CLID{frame.atomid, operands.self});
			if (unlikely(self.isBuiltinOrVoid()))
				return complainInvalidMemberRequestNonClass(name, self.kind);


			selfAtom = cdeftable.findClassdefAtom(self);
			if (selfAtom != nullptr) // the parent has been fully resolved
			{
				// since the parent has been fully resolved, no multiple
				// solution should be available
				assert(frame.resolvePerCLID[self.clid].empty());

				selfAtom->performNameLookupOnChildren(multipleResults, name);
			}
			else
			{
				auto& selfSolutions = frame.resolvePerCLID[self.clid];
				multipleResults.reserve(selfSolutions.size());
				for (auto& atomElement: selfSolutions)
					atomElement.get().performNameLookupOnChildren(multipleResults, name);
			}
		}


		switch (multipleResults.size())
		{
			case 1: // unique match count
			{
				auto& atom = multipleResults[0].get();
				if (unlikely(atom.hasErrors))
					return false;

				// if the resolution is simple (aka only one solution), it is possible that the
				// solution is a member variable. In this case, the atom will be the member itself
				// and not its real type
				if (atom.isMemberVariable())
				{
					assert(not isLocalVariable and "a member variable cannot be a local variable");

					// member variable - the real type is held by 'returnType'
					auto& cdefvar = cdeftable.classdef(atom.returnType.clid);
					auto* atomvar = cdeftable.findClassdefAtom(cdefvar);
					if (unlikely(not (atomvar or cdefvar.isBuiltin())))
						return (ICE() << "invalid variable member type for " << atom.printFullname());

					auto& spare = cdeftable.substitute(operands.lvid);
					spare.import(cdefvar);
					if (atomvar)
						spare.mutateToAtom(atomvar);

					uint32_t self = operands.self;
					if (self == 0) // implicit 'self'
					{
						// retrieving the local self
						if (unlikely(not frame.atom.isClassMember()))
							return (ICE() << "invalid 'self' object");
						self = 2; // 1: return type, 2: self parameter
					}

					auto& origin  = frame.lvids[operands.lvid].origin.varMember;
					assert(self != 0);
					assert(atom.atomid != 0);
					origin.self   = self;
					origin.atomid = atom.atomid;
					origin.field  = atom.varinfo.effectiveFieldIndex;

					if (canGenerateCode())
					{
						// read the address
						out.emitFieldget(operands.lvid, self, atom.varinfo.effectiveFieldIndex);
						tryToAcquireObject(operands.lvid, cdefvar);
					}
				}
				else
				{
					// override the typeinfo
					auto& spare = cdeftable.substitute(cdef.clid.lvid());
					spare.import(cdef);
					spare.mutateToAtom(&atom);

					if (isLocalVariable)
					{
						// disable optimisation to avoid unwanted behavior
						auto& origin = frame.lvids[operands.lvid].origin;
						origin.memalloc = false;
						origin.returnedValue = false;

						if (canGenerateCode())
							acquireObject(operands.lvid);
					}
				}

				// instanciating the type itself, to resolve member variables
				if (atom.isClass() and not atom.classinfo.isInstanciated)
					return instanciateAtomClass(atom);
				return true;
			}

			default: // multiple solutions
			{
				// checking integrity (debug only) - verifying that all results are functions
				if (debugmode)
				{
					if (unlikely(debugResolveListOnlyContainsFunc(multipleResults)))
						return complainOperand((const IR::Instruction&) operands, "resolve-list contains something else than functions");
				}

				// multiple solutions are possible (probably for a func call)
				// keeping the solutions for later resolution by the real func call
				// (with parameters to find the most appropriate one)
				frame.resolvePerCLID[cdef.clid].swap(multipleResults);
				return true;
			}

			case 0: // no identifier found from 'atom map'
			{
				return complainUnknownIdentifier(selfAtom, frame.atom, name);
			}
		}
		return false;
	}



	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::identify>& operands)
	{
		assert(not atomStack.empty());

		bool ok = identify(operands);
		if (unlikely(not ok))
			atomStack.back().invalidate(operands.lvid);
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
