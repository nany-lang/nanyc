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


	SequenceMapping::SequenceMapping(ClassdefTable& cdeftable, Mutex& mutex, IR::Sequence& sequence)
		: cdeftable(cdeftable)
		, mutex(mutex)
		, currentSequence(sequence)
		, localMetadataHandler(this, &retriveReportMetadata)

	{
		// reduce memory allocations
		lastPushedNamedParameters.reserve(8); // arbitrary
		lastPushedIndexedParameters.reserve(8);
	}


	void SequenceMapping::retriveReportMetadata(void* self, Logs::Level level, const AST::Node*, String& filename, uint32_t& line, uint32_t& offset)
	{
		auto& sb    = *(reinterpret_cast<SequenceMapping*>(self));
		sb.success &= Logs::isError(level);
		filename    = sb.currentFilename;
		line        = sb.currentLine;
		offset      = sb.currentOffset;
	}


	inline void SequenceMapping::pushNewFrame(Atom& atom)
	{
		std::unique_ptr<AtomStackFrame> next{atomStack.release()};
		atomStack = std::make_unique<AtomStackFrame>(atom, next);
	}


	void SequenceMapping::printError(const IR::Instruction& operands, AnyString msg)
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


	inline void AtomStackFrame::CaptureVariables::enabled(Atom* newatom)
	{
		atom = newatom;
		atom->flags += Atom::Flags::captureVariables;

		if (!newatom->candidatesForCapture)
		{
			typedef decltype(newatom->candidatesForCapture) Set;
			newatom->candidatesForCapture = std::make_unique<Set::element_type>();
		}
	}


	template<IR::ISA::Op O>
	inline void SequenceMapping::printError(const IR::ISA::Operand<O>& operands, AnyString msg)
	{
		printError(IR::Instruction::fromOpcode(operands), msg);
	}


	template<IR::ISA::Op O>
	inline bool SequenceMapping::checkForLVID(const IR::ISA::Operand<O>& operands, LVID lvid)
	{
		if (debugmode)
		{
			if (unlikely(lvid == 0 or not (lvid < atomStack->classdefs.size())))
			{
				printError(operands, String{"mapping: invalid lvid %"} << lvid << " (upper bound: %"
					<< atomStack->classdefs.size() << ')');
				return false;
			}
		}
		return true;
	}


	inline void SequenceMapping::resetClassdefOriginFromCurrentPosition(Classdef& cdef)
	{
		cdef.origins.filename = currentFilename;
		cdef.origins.line     = currentLine;
		cdef.origins.offset   = currentOffset;
	}


	void SequenceMapping::attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
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


	void SequenceMapping::mapBlueprintFuncdef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		// registering the blueprint into the outline...
		Atom& atom = atomStack->currentAtomNotUnit();
		AnyString funcname = currentSequence.stringrefs[operands.name];
		if (unlikely(funcname.empty()))
			return printError(operands, "invalid func name");

		// reset last lvid and parameters
		lastLVID = 0;
		lastPushedNamedParameters.clear();
		lastPushedIndexedParameters.clear();

		bool isGlobalOperator = (atom.type == Atom::Type::namespacedef)
			and funcname[0] == '^'
			and (not funcname.startsWith("^view^"))
			and (not funcname.startsWith("^prop"));

		auto& parentAtom = (not isGlobalOperator)
			? atom
			: cdeftable.atoms.root;

		MutexLocker locker{mutex};
		// create a new atom in the global type table
		auto* newatom = cdeftable.atoms.createFuncdef(parentAtom, funcname);
		assert(newatom != nullptr);
		newatom->opcodes.sequence  = &currentSequence;
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


	void SequenceMapping::mapBlueprintClassdef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
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


	void SequenceMapping::mapBlueprintParam(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
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
			return printError(operands, "parameter for non-function");

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
			auto* newAliasAtom = cdeftable.atoms.createTypealias(atom, name);
			cdeftable.registerAtom(newAliasAtom);
			newAliasAtom->returnType.clid = cdef.clid; // type of the typedef
		}
	}


	void SequenceMapping::mapBlueprintTypealias(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		Atom& atom = atomStack->currentAtomNotUnit();

		AnyString typedefname = currentSequence.stringrefs[operands.name];
		Atom* newAliasAtom = nullptr;
		{
			MutexLocker locker{mutex};
			newAliasAtom = cdeftable.atoms.createTypealias(atom, typedefname);
			cdeftable.registerAtom(newAliasAtom);
		}

		assert(newAliasAtom != nullptr);
		newAliasAtom->opcodes.sequence = &currentSequence;
		newAliasAtom->opcodes.offset  = currentSequence.offsetOf(operands);

		switch (static_cast<uint32_t>(operands.lvid))
		{
			default:
			{
				CLID clid{atomStack->atom.atomid, operands.lvid};
				newAliasAtom->returnType.clid = clid;
				break;
			}
			case 0:
			{
				newAliasAtom->returnType.clid.reclassToVoid();
				break;
			}
			case (uint32_t) -1: // any
				break;
		}

		// update atomid
		operands.atomid = newAliasAtom->atomid;

		lastPushedNamedParameters.clear();
		lastPushedIndexedParameters.clear();

		if (!firstAtomCreated)
			firstAtomCreated = newAliasAtom;
	}


	void SequenceMapping::mapBlueprintVardef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		// registering the blueprint into the outline...
		Atom& atom = atomStack->currentAtomNotUnit();
		AnyString varname = currentSequence.stringrefs[operands.name];
		if (unlikely(varname.empty()))
			return printError(operands, "invalid func name");
		if (unlikely(atom.type != Atom::Type::classdef))
			return printError(operands, "vardef: invalid parent atom");

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


	void SequenceMapping::mapBlueprintNamespace(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
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


	void SequenceMapping::mapBlueprintUnit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid stack for blueprint");

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
			{
				mapBlueprintFuncdef(operands);
				break;
			}
			case IR::ISA::Blueprint::classdef:
			{
				mapBlueprintClassdef(operands);
				break;
			}

			case IR::ISA::Blueprint::typealias:
			{
				mapBlueprintTypealias(operands);
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid stack for blueprint pragma");

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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::stacksize>& operands)
	{
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid parent atom");

		Atom& parentAtom = atomStack->atom;
		if (unlikely(parentAtom.atomid == 0))
			return printError(operands, "mapping: invalid parent atom id");

		// creating all related classdefs
		// (take max with 1 to prevent against invalid opcode)
		uint localvarCount = operands.add;

		MutexLocker locker{mutex};
		parentAtom.localVariablesCount = localvarCount;
		cdeftable.bulkCreate(atomStack->classdefs, parentAtom.atomid, localvarCount);

		if (parentAtom.type == Atom::Type::funcdef)
		{
			// creating all related classdefs
			// (take max with 1 to prevent against invalid opcode)
			if (unlikely(localvarCount == 0))
				return printError(operands, "invalid local variable count for a func blueprint");

			// like parameters, the return type should not 'ref' by default
			cdeftable.classdef(CLID{parentAtom.atomid, 1}).qualifiers.ref = false;
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::scope>& operands)
	{
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid stack");

		++(atomStack->scope);
		lastLVID = 0;
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::end>&)
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::self>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.self)))
			return;
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid atom stack for 'resolveAsSelf'");

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

		// fallback - error
		printError(operands, "failed to find parent class for 'resolveAsSelf'");
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::identify>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.lvid)))
			return;

		if (unlikely(operands.text == 0))
			return printError(operands, "invalid symbol name");
		AnyString name = currentSequence.stringrefs[operands.text];
		if (unlikely(name.empty()))
			return printError(operands, "invalid empty identifier");

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

	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::identifyset>& operands)
	{
		auto& newopc = IR::Instruction::fromOpcode(operands).to<IR::ISA::Op::identify>();
		visit(newopc);
	}

	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::tpush>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.lvid)))
			return;
	}

	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::push>& operands)
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
				return printError(operands, "named parameters must be provided after standard parameters");

			lastPushedIndexedParameters.emplace_back(operands.lvid);
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		attachFuncCall(operands);
		lastLVID = operands.lvid;
		lastPushedIndexedParameters.clear();
		lastPushedNamedParameters.clear();
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::ret>& operands)
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::follow>& operands)
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.lvid)))
			return;
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::debugfile>& operands)
	{
		currentFilename = currentSequence.stringrefs[operands.filename].c_str();
		if (needAtomDbgFileReport)
		{
			needAtomDbgFileReport = false;
			atomStack->atom.origin.filename = currentFilename;
		}
	}

	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
	{
		assert(static_cast<uint32_t>(operands.qualifier) < IR::ISA::TypeQualifierCount);
		CLID clid {atomStack->atom.atomid, operands.lvid};
		bool onoff = (operands.flag != 0);

		MutexLocker locker{mutex};
		if (debugmode and not cdeftable.hasClassdef(clid))
			return printError(operands, "invalid clid");

		auto& qualifiers = cdeftable.classdef(clid).qualifiers;
		switch (operands.qualifier)
		{
			case IR::ISA::TypeQualifier::ref:      qualifiers.ref = onoff; break;
			case IR::ISA::TypeQualifier::constant: qualifiers.constant = onoff; break;
		}
	}


	template<IR::ISA::Op O>
	inline void SequenceMapping::visit(IR::ISA::Operand<O>& operands)
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
				printError(operands);
				break;
		}
	}


	bool SequenceMapping::map(Atom& parentAtom, uint32_t offset)
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




} // namespace Mapping
} // namespace Pass
} // namespace Nany
