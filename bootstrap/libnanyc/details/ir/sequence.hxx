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

	inline void Sequence::emitTypeIsObject(uint32_t lvid)
	{
		emit<ISA::Op::typeisobject>().lvid = lvid;
	}

	inline void Sequence::emitInheritQualifiers(uint32_t lhs, uint32_t rhs)
	{
		auto& operands   = emit<ISA::Op::inherit>();
		operands.inherit = 2;
		operands.lhs     = lhs;
		operands.rhs     = rhs;
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

	inline void Sequence::emitCommonType(uint32_t lvid, uint32_t previous)
	{
		auto& opc = emit<ISA::Op::commontype>();
		opc.lvid = lvid;
		opc.previous = previous;
	}

	inline uint32_t Sequence::emitLabel(uint32_t labelid)
	{
		emit<ir::ISA::Op::label>().label = labelid;
		return labelid;
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

	inline uint32_t Sequence::emitStackalloc(uint32_t lvid, nytype_t type)
	{
		auto& operands = emit<ISA::Op::stackalloc>();
		operands.lvid   = lvid;
		operands.type   = static_cast<uint32_t>(type);
		operands.atomid = (uint32_t) -1;
		return lvid;
	}

	inline void Sequence::emitNamespace(const AnyString& name)
	{
		auto& operands  = emit<ISA::Op::blueprint>();
		operands.kind   = (uint32_t) ir::ISA::Blueprint::namespacedef;
		operands.name   = stringrefs.ref(name);
		operands.atomid = static_cast<uint32_t>(-1);
		operands.lvid   = 0u;
	}

	inline void Sequence::emitDebugpos(uint32_t line, uint32_t offset)
	{
		auto& operands  = emit<ISA::Op::debugpos>();
		operands.line   = line;
		operands.offset = offset;
	}

	inline uint32_t Sequence::emitMemalloc(uint32_t lvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memalloc>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
		return lvid;
	}

	inline void Sequence::emitMemrealloc(uint32_t lvid, uint32_t oldsize, uint32_t newsize)
	{
		auto& operands   = emit<ISA::Op::memrealloc>();
		operands.lvid    = lvid;
		operands.oldsize = oldsize;
		operands.newsize = newsize;
	}

	inline void Sequence::emitMemFree(uint32_t lvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memfree>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
	}

	inline void Sequence::emitMemFill(uint32_t lvid, uint32_t regsize, uint32_t pattern)
	{
		auto& operands   = emit<ISA::Op::memfill>();
		operands.lvid    = lvid;
		operands.regsize = regsize;
		operands.pattern = pattern;
	}

	inline void Sequence::emitMemCopy(uint32_t lvid, uint32_t srclvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memcopy>();
		operands.lvid    = lvid;
		operands.srclvid = srclvid;
		operands.regsize = regsize;
	}

	inline void Sequence::emitMemMove(uint32_t lvid, uint32_t srclvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memmove>();
		operands.lvid    = lvid;
		operands.srclvid = srclvid;
		operands.regsize = regsize;
	}

	inline void Sequence::emitMemCmp(uint32_t lvid, uint32_t srclvid, uint32_t regsize)
	{
		auto& operands   = emit<ISA::Op::memcmp>();
		operands.lvid    = lvid;
		operands.srclvid = srclvid;
		operands.regsize = regsize;
	}

	inline void Sequence::emitCStrlen(uint32_t lvid, uint32_t bits, uint32_t ptr)
	{
		auto& operands = emit<ISA::Op::cstrlen>();
		operands.lvid  = lvid;
		operands.bits  = bits;
		operands.ptr   = ptr;
	}

	inline void Sequence::emitLoadU64(uint32_t lvid, uint32_t addr)
	{
		auto& opr   = emit<ISA::Op::load_u64>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}
	inline void Sequence::emitLoadU32(uint32_t lvid, uint32_t addr)
	{
		auto& opr   = emit<ISA::Op::load_u32>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}
	inline void Sequence::emitLoadU8(uint32_t lvid, uint32_t addr)
	{
		auto& opr   = emit<ISA::Op::load_u8>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}

	inline void Sequence::emitStoreU64(uint32_t lvid, uint32_t addr)
	{
		auto& opr   = emit<ISA::Op::store_u64>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}
	inline void Sequence::emitStoreU32(uint32_t lvid, uint32_t addr)
	{
		auto& opr   = emit<ISA::Op::store_u32>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}
	inline void Sequence::emitStoreU8(uint32_t lvid, uint32_t addr)
	{
		auto& opr   = emit<ISA::Op::store_u8>();
		opr.lvid    = lvid;
		opr.ptrlvid = addr;
	}

	inline void Sequence::emitSizeof(uint32_t lvid, uint32_t type)
	{
		auto& operands = emit<ISA::Op::classdefsizeof>();
		operands.lvid  = lvid;
		operands.type  = type;
	}

	inline void Sequence::emitNameAlias(uint32_t lvid, const AnyString& name)
	{
		auto& operands = emit<ISA::Op::namealias>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}

	inline void Sequence::emitReturn()
	{
		auto& operands   = emit<ISA::Op::ret>();
		operands.lvid    = 0;
		operands.tmplvid = 0;
	}

	inline void Sequence::emitReturn(uint32_t lvid, uint32_t tmplvid)
	{
		auto& operands   = emit<ISA::Op::ret>();
		operands.lvid    = lvid;
		operands.tmplvid = tmplvid;
	}

	inline void Sequence::emitSelf(uint32_t self)
	{
		emit<ISA::Op::self>().self = self;
	}

	inline void Sequence::emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS)
	{
		auto& operands = emit<ISA::Op::assign>();
		operands.lhs = lhs;
		operands.rhs = rhs;
		operands.disposelhs = canDisposeLHS;
	}

	inline void Sequence::emitIdentify(uint32_t lvid, const AnyString& name, uint32_t self)
	{
		auto& operands = emit<ISA::Op::identify>();
		operands.lvid  = lvid;
		operands.self  = self;
		operands.text  = stringrefs.ref(name);
	}

	inline void Sequence::emitEnsureTypeResolved(uint32_t lvid)
	{
		emit<ISA::Op::ensureresolved>().lvid = lvid;
	}

	inline void Sequence::emitPush(uint32_t lvid)
	{
		auto& operands = emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = 0;
	}

	inline void Sequence::emitPush(uint32_t lvid, const AnyString& name)
	{
		assert(not name.empty());
		auto& operands = emit<ISA::Op::push>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}

	inline void Sequence::emitTPush(uint32_t lvid)
	{
		auto& operands = emit<ISA::Op::tpush>();
		operands.lvid  = lvid;
		operands.name  = 0;
	}

	inline void Sequence::emitTPush(uint32_t lvid, const AnyString& name)
	{
		assert(not name.empty());
		auto& operands = emit<ISA::Op::tpush>();
		operands.lvid  = lvid;
		operands.name  = stringrefs.ref(name);
	}

	inline void Sequence::emitCall(uint32_t lvid, uint32_t ptr2func)
	{
		auto& operands = emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = ptr2func;
		operands.instanceid = (uint32_t) -1;
	}

	inline void Sequence::emitCall(uint32_t lvid, uint32_t atomid, uint32_t instanceid)
	{
		assert(instanceid != (uint32_t) -1);
		auto& operands = emit<ISA::Op::call>();
		operands.lvid  = lvid;
		operands.ptr2func = atomid;
		operands.instanceid = instanceid;
	}

	inline void Sequence::emitIntrinsic(uint32_t lvid, const AnyString& name, uint32_t id)
	{
		auto& operands     = emit<ISA::Op::intrinsic>();
		operands.lvid      = lvid;
		operands.intrinsic = stringrefs.ref(name);
		operands.iid       = id;
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

	inline void Sequence::emitJmp(uint32_t label)
	{
		emit<ISA::Op::jmp>().label = label;
	}

	inline void Sequence::emitJz(uint32_t lvid, uint32_t result, uint32_t label)
	{
		auto& opc  = emit<ISA::Op::jz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}

	inline void Sequence::emitJnz(uint32_t lvid, uint32_t result, uint32_t label)
	{
		auto& opc  = emit<ISA::Op::jnz>();
		opc.lvid   = lvid;
		opc.result = result;
		opc.label  = label;
	}

	inline void Sequence::emitQualifierRef(uint32_t lvid, bool flag)
	{
		auto& operands = emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = ir::ISA::TypeQualifier::ref;
	}

	inline void Sequence::emitQualifierConst(uint32_t lvid, bool flag)
	{
		auto& operands = emit<ISA::Op::qualifiers>();
		operands.lvid  = lvid;
		operands.flag  = static_cast<uint32_t>(flag);
		operands.qualifier = ir::ISA::TypeQualifier::constant;
	}


} // namespace ir
} // namespace ny
