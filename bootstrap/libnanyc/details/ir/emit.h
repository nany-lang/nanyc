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


} // namespace
} // namespace emit
} // namespace ir
} // namespace ny
