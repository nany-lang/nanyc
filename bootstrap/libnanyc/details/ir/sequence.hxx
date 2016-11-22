#pragma once
#include "sequence.h"

#define NANY_PRINT_sequence_OPCODES 0
#if NANY_PRINT_sequence_OPCODES != 0
#include <iostream>
#endif



namespace ny
{
namespace ir
{


	inline size_t Sequence::sizeInBytes() const
	{
		return m_capacity * sizeof(Instruction) + stringrefs.sizeInBytes();
	}


	inline void Sequence::reserve(uint32_t count)
	{
		if (m_capacity < count)
			grow(count);
	}


	inline uint32_t Sequence::opcodeCount() const
	{
		return m_size;
	}


	inline uint32_t Sequence::capacity() const
	{
		return m_capacity;
	}


	inline const Instruction& Sequence::at(uint32_t offset) const
	{
		assert(offset < m_size);
		return m_body[offset];
	}


	inline Instruction& Sequence::at(uint32_t offset)
	{
		assert(offset < m_size);
		return m_body[offset];
	}


	template<ISA::Op O> inline ISA::Operand<O>& Sequence::at(uint32_t offset)
	{
		assert(offset < m_size);
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "m_size mismatch");
		return reinterpret_cast<ISA::Operand<O>&>(m_body[offset]);
	}


	template<ISA::Op O> inline const ISA::Operand<O>& Sequence::at(uint32_t offset) const
	{
		assert(offset < m_size);
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "m_size mismatch");
		return reinterpret_cast<ISA::Operand<O>&>(m_body[offset]);
	}


	inline uint32_t Sequence::offsetOf(const Instruction& instr) const
	{
		assert(m_size > 0 and m_capacity > 0);
		assert(&instr >= m_body);
		assert(&instr <  m_body + m_size);

		auto start = reinterpret_cast<std::uintptr_t>(m_body);
		auto end   = reinterpret_cast<std::uintptr_t>(&instr);
		assert((end - start) / sizeof(Instruction) < 512 * 1024 * 1024); // arbitrary

		uint32_t r = static_cast<uint32_t>(((end - start) / sizeof(Instruction)));
		assert(r < m_size);
		return r;
	}

	inline bool Sequence::isCursorValid(const Instruction& instr) const
	{
		return (m_size > 0 and m_capacity > 0)
			and (&instr >= m_body)
			and (&instr <  m_body + m_size);
	}


	template<ISA::Op O>
	inline uint32_t Sequence::offsetOf(const ISA::Operand<O>& instr) const
	{
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "m_size mismatch");
		return offsetOf(ir::Instruction::fromOpcode(instr));
	}


	inline void Sequence::invalidateCursor(const Instruction*& cursor) const
	{
		cursor = m_body + m_size;
	}


	inline void Sequence::invalidateCursor(Instruction*& cursor) const
	{
		cursor = m_body + m_size;
	}


	inline bool Sequence::jumpToLabelForward(const Instruction*& cursor, uint32_t label) const
	{
		const auto* const end = m_body + m_size;
		const Instruction* instr = cursor;
		while (++instr < end)
		{
			if (instr->opcodes[0] == static_cast<uint32_t>(ir::ISA::Op::label))
			{
				auto& operands = (*instr).to<ir::ISA::Op::label>();
				if (operands.label == label)
				{
					cursor = instr;
					return true;
				}
			}
		}
		// not found - the cursor is alreayd invalidated
		return false;
	}


	inline bool Sequence::jumpToLabelBackward(const Instruction*& cursor, uint32_t label) const
	{
		const auto* const base = m_body;
		const Instruction* instr = cursor;
		while (instr-- > base)
		{
			if (instr->opcodes[0] == static_cast<uint32_t>(ir::ISA::Op::label))
			{
				auto& operands = (*instr).to<ir::ISA::Op::label>();
				if (operands.label == label)
				{
					cursor = instr;
					return true;
				}
			}
		}
		// not found - invalidate
		return false;
	}


	template<class T> inline void Sequence::each(T& visitor, uint32_t offset)
	{
		if (likely(offset < m_size))
		{
			auto* it = m_body + offset;
			const auto* const end = m_body + m_size;
			visitor.cursor = &it;
			for ( ; it < end; ++it)
			{
				#if NANY_PRINT_sequence_OPCODES != 0
				std::cout << "== opcode == at " << (it - m_body) << "|" << (void*) it << " :: "
					<< it->opcodes[0] << ": " << ir::ISA::print(*this, *it) << '\n';
				#endif
				LIBNANYC_IR_VISIT_SEQUENCE(ir::ISA::Operand, visitor, *it);
			}
		}
	}


	template<class T> inline void Sequence::each(T& visitor, uint32_t offset) const
	{
		if (likely(offset < m_size))
		{
			const auto* it = m_body + offset;
			const auto* const end = m_body + m_size;
			visitor.cursor = &it;
			for ( ; it < end; ++it)
			{
				#if NANY_PRINT_sequence_OPCODES != 0
				std::cout << "== opcode == at " << (it - m_body) << "|" << (void*) it << " :: "
					<< it->opcodes[0] << ": " << ir::ISA::print(*this, *it) << '\n';
				#endif
				LIBNANYC_IR_VISIT_SEQUENCE(const ir::ISA::Operand, visitor, *it);
			}
		}
	}





	template<ISA::Op O> inline ISA::Operand<O>& Sequence::emitraw()
	{
		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "m_size mismatch");
		assert(m_size + 1 <= m_capacity);
		auto& result = at<O>(m_size++);
		result.opcode = static_cast<uint32_t>(O);
		return result;
	}


	template<ISA::Op O> inline ISA::Operand<O>& Sequence::emit()
	{
		if (unlikely(m_capacity < m_size + 1))
			grow(m_size + 1);

		static_assert(sizeof(Instruction) >= sizeof(ISA::Operand<O>), "m_size mismatch");
		assert(m_size + 1 <= m_capacity);
		auto& result = at<O>(m_size++);
		result.opcode = static_cast<uint32_t>(O);
		return result;
	}


} // namespace ir
} // namespace ny
