#include "mapping.h"
#include <iostream>

using namespace Yuni;



namespace Nany
{
namespace Pass
{
namespace Mapping
{


	SequenceMapping::SequenceMapping(ClassdefTable& cdeftable, Mutex& mutex, Logs::Report& report, IR::Sequence& sequence)
		: cdeftable(cdeftable)
		, mutex(mutex)
		, currentSequence(sequence)
		, report(report)
	{
		// reduce memory allocations
		lastPushedNamedParameters.reserve(8); // arbitrary
		lastPushedIndexedParameters.reserve(8);
	}


	inline void SequenceMapping::pushNewFrame(Atom& atom)
	{
		std::unique_ptr<AtomStackFrame> next{atomStack.release()};
		atomStack = std::make_unique<AtomStackFrame>(atom, next);
	}


	Logs::Report SequenceMapping::error()
	{
		success = false;
		auto err = report.error();
		err.message.origins.location.filename   = currentFilename;
		err.message.origins.location.pos.line   = currentLine;
		err.message.origins.location.pos.offset = currentOffset;
		return err;
	}


	void SequenceMapping::printError(const IR::Instruction& operands, AnyString msg)
	{
		// example: ICE: unknown opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
		auto trace = report.ICE();
		if (not msg.empty())
			trace << msg << ':';
		else
			trace << "invalid opcode ";

		trace << " '" << IR::ISA::print(currentSequence, operands) << '\'';
		success = false;
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
		auto atomid          = stackframe.atom.atomid;
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


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid stack for blueprint");

		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		switch (kind)
		{
			case IR::ISA::Blueprint::vardef:
			{
				// registering the blueprint into the outline...
				Atom& atom = atomStack->currentAtomNotUnit();
				AnyString varname = currentSequence.stringrefs[operands.name];
				if (unlikely(varname.empty()))
					return printError(operands, "invalid func name");
				if (unlikely(atom.type != Atom::Type::classdef))
					return printError(operands, "vardef: invalid parent atom");

				MutexLocker locker{mutex};
				// create a new atom in the global type table
				auto* newVarAtom = cdeftable.atoms.createVardef(atom, varname);
				assert(newVarAtom != nullptr);
				newVarAtom->usedDefined = true;

				cdeftable.registerAtom(newVarAtom);
				newVarAtom->returnType.clid.reclass(atom.atomid, operands.lvid);
				if (!firstAtomCreated)
					firstAtomCreated = newVarAtom;
				break;
			}

			case IR::ISA::Blueprint::param:
			case IR::ISA::Blueprint::paramself:
			case IR::ISA::Blueprint::gentypeparam:
			{
				auto& frame = *atomStack;

				// calculating the lvid for the current parameter
				// (+1 since %1 is the return value/type)
				uint paramLVID = (++frame.parameterIndex) + 1;

				if (unlikely(not checkForLVID(operands, paramLVID)))
					return;

				bool isTemplate = (kind == IR::ISA::Blueprint::gentypeparam);
				if (unlikely(not isTemplate and not frame.atom.isFunction()))
					return printError(operands, "parameter for non-function");

				CLID clid {frame.atom.atomid, paramLVID};
				AnyString name = currentSequence.stringrefs[operands.name];

				auto& parameters = (not isTemplate)
					? frame.atom.parameters : frame.atom.tmplparams;
				parameters.append(clid, name);

				// keep somewhere that this definition is a variable instance
				MutexLocker locker{mutex};
				auto& cdef = cdeftable.classdef(clid);
				cdef.instance = not isTemplate;
				cdef.qualifiers.ref = false; // should not be 'ref' by default, contrary to all other classdefs
				break;
			}

			case IR::ISA::Blueprint::funcdef:
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

				MutexLocker locker{mutex};
				// create a new atom in the global type table
				auto* newFuncAtom = cdeftable.atoms.createFuncdef(atom, funcname);
				assert(newFuncAtom != nullptr);
				newFuncAtom->usedDefined      = true;
				newFuncAtom->opcodes.sequence  = &currentSequence;
				newFuncAtom->opcodes.offset   = currentSequence.offsetOf(operands);
				// create a pseudo classdef to easily retrieve the real atom from a clid
				cdeftable.registerAtom(newFuncAtom);

				operands.atomid = newFuncAtom->atomid;

				// return type
				newFuncAtom->returnType.clid.reclass(newFuncAtom->atomid, 1);

				// requires additional information
				needAtomDbgFileReport = true;
				needAtomDbgOffsetReport = true;
				pushNewFrame(*newFuncAtom);

				if (!firstAtomCreated)
					firstAtomCreated = newFuncAtom;
				break;
			}

			case IR::ISA::Blueprint::classdef:
			{
				// registering the blueprint into the outline...
				Atom& atom = atomStack->currentAtomNotUnit();

				// reset last lvid and parameters
				lastLVID = 0;
				lastPushedNamedParameters.clear();
				lastPushedIndexedParameters.clear();

				AnyString classname = currentSequence.stringrefs[operands.name];

				Atom* newClassAtom;
				{
					MutexLocker locker{mutex};
					// create a new atom in the global type table
					if (prefixNameForFirstAtomCreated.empty())
					{
						newClassAtom = cdeftable.atoms.createClassdef(atom, classname);
					}
					else
					{
						String tmpname{prefixNameForFirstAtomCreated};
						prefixNameForFirstAtomCreated.clear();
						newClassAtom = cdeftable.atoms.createClassdef(atom, tmpname << classname);
					}
					// create a pseudo classdef to easily retrieve the real atom from a clid
					cdeftable.registerAtom(newClassAtom);
				}

				newClassAtom->usedDefined     = true;
				newClassAtom->opcodes.sequence = &currentSequence;
				newClassAtom->opcodes.offset  = currentSequence.offsetOf(operands);

				// update atomid
				operands.atomid = newClassAtom->atomid;
				// requires additional information
				needAtomDbgFileReport = true;
				needAtomDbgOffsetReport = true;
				pushNewFrame(*newClassAtom);

				if (!firstAtomCreated)
					firstAtomCreated = newClassAtom;
				break;
			}

			case IR::ISA::Blueprint::typealias:
			{
				Atom& atom = atomStack->currentAtomNotUnit();

				AnyString classname = currentSequence.stringrefs[operands.name];
				Atom* newAliasAtom;
				{
					MutexLocker locker{mutex};
					newAliasAtom = cdeftable.atoms.createTypealias(atom, classname);
					cdeftable.registerAtom(newAliasAtom);
				}

				newAliasAtom->usedDefined     = true;
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
				break;
			}

			case IR::ISA::Blueprint::namespacedef:
			{
				AnyString nmname = currentSequence.stringrefs[operands.name];
				Atom& parentAtom = atomStack->currentAtomNotUnit();

				MutexLocker locker{mutex};
				Atom* newRoot = cdeftable.atoms.createNamespace(parentAtom, nmname);
				assert(newRoot != nullptr);
				newRoot->usedDefined = true;
				// create a pseudo classdef to easily retrieve the real atom from a clid
				cdeftable.registerAtom(newRoot);
				pushNewFrame(*newRoot);
				break;
			}

			case IR::ISA::Blueprint::unit:
			{
				Atom& parentAtom = atomStack->currentAtomNotUnit();
				Atom* newRoot;
				{
					MutexLocker locker{mutex};
					newRoot = cdeftable.atoms.createUnit(parentAtom, currentFilename);
					assert(newRoot != nullptr);
					newRoot->usedDefined = true;
					// create a pseudo classdef to easily retrieve the real atom from a clid
					cdeftable.registerAtom(newRoot);
				}
				// update atomid
				operands.atomid = newRoot->atomid;
				assert(newRoot->atomid != 0);
				pushNewFrame(*newRoot);
				break;
			}
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		if (unlikely(nullptr == atomStack))
			return printError(operands, "invalid stack for blueprint pragma");

		if (unlikely(not (operands.pragma < static_cast<uint32_t>(IR::ISA::Pragma::max))))
			return printError(operands, "invalid pragma");

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
				atomStack->atom.canBeSuggestedInErrReporting = onoff;
				break;
			}

