#pragma once
#include "libnanyc.h"
#include "details/ir/isa/opcodes.h"
#include "details/ir/isa/data.h"

namespace ny {
namespace ir {

struct Instruction final {
	uint32_t opcodes[4];

	//! Convert the instruction into an Opcode struct
	template<isa::Op O> isa::Operand<O>& to() {
		return reinterpret_cast<isa::Operand<O>&>(*this);
	}

	//! Convert the instruction into an Opcode struct (const)
	template<isa::Op O> const isa::Operand<O>& to() const {
		return reinterpret_cast<const isa::Operand<O>&>(*this);
	}

	template<isa::Op O>
	static Instruction& fromOpcode(isa::Operand<O>& opc) {
		return reinterpret_cast<Instruction&>(opc);
	}

	template<isa::Op O>
	static const Instruction& fromOpcode(const isa::Operand<O>& opc) {
		return reinterpret_cast<const Instruction&>(opc);
	}

}; // struct Instruction

} // namespace ir
} // namespace ny
