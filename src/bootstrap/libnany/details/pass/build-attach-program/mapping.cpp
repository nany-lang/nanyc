#include "mapping.h"

using namespace Yuni;



namespace Nany
{
namespace Pass
{
namespace Mapping
{


	SequenceMapping::SequenceMapping(Logs::Report& report, Isolate& isolate, IR::Sequence& sequence)
		: isolate(isolate)
		, currentSequence(sequence)
		, report(report)
	{
		// reduce memory allocations
		atomStack.reserve(4); // arbitrary
		lastPushedNamedParameters.reserve(8); // arbitrary
		lastPushedIndexedParameters.reserve(8);
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
			if (unlikely(lvid == 0 or not (lvid < atomStack.back().classdefs.size())))
			{
				printError(operands, String{"mapping: invalid lvid %"} << lvid << " (upper bound: %"
					<< atomStack.back().classdefs.size() << ')');
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


	inline void SequenceMapping::attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.ptr2func)))
			return;
		if (unlikely(not checkForLVID(operands, operands.lvid)))
			return;

		auto& stackframe = atomStack.back();
		auto atomid      = stackframe.atom.atomid;
		auto& clidFuncToCall = stackframe.classdefs[operands.ptr2func];
		auto& clidRetValue   = stackframe.classdefs[operands.lvid];


