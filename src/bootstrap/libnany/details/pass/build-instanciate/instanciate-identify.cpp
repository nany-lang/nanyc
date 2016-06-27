#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	Atom& SequenceBuilder::resolveTypeAlias(Atom& original, const Classdef*& resultcdef)
	{
		assert(original.isTypeAlias());

		// trying a direct resolution
		auto cdef = std::cref(cdeftable.classdef(original.returnType.clid));

		std::unordered_set<uint32_t> encountered; // to avoid circular references
		Atom* alias = nullptr;
		do
		{
			if (cdef.get().isBuiltin()) // gotcha !
			{
				resultcdef = &(cdef.get());
				return original;
			}

			// current atom to check
			alias = cdeftable.findClassdefAtom(cdef.get());
			if (unlikely(!alias))
				break;

			if ((alias->parent == original.parent) and alias->atomid > original.atomid)
			{
				// same parent but declared after (the atomid is likely to be greater
				// than the first one since registered after)
				complainTypedefDeclaredAfter(original, *alias);
				break;
			}

			if (not alias->isTypeAlias()) // gotcha !
			{
				resultcdef = &(cdef.get());
				return *alias;
			}

			// checking for circular aliases
			if (not encountered.insert(alias->atomid).second)
			{
				// circular reference
				complainTypealiasCircularRef(original, *alias);
				break;
			}

			cdef = std::cref(cdeftable.classdef(alias->returnType.clid));
		}
		while (alias != nullptr);

		complainTypedefUnresolved(original);
		resultcdef = nullptr;
		return original;
	}




	bool SequenceBuilder::identify(const IR::ISA::Operand<IR::ISA::Op::identify>& operands,
		const AnyString& name, bool firstChance)
	{
		// keeping traces of the code logic
		frame->lvids[operands.lvid].resolvedName = name;
		frame->lvids[operands.lvid].referer = operands.self;


		if (name == '=') // it is an assignment, not a real method call
		{
			// remember this special case
			frame->lvids[operands.lvid].pointerAssignment = true;
			// for consistency checks, after transformations on the AST, '=' should be a method call
			// we should have something like: 'foo.=(rhs)'
			if (unlikely(0 == operands.self))
				return complainInvalidSelfRefForVariableAssignment(operands.lvid);
			return true;
		}

		if (operands.self != 0)
		{
			if (not frame->verify(operands.self))
				return false;

			if (frame->lvids[operands.self].pointerAssignment)
			{
				// since self was marked as an 'assignment', we're trying to resolve here '^()'
				if (unlikely(name != "^()"))
				{
					ice() << "invalid resolve name for assignment (got '" << name << "')";
					return false;
				}

				// remember this special case
				frame->lvids[operands.lvid].pointerAssignment = true;
				return true;
			}
		}

		auto& cdef = cdeftable.classdef(CLID{frame->atomid, operands.lvid});

		// checking if the lvid does not map to a parameter, which  must
		// have already be resolved when instanciating the function
		if (frame->atom.isFunction())
		{
			assert(cdef.clid.lvid() >= 2 + frame->atom.parameters.size());
			if (unlikely(cdef.clid.lvid() < 2 + frame->atom.parameters.size()))
			{
				String errmsg;
				errmsg << CLID{frame->atomid, operands.lvid} << ": should be alreayd resolved";
				return complainOperand(IR::Instruction::fromOpcode(operands), errmsg);
			}
		}

		// list of all possible atoms when resolving 'name'
		assert(multipleResults.empty());
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
						frame->lvids[operands.lvid].markedAsAny = true;
						frame->partiallyResolved.erase(cdef.clid);
						cdeftable.substitute(operands.lvid).mutateToAny();
						return true;
					}
					break;
				}
				case 'n':
				{
					if (name == "null")
					{
						frame->partiallyResolved.erase(cdef.clid); // just in case

						auto& opc = cdeftable.substitute(operands.lvid);
						opc.mutateToBuiltin(nyt_ptr);
						opc.qualifiers.ref = false;
						out.emitStore_u64(operands.lvid, 0);
						frame->lvids[operands.lvid].synthetic = false;
						return true;
					}
					break;
				}
				case 'v':
				{
					if (name == "void")
					{
						frame->partiallyResolved.erase(cdef.clid);
						cdeftable.substitute(operands.lvid).mutateToVoid();
						return true;
					}
					break;
				}
				case '_':
				{
					if (name.size() > 1 and name[1] == '_')
					{
						frame->partiallyResolved.erase(cdef.clid); // just in case

						if (name == "__false")
						{
							auto& opc = cdeftable.substitute(operands.lvid);
							opc.mutateToBuiltin(nyt_bool);
							opc.qualifiers.ref = false;
							out.emitStore_u64(operands.lvid, 0);
							frame->lvids[operands.lvid].synthetic = false;
							return true;
						}
						if (name == "__true")
						{
							auto& opc = cdeftable.substitute(operands.lvid);
							opc.mutateToBuiltin(nyt_bool);
							opc.qualifiers.ref = false;
							out.emitStore_u64(operands.lvid, 1);
							frame->lvids[operands.lvid].synthetic = false;
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
			LVID lvidVar = frame->findLocalVariable(name);
			if (lvidVar != 0)
			{
				// the variable is used, whatever it is (error or not)
				frame->lvids[lvidVar].hasBeenUsed = true;
				frame->lvids[operands.lvid].alias = lvidVar;
				frame->lvids[operands.lvid].synthetic = false;

				if (not frame->verify(lvidVar)) // suppress spurious errors from previous ones
					return false;

				// acquire the variable
				if (canGenerateCode())
					out.emitStore(operands.lvid, lvidVar);

				auto& varcdef = cdeftable.classdef(CLID{frame->atomid, lvidVar});
				if (not varcdef.isBuiltin())
				{
					auto* varAtom = cdeftable.findClassdefAtom(varcdef);
					if (unlikely(varAtom == nullptr))
					{
						if (varcdef.isVoid())
						{
							cdeftable.substitute(cdef.clid.lvid()).mutateToVoid();
							return true;
						}

						ice() << "invalid atom for local scope variable. clid: " << CLID{frame->atomid, lvidVar}
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
				if (not frame->atom.performNameLookupOnChildren(multipleResults, name))
				{
					if (frame->atom.parent)
						frame->atom.parent->performNameLookupFromParent(multipleResults, name);
				}
			}
		}
		else
		{
			assert(frame->verify(operands.self));
			// self.<something to identify>
			if (unlikely(frame->lvids[operands.lvid].markedAsAny))
			{
				ice() << "can not perform member lookup on 'any'";
				return false;
			}

			auto& self = cdeftable.classdef(CLID{frame->atomid, operands.self});
			if (unlikely(self.isBuiltinOrVoid()))
				return complainInvalidMemberRequestNonClass(name, self.kind);

			bool& singleHop = frame->lvids[operands.self].singleHopForReferer;

			selfAtom = cdeftable.findClassdefAtom(self);
			if (selfAtom != nullptr) // the parent has been fully resolved
			{
				// since the parent has been fully resolved, no multiple
				// solution should be available
				assert(frame->partiallyResolved.count(self.clid) == 0
					   or frame->partiallyResolved[self.clid].empty());

				selfAtom->performNameLookupOnChildren(multipleResults, name, &singleHop);
			}
			else
			{
				auto it = frame->partiallyResolved.find(self.clid);
				if (it != frame->partiallyResolved.end())
				{
					auto& selfSolutions = it->second;
					multipleResults.reserve(selfSolutions.size());
					for (auto& atomE: selfSolutions)
						atomE.get().performNameLookupOnChildren(multipleResults, name, &singleHop);
				}
			}
		}


		switch (multipleResults.size())
		{
			case 1: // unique match count
			{
				auto& resultAtom = multipleResults[0].get();
				const Classdef* cdefTypedef = nullptr;
				auto& atom = (not resultAtom.isTypeAlias())
					? resultAtom
					: resolveTypeAlias(resultAtom, cdefTypedef);

				if (unlikely((!cdefTypedef and resultAtom.isTypeAlias()) or atom.flags(Atom::Flags::error)))
					return false;

				if (unlikely(cdefTypedef and cdefTypedef->isBuiltin()))
				{
					auto& spare = cdeftable.substitute(cdef.clid.lvid());
					spare.import(*cdefTypedef);

					if (isLocalVariable)
					{
						// disable optimisation to avoid unwanted behavior
						auto& lvidinfo = frame->lvids[operands.lvid];
						lvidinfo.synthetic = false;
						lvidinfo.origin.memalloc = false;
						lvidinfo.origin.returnedValue = false;
					}
					return true;
				}

				// if the resolution is simple (aka only one solution), it is possible that the
				// solution is a member variable (`self.myvar`). In this case, the atom will be the member itself
				// and not its real type
				if (atom.isMemberVariable())
				{
					assert(not isLocalVariable and "a member variable cannot be a local variable");
					assert(not atom.returnType.clid.isVoid());

					// member variable - the real type is held by 'returnType'
					auto& cdefvar = cdeftable.classdef(atom.returnType.clid);
					auto* atomvar = (not cdefvar.isBuiltin()) ? cdeftable.findClassdefAtom(cdefvar) : nullptr;
					if (unlikely(!atomvar and not cdefvar.isBuiltin()))
						return (ice() << "invalid variable member type for " << atom.fullname());

					auto& spare = cdeftable.substitute(operands.lvid);
					spare.import(cdefvar);
					if (atomvar)
						spare.mutateToAtom(atomvar);

					uint32_t self = operands.self;
					if (self == 0) // implicit 'self' ?
					{
						if (frame->atom.isClassMember())
						{
							// 'self' is given by the first parameter
							self = 2; // 1: return type, 2: first parameter
						}
						else
						{
							// no 'self' available since it just does not exist, which can be expected
							// for type resolution (the type resolution is done directly from the atom class,
							// where the initialization is done via a proxy function)
							// It's ok for type resolution since we already know we're dealing with a variable member)
							if (frame->atom.isClass() and (not canGenerateCode()))
							{
								// 'self' can stay null
							}
							else
							{
								ice() << "identify: invalid 'self' object for '" << name << "' from '"
									<< frame->atom.caption() << '\'';
								return false;
							}
						}
					}

					auto& lvidinfo = frame->lvids[operands.lvid];
					lvidinfo.synthetic = false;

					auto& origin  = lvidinfo.origin.varMember;
					assert(atom.atomid != 0);
					origin.self   = self;
					origin.atomid = atom.atomid;
					origin.field  = atom.varinfo.effectiveFieldIndex;

					if (canGenerateCode())
					{
						// read the address
						assert(self != 0 and "'self can be null only for type resolution'");
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
						auto& lvidinfo = frame->lvids[operands.lvid];
						lvidinfo.synthetic = false;
						lvidinfo.origin.memalloc = false;
						lvidinfo.origin.returnedValue = false;

						if (canGenerateCode())
							acquireObject(operands.lvid);
					}
				}
				return true;
			}

			default: // multiple solutions
			{
				// multiple solutions are possible (probably for a func call)
				// keeping the solutions for later resolution by the real func call
				// (with parameters to find the most appropriate one)
				frame->partiallyResolved[cdef.clid].swap(multipleResults);
				return true;
			}

			case 0: // no identifier found from 'atom map'
			{
				// nothing has been found
				// trying capturing variable from anonymous classes
				if (firstChance)
				{
					if (frame->atom.canCaptureVariabes())
					{
						if (identifyCapturedVar(operands, name))
							return true;
					}

					// UNKNOWN identifier
					if (debugmode)
					{
						auto err = (error() << "debug: failed identify '" << name);
						err << "' from atom: " << frame->atomid << " aka '" << frame->atom.caption();
						err << "', self: %" << operands.self;
						err << ", lvid: %" << operands.lvid;
						if (unlikely(not firstChance))
							err << " (SECOND TRY - ICE!)";
					}
					return complainUnknownIdentifier(selfAtom, frame->atom, name);
				}
				break;
			}
		}
		return false;
	}



	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::identify>& operands)
	{
		assert(frame != nullptr);

		AnyString name = currentSequence.stringrefs[operands.text];
		bool ok = identify(operands, name);
		if (unlikely(not ok))
			frame->invalidate(operands.lvid);
		multipleResults.clear();
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
