#include "sequence.h"
#include "details/ir/isa/printer.inc.hpp"
#include "details/atom/atom.h"
#include <cstdlib>

using namespace Yuni;

namespace ny::ir {

Sequence::Sequence(const Sequence& other, uint32_t offset)
		: stringrefs(other.stringrefs) {
	assert(offset < other.m_size);
	uint32_t size = other.m_size - offset;
	if (size != 0) {
		grow(size);
		m_size = size;
		YUNI_MEMCPY(m_body, sizeof(Instruction) * m_capacity, other.m_body + offset, size * sizeof(Instruction));
	}
}

Sequence::~Sequence() {
	free(m_body);
}

void Sequence::moveCursorFromBlueprintToEnd(const Instruction*& cursor) const {
	assert((*cursor).opcodes[0] == static_cast<uint32_t>(ir::isa::Op::blueprint));
	if ((*cursor).opcodes[0] == static_cast<uint32_t>(ir::isa::Op::blueprint)) {
		// next opcode, which should be blueprint.size
		(cursor)++;
		// getting the size and moving the cursor
		auto& blueprintsize = (*cursor).to<ir::isa::Op::pragma>();
		assert(blueprintsize.opcode == (uint32_t) ir::isa::Op::pragma);
		assert(blueprintsize.value.blueprintsize >= 2);
		cursor += blueprintsize.value.blueprintsize - 2; // -2: blueprint:class+blueprint:size
		assert((*cursor).opcodes[0] == (uint32_t) ir::isa::Op::end);
	}
}

void Sequence::moveCursorFromBlueprintToEnd(Instruction*& cursor) const {
	assert((*cursor).opcodes[0] == static_cast<uint32_t>(ir::isa::Op::blueprint));
	if ((*cursor).opcodes[0] == static_cast<uint32_t>(ir::isa::Op::blueprint)) {
		// next opcode, which should be blueprint.size
		(cursor)++;
		// getting the size and moving the cursor
		auto& blueprintsize = (*cursor).to<ir::isa::Op::pragma>();
		assert(blueprintsize.opcode == (uint32_t) ir::isa::Op::pragma);
		assert(blueprintsize.value.blueprintsize >= 2);
		cursor += blueprintsize.value.blueprintsize - 2; // -2: blueprint:class+blueprint:size
		assert((*cursor).opcodes[0] == (uint32_t) ir::isa::Op::end);
	}
}

void Sequence::clear() {
	m_size = 0;
	free(m_body);
	m_capacity = 0;
	m_body = nullptr;
	stringrefs.clear();
}

void Sequence::grow(uint32_t count) {
	assert(count > 0);
	uint32_t newCapacity = m_capacity;
	do {
		newCapacity += 1000u;
	}
	while (newCapacity < count);
	auto* newbody = (Instruction*) realloc(m_body, sizeof(Instruction) * newCapacity);
	if (unlikely(nullptr == newbody))
		throw std::bad_alloc();
	m_body = newbody;
	m_capacity = newCapacity;
}

void Sequence::print(YString& out, const AtomMap* atommap, uint32_t offset) const {
	out.reserve(out.size() + (m_size * 100)); // arbitrary
	ir::isa::Printer<String> printer{out, *this};
	printer.atommap = atommap;
	printer.offset = offset;
	each(printer, offset);
}

namespace {

struct WalkerIncreaseLVID final {
	WalkerIncreaseLVID(ir::Sequence& ircode, uint32_t inc, uint32_t greaterThan)
		: greaterThan(greaterThan)
		, inc(inc)
		, ircode(ircode) {
	}

	void visit(ir::isa::Operand<ir::isa::Op::stacksize>& operands) {
		operands.add += inc;
	}

	void visit(ir::isa::Operand<ir::isa::Op::blueprint>& operands) {
		auto kind = static_cast<ir::isa::Blueprint>(operands.kind);
		switch (kind) {
			case ir::isa::Blueprint::funcdef:
			case ir::isa::Blueprint::classdef:
				ircode.moveCursorFromBlueprintToEnd(*cursor);
				break;
			default:
				operands.eachLVID(*this);
				break;
		}
	}

	void visit(ir::isa::Operand<ir::isa::Op::scope>&) {
		++depth;
	}

	void visit(ir::isa::Operand<ir::isa::Op::end>&) {
		if (depth-- == 0)
			ircode.invalidateCursor(*cursor);
	}

	template<ir::isa::Op O> void visit(ir::isa::Operand<O>& operands) {
		// ask to the opcode datatype to update its own lvid
		// (see operator() below)
		operands.eachLVID(*this);
	}

	inline void operator () (uint32_t& lvid) const {
		if (lvid > greaterThan)
			lvid += inc;
	}

	inline void operator () (uint32_t& lvid1, uint32_t& lvid2) const {
		if (lvid1 > greaterThan)
			lvid1 += inc;
		if (lvid2 > greaterThan)
			lvid2 += inc;
	}

	inline void operator () (uint32_t& lvid1, uint32_t& lvid2, uint32_t& lvid3) const {
		if (lvid1 > greaterThan)
			lvid1 += inc;
		if (lvid2 > greaterThan)
			lvid2 += inc;
		if (lvid3 > greaterThan)
			lvid3 += inc;
	}

	uint32_t greaterThan;
	uint32_t inc;
	uint32_t depth = 0;
	Instruction** cursor = nullptr;
	Sequence& ircode;
};

} // anonymous namespace

void Sequence::increaseAllLVID(uint32_t inc, uint32_t greaterThan, uint32_t offset) {
	assert(inc > 0 and "this method should not be called if nothing to do");
	WalkerIncreaseLVID walker{*this, inc, greaterThan};
	each(walker, offset);
}

void Sequence::invalidateCursor(const Instruction*& cursor) const {
	cursor = m_body + m_size;
}

void Sequence::invalidateCursor(Instruction*& cursor) const {
	cursor = m_body + m_size;
}

bool Sequence::jumpToLabelForward(const Instruction*& cursor, uint32_t label) const {
	const auto* const end = m_body + m_size;
	const Instruction* instr = cursor;
	while (++instr < end) {
		if (instr->opcodes[0] == static_cast<uint32_t>(ir::isa::Op::label)) {
			auto& operands = (*instr).to<ir::isa::Op::label>();
			if (operands.label == label) {
				cursor = instr;
				return true;
			}
		}
	}
	// not found - the cursor is alreayd invalidated
	return false;
}

bool Sequence::jumpToLabelBackward(const Instruction*& cursor, uint32_t label) const {
	const auto* const base = m_body;
	const Instruction* instr = cursor;
	while (instr-- > base) {
		if (instr->opcodes[0] == static_cast<uint32_t>(ir::isa::Op::label)) {
			auto& operands = (*instr).to<ir::isa::Op::label>();
			if (operands.label == label) {
				cursor = instr;
				return true;
			}
		}
	}
	// not found - invalidate
	return false;
}

bool Sequence::isCursorValid(const Instruction& instr) const {
	return (m_size > 0 and m_capacity > 0)
		and (&instr >= m_body)
		and (&instr <  m_body + m_size);
}

uint32_t Sequence::offsetOf(const Instruction& instr) const {
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

} // ny::ir
