#pragma once
#include "libnanyc.h"
#include "details/ir/isa/opcodes.h"
#include "details/ir/isa/data.h"



namespace ny
{
namespace ir
{


	struct Instruction final
	{
		uint32_t opcodes[4];

		//! Convert the instruction into an Opcode struct
		template<ISA::Op O> ISA::Operand<O>& to()
		{
			return reinterpret_cast<ISA::Operand<O>&>(*this);
		}

		//! Convert the instruction into an Opcode struct (const)
		template<ISA::Op O> const ISA::Operand<O>& to() const
		{
			return reinterpret_cast<const ISA::Operand<O>&>(*this);
		}

		template<ISA::Op O>
		static Instruction& fromOpcode(ISA::Operand<O>& opc)
		{
			return reinterpret_cast<Instruction&>(opc);
		}

		template<ISA::Op O>
		static const Instruction& fromOpcode(const ISA::Operand<O>& opc)
		{
			return reinterpret_cast<const Instruction&>(opc);
		}

	}; // struct Instruction





} // namespace ir
} // namespace ny
