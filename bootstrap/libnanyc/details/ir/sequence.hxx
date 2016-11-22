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


	inline Sequence::~Sequence()
	{
		free(m_body);
	}


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


	inline void Sequence::emitMemcheckhold(uint32_t lvid, uint32_t size)
	{
		auto& opr = emit<ISA::Op::memcheckhold>();
		opr.lvid = lvid;
		opr.size = size;
	}

	inline void Sequence::emitNOT(uint32_t lvid, uint32_t lhs)
	{
		auto& operands = emit<ISA::Op::negation>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
	}

	inline void Sequence::emitFADD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fadd>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fsub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fdiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fmul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitADD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::add>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::sub>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::div>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::mul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::imul>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::idiv>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::eq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitNEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::neq>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFLT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::flt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::flte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fgt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitFGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::fgte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitLT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::lt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::lte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitILT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::ilt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitILTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::ilte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::gt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::gte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIGT(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::igt>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitIGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::igte>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}


	inline void Sequence::emitAND(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opand>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitOR(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitXOR(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opxor>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitMOD(uint32_t lvid, uint32_t lhs, uint32_t rhs)
	{
		auto& operands = emit<ISA::Op::opmod>();
		operands.lvid  = lvid;
		operands.lhs   = lhs;
		operands.rhs   = rhs;
	}

	inline void Sequence::emitPragmaSynthetic(uint32_t lvid, bool onoff)
	{
		auto& opc  = emit<ISA::Op::pragma>();
		opc.pragma = ir::ISA::Pragma::synthetic;
		opc.value.synthetic.lvid  = lvid;
		opc.value.synthetic.onoff = static_cast<uint32_t>(onoff);
	}

	inline void Sequence::emitPragmaSuggest(bool onoff)
	{
		auto& opc  = emit<ISA::Op::pragma>();
		opc.pragma = ir::ISA::Pragma::suggest;
		opc.value.suggest = static_cast<uint32_t>(onoff);
	}

	inline void Sequence::emitPragmaBuiltinAlias(const AnyString& name)
	{
		auto& opc  = emit<ISA::Op::pragma>();
		opc.pragma = ir::ISA::Pragma::builtinalias;
		opc.value.builtinalias.namesid = stringrefs.ref(name);
	}

	inline void Sequence::emitPragmaShortcircuit(bool evalvalue)
	{
		auto& opc  = emit<ISA::Op::pragma>();
		opc.pragma = ir::ISA::Pragma::shortcircuit;
		opc.value.shortcircuit = static_cast<uint32_t>(evalvalue);
	}

	inline void Sequence::emitPragmaShortcircuitMetadata(uint32_t label)
	{
		auto& opc  = emit<ISA::Op::pragma>();
		opc.pragma = ir::ISA::Pragma::shortcircuitOpNopOffset;
		opc.value.shortcircuitMetadata.label = label;
	}

	inline void Sequence::emitPragmaShortcircuitMutateToBool(uint32_t lvid, uint32_t source)
	{
		auto& opc  = emit<ISA::Op::pragma>();
		opc.pragma = ir::ISA::Pragma::shortcircuitMutateToBool;
		opc.value.shortcircuitMutate.lvid = lvid;
		opc.value.shortcircuitMutate.source = source;
	}

	inline void Sequence::emitPragmaFuncBody()
	{
		emit<ISA::Op::pragma>().pragma = ir::ISA::Pragma::bodystart;
	}

	inline void Sequence::emitPragmaAllowCodeGeneration(bool enabled)
	{
		auto& operands  = emit<ISA::Op::pragma>();
		operands.pragma = ISA::Pragma::codegen;
		operands.value.codegen = static_cast<uint32_t>(enabled);
	}

	inline void Sequence::emitVisibility(nyvisibility_t visibility)
	{
		auto& operands  = emit<ISA::Op::pragma>();
		operands.pragma = ISA::Pragma::visibility;
		operands.value.visibility = static_cast<uint32_t>(visibility);
	}

	inline uint32_t Sequence::emitStackSizeIncrease(uint32_t size)
	{
		uint32_t offset = m_size;
		emit<ISA::Op::stacksize>().add = size;
		return offset;
	}

	inline uint32_t Sequence::emitStackSizeIncrease()
	{
		return emitStackSizeIncrease(0);
	}

	inline uint32_t Sequence::emitBlueprintSize()
	{
		uint32_t offset = m_size;
		auto& operands = emit<ISA::Op::pragma>();
		operands.pragma = ISA::Pragma::blueprintsize;
		operands.value.blueprintsize = 0;
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintTypealias(const AnyString& name, uint32_t atomid)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::typealias;
		operands.name   = stringrefs.ref(name);
		operands.atomid = atomid;
		operands.lvid   = 0u;
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintUnit(const AnyString& filename)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::unit;
		operands.name   = stringrefs.ref(filename);
		operands.atomid = static_cast<uint32_t>(-1);
		operands.lvid   = 0u;
		return offset;
	}

	inline void Sequence::emitBlueprintClass(const AnyString& name, uint32_t atomid)
	{
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::classdef;
		operands.name   = stringrefs.ref(name);
		operands.atomid = atomid;
		operands.lvid   = 0u;
	}

	inline uint32_t Sequence::emitBlueprintClass(uint32_t lvid)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::classdef;
		operands.name   = 0u;
		operands.atomid = static_cast<uint32_t>(-1);
		operands.setLVID(lvid);
		return offset;
	}

	inline void Sequence::emitBlueprintFunc(const AnyString& name, uint32_t atomid)
	{
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::funcdef;
		operands.name   = stringrefs.ref(name);
		operands.atomid = atomid;
		operands.lvid   = 0u;
	}

	inline uint32_t Sequence::emitBlueprintFunc()
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::funcdef;
		operands.name   = 0u;
		operands.atomid = static_cast<uint32_t>(-1);
		operands.lvid   = 0u;
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintGenericTypeParam(LVID lvid, const AnyString& name)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::gentypeparam;
		operands.name   = stringrefs.ref(name);
		operands.atomid = static_cast<uint32_t>(-1);
		operands.setLVID(lvid);
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintGenericTypeParam(LVID lvid)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::gentypeparam;
		operands.name   = 0;
		operands.atomid = static_cast<uint32_t>(-1);
		operands.setLVID(lvid);
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintParam(LVID lvid, const AnyString& name)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::param;
		operands.name   = stringrefs.ref(name);
		operands.atomid = static_cast<uint32_t>(-1);
		operands.setLVID(lvid);
		return offset;
	}

	inline uint32_t Sequence::emitBlueprintParam(LVID lvid)
	{
		uint32_t offset = m_size;
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::param;
		operands.name   = 0;
		operands.atomid = static_cast<uint32_t>(-1);
		operands.setLVID(lvid);
		return offset;
	}

	inline void Sequence::emitBlueprintVardef(LVID lvid, const AnyString& name)
	{
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::vardef;
		operands.name   = stringrefs.ref(name);
		operands.atomid = static_cast<uint32_t>(-1);
		operands.setLVID(lvid);
	}

	inline void Sequence::emitNamespace(const AnyString& name)
	{
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::namespacedef;
		operands.name   = stringrefs.ref(name);
		operands.atomid = static_cast<uint32_t>(-1);
		operands.lvid   = 0u;
	}

	inline void Sequence::emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS)
	{
		auto& operands = emit<ISA::Op::assign>();
		operands.lhs = lhs;
		operands.rhs = rhs;
		operands.disposelhs = canDisposeLHS;
	}

	inline void Sequence::emitFieldget(uint32_t lvid, uint32_t self, uint32_t fieldindex)
	{
		assert(lvid != 0 and self != 0);
		auto& operands = emit<ISA::Op::fieldget>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = fieldindex;
	}

	inline void Sequence::emitFieldset(uint32_t lvid, uint32_t self, uint32_t varid)
	{
		assert(lvid != 0 and self != 0);
		auto& operands = emit<ISA::Op::fieldset>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.var   = varid;
	}


} // namespace ir
} // namespace ny