			case IR::ISA::Pragma::blueprintsize:
			case IR::ISA::Pragma::visibility:
			case IR::ISA::Pragma::bodystart:
			case IR::ISA::Pragma::shortcircuitOpNopOffset:
			case IR::ISA::Pragma::unknown:
			case IR::ISA::Pragma::max:
			{
				break;
			}
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
			if (localvarCount == 0)
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

		lastLVID = operands.lvid;
		auto& atomFrame = *atomStack;
		auto& localClassdefs = atomFrame.classdefs;

		assert(operands.lvid < localClassdefs.size());
		auto& clid = localClassdefs[operands.lvid];


		if (operands.self == 0)
		{
			// directly resolving a symbol accessible from the current scope
			// in this pass, we will only resolve local variables (and parameters)
			// all function calls must be resolved later

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
		CLID clid {atomStack->atom.atomid, operands.lvid};
		bool onoff = (operands.flag != 0);

		switch (operands.qualifier)
		{
			case 1: // ref
			{
				MutexLocker locker{mutex};
				if (debugmode and not cdeftable.hasClassdef(clid))
					return printError(operands, "invalid clid");
				cdeftable.classdef(clid).qualifiers.ref = onoff;
				break;
			}
			case 2: // const
			{
				MutexLocker locker{mutex};
				if (debugmode and not cdeftable.hasClassdef(clid))
					return printError(operands, "invalid clid");
				cdeftable.classdef(clid).qualifiers.constant = onoff;
				break;
			}
			default:
				printError(operands, "invalid qualifier value");
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
