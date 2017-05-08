#include "semantic-analysis.h"

using namespace Yuni;


namespace ny {
namespace semantic {


void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::raise>& operands) {
	auto& localfunc = frame->atom;
	if (unlikely(not localfunc.isFunction()))
		return (void)(error() << "errors can only be raised from a function body");
	if (unlikely(localfunc.isDtor()))
		return (void)(error() << "'raise' not allowed in destructor");
	auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
	auto* atomError = cdeftable.findClassdefAtom(cdef);
	if (atomError == nullptr)
		return (void)(error() << "only user-defined classes can be used for raising an error");
	ir::emit::ref(out, operands.lvid);
	uint32_t labelid = 0;
	if (onScopeFail.empty()) {
		// not within an error handler defined by the current function
		frame->atom.funcinfo.raisedErrors.add(*atomError, frame->atom, currentLine, currentOffset);
		releaseAllScopedVariables();
	}
	else {
		// within an error handler defined by the function
		auto* handler = onScopeFail.find(atomError);
		if (unlikely(handler == nullptr))
			return complainNoErrorHandler(*atomError, nullptr, {});
		handler->used = true;
		labelid = handler->label;
		releaseScopedVariables(handler->scope, /*forget*/ true);
	}
	ir::emit::raise(out, operands.lvid, labelid);
}


} // namespace semantic
} // namespace ny
