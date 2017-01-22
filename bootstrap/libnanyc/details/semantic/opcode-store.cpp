#include "semantic-analysis.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace semantic {


void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::storeConstant>& operands) {
	assert(frame != nullptr);
	frame->lvids(operands.lvid).synthetic = false;
	if (canGenerateCode())
		ir::emit::constantu64(out, operands.lvid, operands.value.u64);
}


void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::store>& operands) {
	assert(frame != nullptr);
	frame->lvids(operands.lvid).synthetic = false;
	if (not frame->verify(operands.source))
		return frame->invalidate(operands.lvid);
	auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
	if (cdef.isAny()) {
		// type propagation
		auto& cdefsrc = cdeftable.classdef(CLID{frame->atomid, operands.source});
		auto& spare = cdeftable.substitute(cdef.clid.lvid());
		spare.import(cdefsrc);
	}
	if (canGenerateCode())
		ir::emit::copy(out, operands.lvid, operands.source);
}


void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::storeText>& operands) {
	if (canGenerateCode()) {
		uint32_t sid = ir::emit::constantText(out, operands.lvid, currentSequence.stringrefs[operands.text]);
		auto& lvidinfo = frame->lvids(operands.lvid);
		lvidinfo.synthetic = false;
		lvidinfo.text_sid  = sid;
	}
}


} // namespace semantic
} // namespace ny
