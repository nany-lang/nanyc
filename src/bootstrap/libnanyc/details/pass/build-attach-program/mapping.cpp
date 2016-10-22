#include "mapping.h"
#include "details/atom/classdef-table-view.h"
#include "details/ir/sequence.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Mapping
{

	namespace {


	//! Stack frame per atom definition (class, func)
	struct AtomStackFrame final
	{
		AtomStackFrame(Atom& atom, std::unique_ptr<AtomStackFrame>& next)
			: atom(atom), next(std::move(next))
		{}
		//! The current atom
		Atom& atom;
		//! The current scope depth for the current stack frame
		uint32_t scope = 0u;
		uint32_t parameterIndex = 0u;
		//! Convenient classdefs alias
		std::vector<CLID> classdefs;
		// Next frame
		std::unique_ptr<AtomStackFrame> next;

		//! Information for capturing variables
		struct CaptureVariables {
			//! Get if allowed to capture variables
			bool enabled() const { return atom != nullptr; }

			void enabled(Atom* newatom)
			{
				atom = newatom;
				atom->flags += Atom::Flags::captureVariables;
				if (!newatom->candidatesForCapture)
				{
					typedef decltype(newatom->candidatesForCapture) Set;
					newatom->candidatesForCapture = std::make_unique<Set::element_type>();
				}
			}

			//! Try to list of all unknown identifiers, potential candidates for capture
			Atom* atom = nullptr;
			//! all local named variables (name / scope)
			std::unordered_map<AnyString, uint32_t> knownVars;
		}
		capture;

		Atom& currentAtomNotUnit() { return (not atom.isUnit()) ? atom : (*(atom.parent)); }

	}; // class AtomStackFrame




	struct OpcodeReader final
	{
		OpcodeReader(SequenceMapping& mapping)
			: cdeftable(mapping.cdeftable)
			, mutex(mapping.mutex)
			, currentSequence(mapping.currentSequence)
			, firstAtomCreated(mapping.firstAtomCreated)
			, prefixNameForFirstAtomCreated{mapping.prefixNameForFirstAtomCreated}
			, evaluateWholeSequence(mapping.evaluateWholeSequence)
			, mapping(mapping)
		{
			firstAtomCreated = nullptr;
			lastPushedNamedParameters.reserve(8); // arbitrary
			lastPushedIndexedParameters.reserve(8);
		}


		void complainOperand(const IR::Instruction& operands, AnyString msg)
		{
			// example: ICE: unknown opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
			auto trace = ice();
			if (not msg.empty())
				trace << msg << ':';
			else
				trace << "invalid opcode ";

			trace << " '" << IR::ISA::print(currentSequence, operands) << '\'';
			success = false;
		}


		template<IR::ISA::Op O>
		void complainOperand(const IR::ISA::Operand<O>& operands, AnyString msg)
		{
			complainOperand(IR::Instruction::fromOpcode(operands), msg);
		}


		template<IR::ISA::Op O>
		bool checkForLVID(const IR::ISA::Operand<O>& operands, LVID lvid)
		{
			if (debugmode)
			{
				if (unlikely(lvid == 0 or not (lvid < atomStack->classdefs.size())))
				{
					complainOperand(operands, String{"mapping: invalid lvid %"} << lvid
						<< " (upper bound: %" << atomStack->classdefs.size() << ')');
					return false;
				}
			}
			return true;
		}


		void pushNewFrame(Atom& atom)
		{
			std::unique_ptr<AtomStackFrame> next{atomStack.release()};
			atomStack = std::make_unique<AtomStackFrame>(atom, next);
		}


		void resetClassdefOriginFromCurrentPosition(Classdef& cdef)
		{
			cdef.origins.filename = currentFilename;
			cdef.origins.line     = currentLine;
			cdef.origins.offset   = currentOffset;
		}


		void attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.ptr2func)))
				return;
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;

			auto& stackframe     = *atomStack;
			auto  atomid         = stackframe.atom.atomid;
			auto& clidFuncToCall = stackframe.classdefs[operands.ptr2func];
			auto& clidRetValue   = stackframe.classdefs[operands.lvid];

			// update all underlying types
			{
				CLID clid{atomid, 0};

				MutexLocker locker{mutex};

				resetClassdefOriginFromCurrentPosition(cdeftable.classdef(clidRetValue));
				auto& funcdef = cdeftable.addClassdefInterfaceSelf(clidFuncToCall);
				funcdef.resetReturnType(clidRetValue);
				funcdef.clid = clidFuncToCall;

				for (uint i = 0; i != lastPushedIndexedParameters.size(); ++i)
				{
					auto lvid = lastPushedIndexedParameters[i];
					clid.reclass(lvid);
					funcdef.appendParameter(clid);

					auto& followup = cdeftable.classdef(clid).followup;
					followup.pushedIndexedParams.push_back(std::make_pair(clidFuncToCall, i));
				}

				for (auto& pair: lastPushedNamedParameters)
				{
					clid.reclass(pair.second);
					funcdef.appendParameter(pair.first, clid);

					auto& followup = cdeftable.classdef(clid).followup;
					followup.pushedNamedParams.push_back(std::make_pair(clidFuncToCall, pair.first));
				}
			}
		}


		void mapBlueprintFuncdefOrTypedef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			// functions and typedef are instanciated the sameway (with some minor differences)
			auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
			bool isFuncdef = (kind == IR::ISA::Blueprint::funcdef);

			// registering the blueprint into the outline...
			Atom& atom = atomStack->currentAtomNotUnit();
			AnyString funcname = currentSequence.stringrefs[operands.name];
			if (unlikely(funcname.empty()))
				return complainOperand(operands, (isFuncdef) ? "invalid func name" : "invalid typedef name");

			// reset last lvid and parameters
			lastLVID = 0;
			lastPushedNamedParameters.clear();
			lastPushedIndexedParameters.clear();

			// global func operators (or unittest) always belong to root, even if declared in a specific namespace
			bool isGlobalOperator = isFuncdef and atom.type == Atom::Type::namespacedef
				and funcname[0] == '^'
				and (not funcname.startsWith("^view^"))
				and (not funcname.startsWith("^prop"));

			auto& parentAtom = (not isGlobalOperator)
				? atom
				: cdeftable.atoms.root;

			MutexLocker locker{mutex};
			// create a new atom in the global type table
			auto* newatom = isFuncdef
				? cdeftable.atoms.createFuncdef(parentAtom, funcname)
				: cdeftable.atoms.createTypealias(parentAtom, funcname);

			assert(newatom != nullptr);
			newatom->opcodes.sequence = &currentSequence;
			newatom->opcodes.offset   = currentSequence.offsetOf(operands);
			// create a pseudo classdef to easily retrieve the real atom from a clid
			cdeftable.registerAtom(newatom);

			operands.atomid = newatom->atomid;

			// return type
			newatom->returnType.clid.reclass(newatom->atomid, 1);
			// scope resolution
			if (parentAtom.atomid != atom.atomid)
				newatom->scopeForNameResolution = &atom;

			// requires additional information
			needAtomDbgFileReport = true;
			needAtomDbgOffsetReport = true;
			pushNewFrame(*newatom);

			// capture unknown variables ?
			if (not isGlobalOperator)
			{
				if (newatom->isClassMember() and newatom->parent->flags(Atom::Flags::captureVariables))
				{
					atomStack->capture.enabled(newatom->parent);
					newatom->flags += Atom::Flags::captureVariables;
				}
			}

			if (!firstAtomCreated)
				firstAtomCreated = newatom;
		}


		void mapBlueprintClassdef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			// registering the blueprint into the outline...
			Atom& atom = atomStack->currentAtomNotUnit();

			// reset last lvid and parameters
			lastLVID = 0;
			lastPushedNamedParameters.clear();
			lastPushedIndexedParameters.clear();

			AnyString classname = currentSequence.stringrefs[operands.name];

			Atom* newClassAtom = nullptr;
			// create a new atom in the global type table
			if (prefixNameForFirstAtomCreated.empty())
			{
				MutexLocker locker{mutex};
				newClassAtom = cdeftable.atoms.createClassdef(atom, classname);
				// create a pseudo classdef to easily retrieve the real atom from a clid
				cdeftable.registerAtom(newClassAtom);
			}
			else
			{
				String tmpname;
				tmpname.reserve(classname.size() + prefixNameForFirstAtomCreated.size());
				tmpname << prefixNameForFirstAtomCreated << classname;
				prefixNameForFirstAtomCreated.clear();

				MutexLocker locker{mutex};
				newClassAtom = cdeftable.atoms.createClassdef(atom, tmpname);
				// create a pseudo classdef to easily retrieve the real atom from a clid
				cdeftable.registerAtom(newClassAtom);
			}

			assert(newClassAtom != nullptr);
			newClassAtom->opcodes.sequence = &currentSequence;
			newClassAtom->opcodes.offset   = currentSequence.offsetOf(operands);

			// update atomid
			operands.atomid = newClassAtom->atomid;
			// requires additional information
			needAtomDbgFileReport = true;
			needAtomDbgOffsetReport = true;
			pushNewFrame(*newClassAtom);

			if (operands.lvid != 0)
				atomStack->capture.enabled(newClassAtom);

			if (!firstAtomCreated)
				firstAtomCreated = newClassAtom;
		}


		void mapBlueprintParam(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			auto& frame = *atomStack;

			// calculating the lvid for the current parameter
			// (+1 since %1 is the return value/type)
			uint paramLVID = (++frame.parameterIndex) + 1;

			if (unlikely(not checkForLVID(operands, paramLVID)))
				return;

			auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
			assert(kind == IR::ISA::Blueprint::param or kind == IR::ISA::Blueprint::paramself
				   or kind == IR::ISA::Blueprint::gentypeparam);

			bool isGenTypeParam = (kind == IR::ISA::Blueprint::gentypeparam);
			if (unlikely(not isGenTypeParam and not frame.atom.isFunction()))
				return complainOperand(operands, "parameter for non-function");

			CLID clid{frame.atom.atomid, paramLVID};
			AnyString name = currentSequence.stringrefs[operands.name];

			auto& parameters = (not isGenTypeParam)
				? frame.atom.parameters : frame.atom.tmplparams;
			parameters.append(clid, name);

			if (frame.capture.enabled())
				frame.capture.knownVars[name] = frame.scope;

			MutexLocker locker{mutex};
			// information about the parameter itself
			auto& cdef = cdeftable.classdef(clid);
			cdef.instance = not isGenTypeParam;
			cdef.qualifiers.ref = false; // should not be 'ref' by default, contrary to all other classdefs

			// making sure the classdef is 'any'
			assert(cdef.atom == nullptr and cdef.isAny());

			if (isGenTypeParam)
			{
				// if a generic type parameter, generating an implicit typedef
				Atom& atom = atomStack->currentAtomNotUnit();
				auto pindex = atom.childrenCount(); // children are currently only typedefs from generic params
				auto* newAliasAtom = cdeftable.atoms.createTypealias(atom, name);
				newAliasAtom->classinfo.isInstanciated = true;
				cdeftable.registerAtom(newAliasAtom);
				newAliasAtom->returnType.clid = cdef.clid; // type of the typedef
				newAliasAtom->classinfo.nextFieldIndex = static_cast<uint16_t>(pindex);
			}
		}


		void mapBlueprintVardef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			// registering the blueprint into the outline...
			Atom& atom = atomStack->currentAtomNotUnit();
			AnyString varname = currentSequence.stringrefs[operands.name];
			if (unlikely(varname.empty()))
				return complainOperand(operands, "invalid func name");
			if (unlikely(atom.type != Atom::Type::classdef))
				return complainOperand(operands, "vardef: invalid parent atom");

			if (atomStack->capture.enabled())
				atomStack->capture.knownVars[varname] = atomStack->scope;

			MutexLocker locker{mutex};
			// create a new atom in the global type table
			auto* newVarAtom = cdeftable.atoms.createVardef(atom, varname);
			assert(newVarAtom != nullptr);

			cdeftable.registerAtom(newVarAtom);
			newVarAtom->returnType.clid.reclass(atom.atomid, operands.lvid);
			if (!firstAtomCreated)
				firstAtomCreated = newVarAtom;
		}


		void mapBlueprintNamespace(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			AnyString nmname = currentSequence.stringrefs[operands.name];
			Atom& parentAtom = atomStack->currentAtomNotUnit();

			MutexLocker locker{mutex};
			Atom* newRoot = cdeftable.atoms.createNamespace(parentAtom, nmname);
			assert(newRoot != nullptr);
			// create a pseudo classdef to easily retrieve the real atom from a clid
			cdeftable.registerAtom(newRoot);
			pushNewFrame(*newRoot);
		}


		void mapBlueprintUnit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			Atom& parentAtom = atomStack->currentAtomNotUnit();
			Atom* newRoot;
			{
				MutexLocker locker{mutex};
				newRoot = cdeftable.atoms.createUnit(parentAtom, currentFilename);
				assert(newRoot != nullptr);
				// create a pseudo classdef to easily retrieve the real atom from a clid
				cdeftable.registerAtom(newRoot);
			}
			// update atomid
			operands.atomid = newRoot->atomid;
			assert(newRoot->atomid != 0);
			pushNewFrame(*newRoot);
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
		{
			if (unlikely(nullptr == atomStack))
				return complainOperand(operands, "invalid stack for blueprint");

			auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
			switch (kind)
			{
				case IR::ISA::Blueprint::vardef:
					{
						mapBlueprintVardef(operands);
						break;
					}
				case IR::ISA::Blueprint::param:
				case IR::ISA::Blueprint::paramself:
				case IR::ISA::Blueprint::gentypeparam:
					{
						mapBlueprintParam(operands);
						break;
					}
				case IR::ISA::Blueprint::funcdef:
				case IR::ISA::Blueprint::typealias:
					{
						mapBlueprintFuncdefOrTypedef(operands);
						break;
					}
				case IR::ISA::Blueprint::classdef:
					{
						mapBlueprintClassdef(operands);
						break;
					}
				case IR::ISA::Blueprint::namespacedef:
					{
						mapBlueprintNamespace(operands);
						break;
					}
				case IR::ISA::Blueprint::unit:
					{
						mapBlueprintUnit(operands);
						break;
					}
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
		{
			if (unlikely(nullptr == atomStack))
				return complainOperand(operands, "invalid stack for blueprint pragma");

			assert(static_cast<uint32_t>(operands.pragma) < IR::ISA::PragmaCount);
			switch ((IR::ISA::Pragma) operands.pragma)
			{
				case IR::ISA::Pragma::codegen:
					{
						break;
					}
				case IR::ISA::Pragma::builtinalias:
					{
						Atom& atom = atomStack->atom;
						atom.builtinalias = currentSequence.stringrefs[operands.value.builtinalias.namesid];
						break;
					}
				case IR::ISA::Pragma::shortcircuit:
					{
						bool onoff = (0 != operands.value.shortcircuit);
						atomStack->atom.parameters.shortcircuitValue = onoff;
						break;
					}
				case IR::ISA::Pragma::suggest:
					{
						bool onoff = (0 != operands.value.suggest);
						if (not onoff)
							atomStack->atom.flags -= Atom::Flags::suggestInReport;
						else
							atomStack->atom.flags += Atom::Flags::suggestInReport;
						break;
					}

				case IR::ISA::Pragma::synthetic:
				case IR::ISA::Pragma::blueprintsize:
				case IR::ISA::Pragma::visibility:
				case IR::ISA::Pragma::bodystart:
				case IR::ISA::Pragma::shortcircuitOpNopOffset:
				case IR::ISA::Pragma::shortcircuitMutateToBool:
				case IR::ISA::Pragma::unknown:
					break;
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::stacksize>& operands)
		{
			if (unlikely(nullptr == atomStack))
				return complainOperand(operands, "invalid parent atom");

			Atom& parentAtom = atomStack->atom;
			if (unlikely(parentAtom.atomid == 0))
				return complainOperand(operands, "mapping: invalid parent atom id");

			// creating all related classdefs
			// (take max with 1 to prevent against invalid opcode)
			uint localvarCount = operands.add;

			MutexLocker locker{mutex};
			parentAtom.localVariablesCount = localvarCount;
			cdeftable.bulkCreate(atomStack->classdefs, parentAtom.atomid, localvarCount);

			switch (parentAtom.type)
			{
				case Atom::Type::funcdef:
				case Atom::Type::typealias:
					{
						// creating all related classdefs
						// (take max with 1 to prevent against invalid opcode)
						if (unlikely(localvarCount == 0))
							return complainOperand(operands, "invalid local variable count for a func blueprint");

						// like parameters, the return type should not 'ref' by default
						cdeftable.classdef(CLID{parentAtom.atomid, 1}).qualifiers.ref = false;
						break;
					}
				default: break;
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::scope>& operands)
		{
			if (unlikely(nullptr == atomStack))
				return complainOperand(operands, "invalid stack");

			++(atomStack->scope);
			lastLVID = 0;
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::end>&)
		{
			// reset the last lvid
			lastLVID = 0u;

			if (likely(atomStack))
			{
				auto& scope = atomStack->scope;
				// the scope might be zero if the opcode 'end' comes from a class or a func
				if (scope > 0)
				{
					--scope;

					if (atomStack->capture.enabled())
					{
						auto& set = atomStack->capture.knownVars;
						for (auto it = set.begin(); it != set.end(); )
						{
							if (it->second > scope)
								it = set.erase(it);
							else
								++it;
						}
					}
				}
				else
				{
					// pop the stack frame
					std::unique_ptr<AtomStackFrame> next{atomStack->next.release()};
					atomStack.swap(next);

					if (!atomStack or (not evaluateWholeSequence and !atomStack->next))
						currentSequence.invalidateCursor(*cursor);
				}
			}
			else
			{
				assert(false and "invalid stack");
				currentSequence.invalidateCursor(*cursor);
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;
			lastLVID = operands.lvid;
			auto clid = atomStack->classdefs[operands.lvid];

			MutexLocker locker{mutex};
			auto& cdef = cdeftable.classdef(clid);
			resetClassdefOriginFromCurrentPosition(cdef);
			switch ((nytype_t) operands.type)
			{
				default:
					{
						cdef.mutateToBuiltin((nytype_t) operands.type);
						cdef.instance = true; // keep somewhere that this definition is a variable instance
						break;
					}
				case nyt_void: cdef.mutateToVoid(); break;
				case nyt_any:  cdef.mutateToAny(); break;
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::self>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.self)))
				return;
			if (unlikely(nullptr == atomStack))
				return complainOperand(operands, "invalid atom stack for 'resolveAsSelf'");

			// we can have at least 2 patterns:
			//
			//  * the most frequent, called from a method contained within a class
			//  * from the class itself, most likely a variable

			// clid of the target variable
			CLID clid {atomStack->atom.atomid, operands.self};

			for (auto* rit = atomStack.get(); rit; rit = rit->next.get())
			{
				if (rit->atom.isClass())
				{
					MutexLocker locker{mutex};
					auto& cdef = cdeftable.classdef(clid);
					resetClassdefOriginFromCurrentPosition(cdef);
					cdef.mutateToAtom(&(rit->atom));
					cdef.qualifiers.ref = true;
					return;
				}
			}

			// fallback - complainOperand
			complainOperand(operands, "failed to find parent class for 'resolveAsSelf'");
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::identify>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;

			if (unlikely(operands.text == 0))
				return complainOperand(operands, "invalid symbol name");
			AnyString name = currentSequence.stringrefs[operands.text];
			if (unlikely(name.empty()))
				return complainOperand(operands, "invalid empty identifier");

			lastLVID = operands.lvid;
			auto& atomFrame = *atomStack;
			auto& localClassdefs = atomFrame.classdefs;

			assert(operands.lvid < localClassdefs.size());
			auto& clid = localClassdefs[operands.lvid];

			if (operands.self == 0)
			{
				// try to determine whether a new variable should be captured
				if (atomFrame.capture.enabled())
				{
					if (name[0] != '^' and atomFrame.capture.knownVars.count(name) == 0)
					{
						// not 100% sure, but this unknown name might be a variable to capture
						Atom& atm = *atomFrame.capture.atom;
						atm.candidatesForCapture->insert(name);
						atomFrame.capture.knownVars[name] = 0; // no scope
					}
				}

				MutexLocker locker{mutex};
				// will see later - currently unknown
				// classdef.mutateToAny();
				cdeftable.addClassdefInterfaceSelf(clid, name);
				resetClassdefOriginFromCurrentPosition(cdeftable.classdef(clid));
			}
			else
			{
				// trying to resolve an attribute
				// (aka `parent.current`)
				if (not checkForLVID(operands, operands.self))
					return;

				// the type is currently unknown
				// classdef.mutateToAny();
				// retrieving the parent classdef
				assert(operands.self < localClassdefs.size());
				auto& selfClassdef = localClassdefs[operands.self];

				MutexLocker locker{mutex};
				auto& funcdef = cdeftable.addClassdefInterface(selfClassdef, name);
				funcdef.clid  = clid;
				auto& cdef = cdeftable.classdef(clid);
				cdef.parentclid = selfClassdef;
				resetClassdefOriginFromCurrentPosition(cdef);
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::identifyset>& operands)
		{
			auto& newopc = IR::Instruction::fromOpcode(operands).to<IR::ISA::Op::identify>();
			visit(newopc);
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::tpush>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::push>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;

			if (operands.name != 0) // named parameter
			{
				AnyString name = currentSequence.stringrefs[operands.name];
				lastPushedNamedParameters.emplace_back(std::make_pair(name, operands.lvid));
			}
			else
			{
				if (unlikely(not lastPushedNamedParameters.empty()))
					return complainOperand(operands, "named parameters must be provided after standard parameters");
				lastPushedIndexedParameters.emplace_back(operands.lvid);
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::call>& operands)
		{
			attachFuncCall(operands);
			lastLVID = operands.lvid;
			lastPushedIndexedParameters.clear();
			lastPushedNamedParameters.clear();
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::ret>& operands)
		{
			if (operands.lvid != 0)
			{
				if (unlikely(not checkForLVID(operands, operands.lvid)))
					return;

				auto& classdefRetValue = atomStack->classdefs[1]; // 1 is the return value
				auto& classdef = atomStack->classdefs[operands.lvid];

				MutexLocker locker{mutex};
				auto& followup = cdeftable.classdef(classdefRetValue).followup;
				followup.extends.push_back(classdef);
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::follow>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;
			if (unlikely(not checkForLVID(operands, operands.follower)))
				return;

			auto& clidFollower = atomStack->classdefs[operands.follower];
			auto& clid = atomStack->classdefs[operands.lvid];

			MutexLocker locker{mutex};
			if (operands.symlink)
				cdeftable.makeHardlink(clid, clidFollower);
			else
				cdeftable.classdef(clidFollower).followup.extends.push_back(clid);
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
		{
			if (unlikely(not checkForLVID(operands, operands.lvid)))
				return;
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::debugfile>& operands)
		{
			currentFilename = currentSequence.stringrefs[operands.filename].c_str();
			if (needAtomDbgFileReport)
			{
				needAtomDbgFileReport = false;
				atomStack->atom.origin.filename = currentFilename;
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
		{
			currentLine = operands.line;
			currentOffset = operands.offset;

			if (unlikely(needAtomDbgOffsetReport))
			{
				needAtomDbgOffsetReport = false;
				atomStack->atom.origin.line = currentLine;
				atomStack->atom.origin.offset = currentOffset;
			}
		}


		void visit(IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
		{
			assert(static_cast<uint32_t>(operands.qualifier) < IR::ISA::TypeQualifierCount);
			CLID clid {atomStack->atom.atomid, operands.lvid};
			bool onoff = (operands.flag != 0);

			MutexLocker locker{mutex};
			if (debugmode and not cdeftable.hasClassdef(clid))
				return complainOperand(operands, "invalid clid");

			auto& qualifiers = cdeftable.classdef(clid).qualifiers;
			switch (operands.qualifier)
			{
				case IR::ISA::TypeQualifier::ref:      qualifiers.ref = onoff; break;
				case IR::ISA::TypeQualifier::constant: qualifiers.constant = onoff; break;
			}
		}


		template<IR::ISA::Op O>
		void visit(IR::ISA::Operand<O>& operands)
		{
			switch (O)
			{
				// all following opcodes can be safely ignored
				case IR::ISA::Op::allocate:
				case IR::ISA::Op::comment:
				case IR::ISA::Op::ensureresolved:
				case IR::ISA::Op::classdefsizeof:
				case IR::ISA::Op::namealias:
				case IR::ISA::Op::store:
				case IR::ISA::Op::storeText:
				case IR::ISA::Op::storeConstant:
				case IR::ISA::Op::memalloc:
				case IR::ISA::Op::memcopy:
				case IR::ISA::Op::typeisobject:
				case IR::ISA::Op::ref:
				case IR::ISA::Op::unref:
				case IR::ISA::Op::assign:
				case IR::ISA::Op::inherit:
				case IR::ISA::Op::label:
				case IR::ISA::Op::jmp:
				case IR::ISA::Op::jz:
				case IR::ISA::Op::jnz:
				case IR::ISA::Op::nop:
					break;
					// error for all the other ones
				default:
					complainOperand(operands, "unsupported opcode in mapping");
					break;
			}
		}


		bool map(Atom& parentAtom, uint32_t offset)
		{
			// some reset if reused
			assert(not atomStack);
			pushNewFrame(parentAtom);

			lastLVID = 0u;
			lastPushedNamedParameters.clear();
			lastPushedIndexedParameters.clear();

			currentFilename = nullptr;
			currentOffset = 0u;
			currentLine = 0u;
			success = true;

			// -- walk through all opcodes
			currentSequence.each(*this, offset);

			atomStack.reset(nullptr); // cleanup after use
			return success;
		}


		//! The classdef table (must be protected by 'mutex' in some passes)
		ClassdefTable& cdeftable;
		//! Mutex for the cdeftable
		Yuni::Mutex& mutex;
		//! Current sequence
		IR::Sequence& currentSequence;

		//! Blueprint root element
		std::unique_ptr<AtomStackFrame> atomStack;
		//! Last lvid (for pushed parameters)
		LVID lastLVID = 0;
		//! The first atom created by the mapping
		Atom*& firstAtomCreated;
		//! Last pushed indexed parameters
		std::vector<LVID> lastPushedIndexedParameters;
		//! Last pushed named parameters
		std::vector<std::pair<AnyString, LVID>> lastPushedNamedParameters;
		//! Prefix to prepend for the first atom created by the mapping
		AnyString prefixNameForFirstAtomCreated;
		//! Flag to evaluate the whole sequence, or only a portion of it
		bool evaluateWholeSequence = true;

		bool needAtomDbgFileReport = false;
		bool needAtomDbgOffsetReport = false;

		//! cursor for iterating through all opcocdes
		IR::Instruction** cursor = nullptr;

		const char* currentFilename = nullptr;
		uint32_t currentLine = 0;
		uint32_t currentOffset = 0;
		SequenceMapping& mapping;
		bool success = false;
	};


	} // anonymous namespace




	SequenceMapping::SequenceMapping(ClassdefTable& cdeftable, Mutex& mutex, IR::Sequence& sequence)
		: cdeftable(cdeftable)
		, mutex(mutex)
		, currentSequence(sequence)
	{
	}


	static void retriveReportMetadata(void* self, Logs::Level level, const AST::Node*, String& filename, uint32_t& line, uint32_t& offset)
	{
		auto& sb    = *(reinterpret_cast<OpcodeReader*>(self));
		sb.success &= Logs::isError(level);
		filename    = sb.currentFilename;
		line        = sb.currentLine;
		offset      = sb.currentOffset;
	}


	bool SequenceMapping::map(Atom& parentAtom, uint32_t offset)
	{
		OpcodeReader reader{*this};
		Logs::MetadataHandler handler{&reader, &retriveReportMetadata};
		return reader.map(parentAtom, offset);
	}




} // namespace Mapping
} // namespace Pass
} // namespace Nany
