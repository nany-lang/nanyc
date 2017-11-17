#include "semantic-analysis.h"

using namespace Yuni;

namespace ny::semantic {

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::assign>& operands) {
	assert(frame != nullptr);
	frame->lvids(operands.lhs).synthetic = false;
	if (not frame->verify(operands.rhs))
		return frame->invalidate(operands.lhs);
	auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lhs});
	if (cdef.isAny()) {
		// type propagation
		auto& cdefsrc = cdeftable.classdef(CLID{frame->atomid, operands.rhs});
		auto& spare = cdeftable.substitute(cdef.clid.lvid());
		spare.import(cdefsrc);
	}
	bool canDisposeLHS = (operands.disposelhs != 0);
	bool r = instanciateAssignment(*frame, operands.lhs, operands.rhs, canDisposeLHS);
	if (unlikely(not r))
		frame->invalidate(operands.lhs);
}

} // ny::semantic
