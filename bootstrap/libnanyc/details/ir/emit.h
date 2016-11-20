#pragma once
#include "sequence.h"
#include "details/ir/isa/data.h"


namespace ny
{
namespace ir
{
namespace emit
{
namespace
{

	struct SequenceRef final {
		SequenceRef(Sequence& sequence) :sequence(sequence) {}
		SequenceRef(Sequence* sequence) :sequence(*sequence) {
			assert(sequence != nullptr);
		}

		Sequence& sequence;
	};


	//! Copy two register
	inline void copy(SequenceRef ref, uint32_t lvid, uint32_t source) {
		auto& operands = ref.sequence.emit<ISA::Op::store>();
		operands.lvid = lvid;
		operands.source = source;
	}


	inline void constantu64(SequenceRef ref, uint32_t lvid, uint64_t value) {
		auto& operands = ref.sequence.emit<ISA::Op::storeConstant>();
		operands.lvid = lvid;
		operands.value.u64 = value;
	}


	inline void constantf64(SequenceRef ref, uint32_t lvid, double value) {
		auto& operands = ref.sequence.emit<ISA::Op::storeConstant>();
		operands.lvid  = lvid;
		operands.value.f64 = value;
	}


	inline void constantbool(SequenceRef ref, uint32_t lvid, bool value) {
		constantu64(ref, lvid, value ? 1 : 0);
	}


	inline uint32_t constantText(SequenceRef ref, uint32_t lvid, const AnyString& text) {
		auto& operands = ref.sequence.emit<ISA::Op::storeText>();
		operands.lvid = lvid;
		operands.text = ref.sequence.stringrefs.ref(text);
		return operands.text;
	}


	//! Allocate a new variable on the stack and assign a value to it and get the register
	inline uint32_t allocu64(SequenceRef ref, uint32_t lvid, nytype_t type, uint64_t value) {
		ref.sequence.emitStackalloc(lvid, type);
		ir::emit::constantu64(ref, lvid, value);
		return lvid;
	}
	

	//! Allocate a new variable on the stack and assign a value to it and get the register
	inline uint32_t allocf64(SequenceRef ref, uint32_t lvid, nytype_t type, double value) {
		ref.sequence.emitStackalloc(lvid, type);
		ir::emit::constantf64(ref, lvid, value);
		return lvid;
	}


	//! Allocate a new variable on the stack and assign a text to it and get the register
	inline uint32_t alloctext(SequenceRef ref, uint32_t lvid, const AnyString& text) {
		ref.sequence.emitStackalloc(lvid, nyt_ptr);
		ir::emit::constantText(ref, lvid, text);
		return lvid;
	}


} // namespace
} // namespace emit
} // namespace ir
} // namespace ny
