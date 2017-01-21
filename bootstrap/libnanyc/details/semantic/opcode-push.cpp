#include "semantic-analysis.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::push>& operands) {
	bool verified = (frame->verify(operands.lvid));
	// always push the parameter to have a consistent output
	if (0 == operands.name) {
		pushedparams.func.indexed.emplace_back(operands.lvid, currentLine, currentOffset);
		if (verified and unlikely(frame->lvids(operands.lvid).synthetic)) {
			auto pindex = static_cast<uint32_t>(pushedparams.func.indexed.size());
			complainPushedSynthetic(CLID{frame->atomid, operands.lvid}, pindex);
		}
	}
	else {
		const auto& name = currentSequence.stringrefs[operands.name];
		pushedparams.func.named.emplace_back(name, operands.lvid, currentLine, currentOffset);
		if (verified and unlikely(frame->lvids(operands.lvid).synthetic))
			complainPushedSynthetic(CLID{frame->atomid, operands.lvid}, 0, name);
	}
}


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::tpush>& operands) {
	frame->verify(operands.lvid);
	if (0 == operands.name)
		pushedparams.gentypes.indexed.emplace_back(operands.lvid, currentLine, currentOffset);
	else {
		const auto& name = currentSequence.stringrefs[operands.name];
		pushedparams.gentypes.named.emplace_back(name, operands.lvid, currentLine, currentOffset);
	}
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
