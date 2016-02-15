#pragma once
#include "instanciate.h"



namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	inline void SequenceBuilder::PushedParameters::clear()
	{
		func.indexed.clear();
		func.named.clear();
		gentypes.indexed.clear();
		gentypes.named.clear();
	}


	inline bool SequenceBuilder::canBeAcquired(const Classdef& cdef) const
	{
		bool success = not cdef.isBuiltinOrVoid();
		#ifndef NDEBUG
		if (unlikely(success and unlikely(cdeftable.findClassdefAtom(cdef) == nullptr)))
			return (ICE() << "canBeAcquired: invalid atom " << cdef.clid);
		assert(not (success and unlikely(cdeftable.findClassdefAtom(cdef) == nullptr)));
		#endif
		return success;
	}


	inline bool SequenceBuilder::canBeAcquired(const CLID& clid) const
	{
		auto& cdef = cdeftable.classdef(clid);
		return canBeAcquired(cdef);
	}


	inline bool SequenceBuilder::canBeAcquired(LVID lvid) const
	{
		assert(not atomStack.empty());
		assert(lvid < atomStack.back().lvids.size());
		return canBeAcquired(CLID{atomStack.back().atomid, lvid});
	}


	inline bool SequenceBuilder::canGenerateCode() const
	{
		return (codeGenerationLock == 0);
	}

	inline void SequenceBuilder::disableCodeGeneration()
	{
		++codeGenerationLock;
	}




	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::namealias>& operands)
	{
		const auto& name = currentSequence.stringrefs[operands.name];
		declareNamedVariable(name, operands.lvid);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::debugfile>& operands)
	{
		currentFilename = currentSequence.stringrefs[operands.filename].c_str();
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::debugpos>& operands)
	{
		currentLine   = operands.line;
		currentOffset = operands.offset;
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::follow>& operands)
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


	inline bool SequenceBuilder::checkForIntrinsicParamCount(const AnyString& name, uint32_t count)
	{
		return (pushedparams.func.indexed.size() == count)
			? true
			: complainIntrinsicParameterCount(name, count);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::inherit>& operands)
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


	inline void SequenceBuilder::acquireObject(LVID lvid)
	{
		assert(lvid > 1);
		assert(canBeAcquired(lvid));

		out.emitRef(lvid);

		// force unref
		auto& frame = atomStack.back();
		assert(lvid < frame.lvids.size());
		frame.lvids[lvid].autorelease = true;
	}


	inline void SequenceBuilder::tryToAcquireObject(LVID lvid)
	{
		if (canBeAcquired(lvid))
			acquireObject(lvid);
	}


	template<class T> inline void SequenceBuilder::tryToAcquireObject(LVID lvid, const T& type)
	{
		if (canBeAcquired(type))
			acquireObject(lvid);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::unref>& operands)
	{
		tryUnrefObject(operands.lvid);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::nop>&)
	{
		// duplicate nop as well since they can be used to insert code
		// (for shortcircuit for example)
		out.emitNop();
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::label>& operands)
	{
		out.emitLabel(operands.label);
	}


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::qualifiers>& operands)
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


	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jmp>& opc)
	{
		out.emit<IR::ISA::Op::jmp>() = opc;
	}

	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jz>& opc)
	{
		out.emit<IR::ISA::Op::jz>() = opc;
	}

	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::jnz>& opc)
	{
		out.emit<IR::ISA::Op::jnz>() = opc;
	}

	inline void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::comment>& opc)
	{
		// keep the comments in debug
		if (Yuni::debugmode)
			out.emitComment(currentSequence.stringrefs[opc.text]);
	}




	template<IR::ISA::Op O>
	void SequenceBuilder::visit(const IR::ISA::Operand<O>& operands)
	{
		complainOperand(IR::Instruction::fromOpcode(operands));
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