		// update all underlying types
		{
			MutexLocker locker{isolate.mutex};

			resetClassdefOriginFromCurrentPosition(isolate.classdefTable.classdef(clidRetValue));

			auto& funcdef = isolate.classdefTable.addClassdefInterfaceSelf(clidFuncToCall);
			funcdef.resetReturnType(clidRetValue);
			funcdef.clid = clidFuncToCall;
			CLID clid{atomid, 0};

			for (uint i = 0; i != lastPushedIndexedParameters.size(); ++i)
			{
				auto lvid = lastPushedIndexedParameters[i];
				clid.reclass(lvid);
				funcdef.appendParameter(clid);

				auto& followup = isolate.classdefTable.classdef(clid).followup;
				followup.pushedIndexedParams.push_back(std::make_pair(clidFuncToCall, i));
			}

			for (auto& pair: lastPushedNamedParameters)
			{
				clid.reclass(pair.second);
				funcdef.appendParameter(pair.first, clid);

				auto& followup = isolate.classdefTable.classdef(clid).followup;
				followup.pushedNamedParams.push_back(std::make_pair(clidFuncToCall, pair.first));
			}
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands)
	{
		auto kind = static_cast<IR::ISA::Blueprint>(operands.kind);
		switch (kind)
		{
			case IR::ISA::Blueprint::vardef:
			{
				assert(not atomStack.empty());
				// registering the blueprint into the outline...
				Atom& atom = atomStack.back().currentAtomNotUnit();
				AnyString varname = currentSequence.stringrefs[operands.name];
				if (unlikely(varname.empty()))
					return printError(operands, "invalid func name");
				if (unlikely(atom.type != Atom::Type::classdef))
					return printError(operands, "vardef: invalid parent atom");

				MutexLocker locker{isolate.mutex};
				// create a new atom in the global type table
				auto* newVarAtom = isolate.classdefTable.atoms.createVardef(atom, varname);
				assert(newVarAtom != nullptr);
				newVarAtom->usedDefined = true;

				isolate.classdefTable.registerAtom(newVarAtom);
				newVarAtom->returnType.clid.reclass(atom.atomid, operands.lvid);
				break;
			}

			case IR::ISA::Blueprint::param:
			case IR::ISA::Blueprint::paramself:
			case IR::ISA::Blueprint::tmplparam:
			{
				assert(not atomStack.empty());
				auto& frame = atomStack.back();
				assert(frame.atom.isFunction());

				// calculating the lvid for the current parameter
				// (+1 since %1 is the return value/type)
				uint paramLVID = (++frame.parameterIndex) + 1;

				if (unlikely(not checkForLVID(operands, paramLVID)))
					return;

				bool isTemplate = (kind == IR::ISA::Blueprint::tmplparam);

				CLID clid {frame.atom.atomid, paramLVID};
				AnyString name = currentSequence.stringrefs[operands.name];

				auto& parameters = (not isTemplate)
					? frame.atom.parameters : frame.atom.tmplparams;
				parameters.append(clid, name);

				// keep somewhere that this definition is a variable instance
				MutexLocker locker{isolate.mutex};
				auto& cdef = isolate.classdefTable.classdef(clid);
				cdef.instance = not isTemplate;
				cdef.qualifiers.ref = false; // should not be 'ref' by default, contrary to all other classdefs
				break;
			}

			case IR::ISA::Blueprint::funcdef:
			{
				assert(not atomStack.empty());
				// registering the blueprint into the outline...
				Atom& atom = atomStack.back().currentAtomNotUnit();
				AnyString funcname = currentSequence.stringrefs[operands.name];
				if (unlikely(funcname.empty()))
					return printError(operands, "invalid func name");

				// reset last lvid and parameters
				lastLVID = 0;
				lastPushedNamedParameters.clear();
				lastPushedIndexedParameters.clear();

				MutexLocker locker{isolate.mutex};
				// create a new atom in the global type table
				auto* newFuncAtom = isolate.classdefTable.atoms.createFuncdef(atom, funcname);
				assert(newFuncAtom != nullptr);
				newFuncAtom->usedDefined      = true;
				newFuncAtom->opcodes.sequence  = &currentSequence;
				newFuncAtom->opcodes.offset   = currentSequence.offsetOf(operands);
				// create a pseudo classdef to easily retrieve the real atom from a clid
				isolate.classdefTable.registerAtom(newFuncAtom);

				operands.atomid = newFuncAtom->atomid;

				// return type
				newFuncAtom->returnType.clid.reclass(newFuncAtom->atomid, 1);

				// requires additional information
				needAtomDbgFileReport = true;
				needAtomDbgOffsetReport = true;
				atomStack.push_back(AtomStackFrame{*newFuncAtom});
				break;
			}

			case IR::ISA::Blueprint::classdef:
			{
				// registering the blueprint into the outline...
				assert(not atomStack.empty());
				Atom& atom = atomStack.back().currentAtomNotUnit();

				// reset last lvid and parameters
				lastLVID = 0;
				lastPushedNamedParameters.clear();
				lastPushedIndexedParameters.clear();

				AnyString classname = currentSequence.stringrefs[operands.name];

				Atom* newClassAtom;
				{
					MutexLocker locker{isolate.mutex};
					// create a new atom in the global type table
					newClassAtom = isolate.classdefTable.atoms.createClassdef(atom, classname);
					// create a pseudo classdef to easily retrieve the real atom from a clid
					isolate.classdefTable.registerAtom(newClassAtom);
				}

				newClassAtom->usedDefined     = true;
				newClassAtom->opcodes.sequence = &currentSequence;
				newClassAtom->opcodes.offset  = currentSequence.offsetOf(operands);

				// update atomid
				operands.atomid = newClassAtom->atomid;
				// requires additional information
				needAtomDbgFileReport = true;
				needAtomDbgOffsetReport = true;
				atomStack.push_back(AtomStackFrame{*newClassAtom});
				break;
			}

			case IR::ISA::Blueprint::typealias:
			{
				assert(not atomStack.empty());
				Atom& atom = atomStack.back().currentAtomNotUnit();

				AnyString classname = currentSequence.stringrefs[operands.name];
				Atom* newAliasAtom;
				{
					MutexLocker locker{isolate.mutex};
					newAliasAtom = isolate.classdefTable.atoms.createTypealias(atom, classname);
					isolate.classdefTable.registerAtom(newAliasAtom);
				}

				newAliasAtom->usedDefined     = true;
				newAliasAtom->opcodes.sequence = &currentSequence;
				newAliasAtom->opcodes.offset  = currentSequence.offsetOf(operands);

				switch (static_cast<uint32_t>(operands.lvid))
				{
					default:
					{
						CLID clid{atomStack.back().atom.atomid, operands.lvid};
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
				break;
			}

			case IR::ISA::Blueprint::namespacedef:
			{
				AnyString nmname = currentSequence.stringrefs[operands.name];
				assert(not atomStack.empty());
				Atom& parentAtom = atomStack.back().currentAtomNotUnit();

				MutexLocker locker{isolate.mutex};
				Atom* newRoot = isolate.classdefTable.atoms.createNamespace(parentAtom, nmname);
				assert(newRoot != nullptr);
				newRoot->usedDefined = true;
				// create a pseudo classdef to easily retrieve the real atom from a clid
				isolate.classdefTable.registerAtom(newRoot);
				atomStack.push_back(AtomStackFrame{*newRoot});
				break;
			}

			case IR::ISA::Blueprint::unit:
			{
				assert(not atomStack.empty());
				Atom& parentAtom = atomStack.back().currentAtomNotUnit();

				Atom* newRoot;
				{
					MutexLocker locker{isolate.mutex};
					newRoot = isolate.classdefTable.atoms.createUnit(parentAtom, currentFilename);
					assert(newRoot != nullptr);
					newRoot->usedDefined = true;
					// create a pseudo classdef to easily retrieve the real atom from a clid
					isolate.classdefTable.registerAtom(newRoot);
				}

				// update atomid
				operands.atomid = newRoot->atomid;
				assert(newRoot->atomid != 0);

				atomStack.push_back(AtomStackFrame{*newRoot});
				break;
			}
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::pragma>& operands)
	{
		assert(not atomStack.empty());
		if (atomStack.empty())
			return printError(operands, "invalid stack for blueprint param");

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
				assert(not atomStack.empty());
				Atom& atom = atomStack.back().atom;
				atom.builtinalias = currentSequence.stringrefs[operands.value.builtinalias.namesid];
				break;
			}

			case IR::ISA::Pragma::shortcircuit:
			{
				assert(not atomStack.empty());
				atomStack.back().atom.parameters.shortcircuitValue = (0 != operands.value.shortcircuit);
				break;
			}
			case IR::ISA::Pragma::suggest:
			{
				assert(not atomStack.empty());
				atomStack.back().atom.canBeSuggestedInErrReporting = (0 != operands.value.suggest);
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
		if (unlikely(atomStack.empty()))
			return printError(operands, "invalid parent atom");

		Atom& parentAtom = atomStack.back().atom;
		if (unlikely(parentAtom.atomid == 0))
			return printError(operands, "mapping: invalid parent atom id");

		// creating all related classdefs
		// (take max with 1 to prevent against invalid opcode)
		uint localvarCount = operands.add;

		MutexLocker locker{isolate.mutex};
		parentAtom.localVariablesCount = localvarCount;
		isolate.classdefTable.bulkCreate(atomStack.back().classdefs, parentAtom.atomid, localvarCount);

		if (parentAtom.type == Atom::Type::funcdef)
		{
			// creating all related classdefs
			// (take max with 1 to prevent against invalid opcode)
			if (localvarCount == 0)
				return printError(operands, "invalid local variable count for a func blueprint");

			// like parameters, the return type should not 'ref' by default
			isolate.classdefTable.classdef(CLID{parentAtom.atomid, 1}).qualifiers.ref = false;
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::scope>&)
	{
		if (unlikely(not (atomStack.size() > 0)))
			throw (String{} << currentFilename << ':' << currentLine << ": invalid stack");

		if (not atomStack.empty())
			++(atomStack.back().scope);

		lastLVID = 0;
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::end>&)
	{
		// reset the last lvid
		lastLVID = 0;
		// pop the stack

		if (unlikely(not (atomStack.size() > 1)))
			throw (String{} << currentFilename << ':' << currentLine << ": invalid stack");

		if (likely(not atomStack.empty()))
		{
			auto& scope = atomStack.back().scope;
			// the scope might be zero if the opcode 'end' comes from a class or a func
			if (scope > 0)
			{
				--scope;
			}
			else
			{
				// remove a part of the stack
				atomStack.pop_back();
				if (not evaluateWholeSequence and atomStack.size() == 1)
					currentSequence.invalidateCursor(*cursor);
			}
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.lvid)))
			return;
		lastLVID = operands.lvid;
		auto clid = atomStack.back().classdefs[operands.lvid];

		MutexLocker locker{isolate.mutex};
		auto& cdef = isolate.classdefTable.classdef(clid);
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
		if (unlikely(atomStack.empty()))
			return printError(operands, "invalid atom stack for 'resolveAsSelf'");

		// we can have at least 2 patterns:
		//
		//  * the most frequent, called from a method contained within a class
		//  * from the class itself, most likely a variable
		for (auto rit = atomStack.rbegin(); rit != atomStack.rend(); ++rit)
		{
			if (rit->atom.isClass())
			{
				// clid of the target variable
				CLID clid {atomStack.back().atom.atomid, operands.self};

				MutexLocker locker{isolate.mutex};
				auto& cdef = isolate.classdefTable.classdef(clid);
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
		auto& atomFrame = atomStack.back();
		auto& localClassdefs = atomFrame.classdefs;

		assert(operands.lvid < localClassdefs.size());
		auto& clid = localClassdefs[operands.lvid];


		if (operands.self == 0)
		{
			// directly resolving a symbol accessible from the current scope
			// in this pass, we will only resolve local variables (and parameters)
			// all function calls must be resolved later

			MutexLocker locker{isolate.mutex};
			// will see later - currently unknown
			// classdef.mutateToAny();
			isolate.classdefTable.addClassdefInterfaceSelf(clid, name);
			resetClassdefOriginFromCurrentPosition(isolate.classdefTable.classdef(clid));
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

			MutexLocker locker{isolate.mutex};
			auto& funcdef = isolate.classdefTable.addClassdefInterface(selfClassdef, name);
			funcdef.clid  = clid;
			auto& cdef = isolate.classdefTable.classdef(clid);
			cdef.parentclid = selfClassdef;
			resetClassdefOriginFromCurrentPosition(cdef);
		}
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

			auto& stackframe = atomStack.back();
			auto& classdefRetValue = stackframe.classdefs[1]; // 1 is the return value
			auto& classdef = stackframe.classdefs[operands.lvid];

			MutexLocker locker{isolate.mutex};
			auto& followup = isolate.classdefTable.classdef(classdefRetValue).followup;
			followup.extends.push_back(classdef);
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::follow>& operands)
	{
		if (unlikely(not checkForLVID(operands, operands.lvid)))
			return;
		if (unlikely(not checkForLVID(operands, operands.follower)))
			return;

		auto& stackframe = atomStack.back();
		auto& clidFollower = stackframe.classdefs[operands.follower];
		auto& clid = stackframe.classdefs[operands.lvid];

		MutexLocker locker{isolate.mutex};
		if (operands.symlink)
			isolate.classdefTable.makeHardlink(clid, clidFollower);
		else
			isolate.classdefTable.classdef(clidFollower).followup.extends.push_back(clid);
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
			atomStack.back().atom.origin.filename = currentFilename;
		}
	}

	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
	{
		currentLine = operands.line;
		currentOffset = operands.offset;

		if (unlikely(needAtomDbgOffsetReport))
		{
			needAtomDbgOffsetReport = false;
			atomStack.back().atom.origin.line = currentLine;
			atomStack.back().atom.origin.offset = currentOffset;
		}
	}


	inline void SequenceMapping::visit(IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
	{
		auto& frame = atomStack.back();
		CLID clid {frame.atom.atomid, operands.lvid};
		bool onoff = (operands.flag != 0);

		switch (operands.qualifier)
		{
			case 1: // ref
			{
				MutexLocker locker{isolate.mutex};
				if (debugmode and not isolate.classdefTable.hasClassdef(clid))
					return printError(operands, "invalid clid");
				isolate.classdefTable.classdef(clid).qualifiers.ref = onoff;
				break;
			}
			case 2: // const
			{
				MutexLocker locker{isolate.mutex};
				if (debugmode and not isolate.classdefTable.hasClassdef(clid))
					return printError(operands, "invalid clid");
				isolate.classdefTable.classdef(clid).qualifiers.constant = onoff;
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


	bool SequenceMapping::map(Atom& root, uint32_t offset)
	{
		// few reset
		atomStack.clear();
		atomStack.emplace_back(root);
		lastLVID = 0;
		lastPushedNamedParameters.clear();
		lastPushedIndexedParameters.clear();

		currentFilename = nullptr;
		currentOffset = 0;
		currentLine = 0;
		success = true;

		currentSequence.each(*this, offset);
		return success;
	}




} // namespace Mapping
} // namespace Pass
} // namespace Nany
