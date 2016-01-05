#pragma once
#include "instanciate.h"



namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	inline bool ProgramBuilder::canBeAcquired(const Classdef& cdef) const
	{
		bool success = not cdef.isBuiltinOrVoid();
		#ifndef NDEBUG
		if (unlikely(success and unlikely(cdeftable.findClassdefAtom(cdef) == nullptr)))
			return (ICE() << "canBeAcquired: invalid atom " << cdef.clid);
		assert(not (success and unlikely(cdeftable.findClassdefAtom(cdef) == nullptr)));
		#endif
		return success;
	}


	inline bool ProgramBuilder::canBeAcquired(const CLID& clid) const
	{
		auto& cdef = cdeftable.classdef(clid);
		return canBeAcquired(cdef);
	}


	inline bool ProgramBuilder::canBeAcquired(LVID lvid) const
	{
		assert(not atomStack.empty());
		assert(lvid < atomStack.back().lvids.size());
		return canBeAcquired(CLID{atomStack.back().atomid, lvid});
	}


	inline bool ProgramBuilder::canGenerateCode() const
	{
		return (codeGenerationLock == 0);
	}

	inline void ProgramBuilder::disableCodeGeneration()
	{
		++codeGenerationLock;
	}




	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::namealias>& operands)
	{
		declareNamedVariable(currentProgram.stringrefs[operands.name], operands.lvid);
	}

	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::debugfile>& operands)
	{
		currentFilename = currentProgram.stringrefs[operands.filename].c_str();
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
	{
		currentLine   = operands.line;
		currentOffset = operands.offset;
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::push>& operands)
	{
		if (operands.name != 0) // named parameter
		{
			AnyString name = currentProgram.stringrefs[operands.name];
			lastPushedNamedParameters.emplace_back(name, operands.lvid, currentLine, currentOffset);
		}
		else
		{
			lastPushedIndexedParameters.emplace_back(operands.lvid, currentLine, currentOffset);
		}
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::follow>& operands)
	{
		if (not operands.symlink)
		{
			auto& frame = atomStack.back();
			auto& cdef  = cdeftable.classdef(CLID{frame.atomid, operands.lvid});

			auto& spare = cdeftable.substitute(operands.follower);
			spare.import(cdef);
			spare.instance = true;
		}
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::inherit>& operands)
	{
		auto& frame = atomStack.back();
		if (not frame.verify(operands.lhs))
			return;

		switch (operands.inherit)
		{
			case 2: // qualifiers
			{
				auto& spare = cdeftable.substitute(operands.lhs);
				spare.qualifiers = cdeftable.classdef(CLID{frame.atomid, operands.rhs}).qualifiers;
				break;
			}
			default:
			{
				ICE() << "invalid inherit value " << operands.inherit;
			}
		}
	}


	inline void ProgramBuilder::acquireObject(LVID lvid)
	{
		assert(lvid > 1);
		assert(canBeAcquired(lvid));

		out.emitRef(lvid);

		// force unref
		auto& frame = atomStack.back();
		assert(lvid < frame.lvids.size());
		frame.lvids[lvid].autorelease = true;
	}


	inline void ProgramBuilder::tryToAcquireObject(LVID lvid)
	{
		if (canBeAcquired(lvid))
			acquireObject(lvid);
	}


	template<class T> inline void ProgramBuilder::tryToAcquireObject(LVID lvid, const T& type)
	{
		if (canBeAcquired(type))
			acquireObject(lvid);
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::unref>& operands)
	{
		tryUnrefObject(operands.lvid);
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::nop>&)
	{
		// duplicate nop as well since they can be used to insert code
		// (for shortcircuit for example)
		out.emitNop();
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::label>& operands)
	{
		out.emitLabel(operands.label);
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
	{
		assert(not atomStack.empty());
		bool  onoff = (operands.flag != 0);
		auto& spare = cdeftable.substitute(operands.lvid);

		switch (operands.qualifier)
		{
			case 1: // ref
			{
				spare.qualifiers.ref = onoff;
				break;
			}
			case 2: // const
			{
				spare.qualifiers.constant = onoff;
				break;
			}
			default:
				ICE() << "unknown qualifier constant " << operands.qualifier;
		}
	}


	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jmp>& opc)
	{
		out.emit<IR::ISA::Op::jmp>() = opc;
	}

	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jz>& opc)
	{
		out.emit<IR::ISA::Op::jz>() = opc;
	}

	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jnz>& opc)
	{
		out.emit<IR::ISA::Op::jnz>() = opc;
	}

	inline void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::comment>& opc)
	{
		// keep the comments in debug
		if (Yuni::debugmode)
			out.emitComment(currentProgram.stringrefs[opc.text]);
	}




	template<enum IR::ISA::Op O>
	void ProgramBuilder::visit(const IR::ISA::Operand<O>& operands)
	{
		complainOperand(reinterpret_cast<const IR::Instruction&>(operands));
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
