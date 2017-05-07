#include "sequence.h"
#include "details/ir/isa/printer.inc.hpp"
#include "details/atom/atom.h"
#include <cstdlib>

using namespace Yuni;


namespace ny {
namespace ir {


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
	stringrefs.clear();
	free(m_body);
	m_capacity = 0;
	m_body = nullptr;
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


} // namespace ir
} // namespace ny
