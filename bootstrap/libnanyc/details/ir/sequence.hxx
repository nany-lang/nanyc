#pragma once
#include "sequence.h"

#define NANY_PRINT_sequence_OPCODES 0
#if NANY_PRINT_sequence_OPCODES != 0
#include <iostream>
#endif

namespace ny::ir {

inline void Sequence::reserve(uint32_t count) {
	if (m_capacity < count)
		grow(count);
}

inline uint32_t Sequence::opcodeCount() const {
	return m_size;
}

inline uint32_t Sequence::capacity() const {
	return m_capacity;
}

inline const Instruction& Sequence::at(uint32_t offset) const {
	assert(offset < m_size);
	return m_body[offset];
}

inline Instruction& Sequence::at(uint32_t offset) {
	assert(offset < m_size);
	return m_body[offset];
}

template<isa::Op O> inline isa::Operand<O>& Sequence::at(uint32_t offset) {
	assert(offset < m_size);
	static_assert(sizeof(Instruction) >= sizeof(isa::Operand<O>), "m_size mismatch");
	return reinterpret_cast<isa::Operand<O>&>(m_body[offset]);
}

template<isa::Op O> inline const isa::Operand<O>& Sequence::at(uint32_t offset) const {
	assert(offset < m_size);
	static_assert(sizeof(Instruction) >= sizeof(isa::Operand<O>), "m_size mismatch");
	return reinterpret_cast<isa::Operand<O>&>(m_body[offset]);
}

template<isa::Op O>
inline uint32_t Sequence::offsetOf(const isa::Operand<O>& instr) const {
	static_assert(sizeof(Instruction) >= sizeof(isa::Operand<O>), "m_size mismatch");
	return offsetOf(ir::Instruction::fromOpcode(instr));
}

template<class T> inline void Sequence::each(T& visitor, uint32_t offset) {
	if (likely(offset < m_size)) {
		auto* it = m_body + offset;
		const auto* const end = m_body + m_size;
		visitor.cursor = &it;
		for ( ; it < end; ++it) {
			#if NANY_PRINT_sequence_OPCODES != 0
			std::cout << "== opcode == at " << (it - m_body) << "|" << (void*) it << " :: ";
			std::cout << it->opcodes[0] << ": " << ir::isa::print(*this, *it) << '\n';
			#endif
			LIBNANYC_IR_VISIT_SEQUENCE(ir::isa::Operand, visitor, *it);
		}
	}
}

template<class T> inline void Sequence::each(T& visitor, uint32_t offset) const {
	if (likely(offset < m_size)) {
		const auto* it = m_body + offset;
		const auto* const end = m_body + m_size;
		visitor.cursor = &it;
		for ( ; it < end; ++it) {
			#if NANY_PRINT_sequence_OPCODES != 0
			std::cout << "== opcode == at " << (it - m_body) << "|" << (void*) it << " :: ";
			std::cout << it->opcodes[0] << ": " << ir::isa::print(*this, *it) << '\n';
			#endif
			LIBNANYC_IR_VISIT_SEQUENCE(const ir::isa::Operand, visitor, *it);
		}
	}
}

template<isa::Op O> inline isa::Operand<O>& Sequence::emit() {
	if (unlikely(m_capacity < m_size + 1))
		grow(m_size + 1);
	static_assert(sizeof(Instruction) >= sizeof(isa::Operand<O>), "m_size mismatch");
	assert(m_size + 1 <= m_capacity);
	auto& result = at<O>(m_size++);
	result.opcode = static_cast<uint32_t>(O);
	return result;
}

} // ny::ir
