#include "semantic-analysis.h"

using namespace Yuni;


namespace ny {
namespace semantic {


void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::onscopefail>& operands) {
	assert(frame != nullptr);
	if (unlikely(frame->atom.type != Atom::Type::funcdef))
		return (void)(error() << "error handlers can only be defined in a function body");
	Atom* atomError = nullptr;
	bool hasTypedParameter = operands.lvid != 0;
	if (hasTypedParameter) {
		auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
		if (cdef.isAny()) {
			return (void)(error() << "'on scope fail' does not accept 'any' parameter. Type required.");
		if (unlikely(cdef.isVoid()))
			return (void)(error() << "'on scope fail' does not accept void");
		if (unlikely(cdef.isBuiltin()))
			return (void)(error() << "'on scope fail' does not accept builtin types");
		atomError = cdeftable.findClassdefAtom(cdef);
		if (atomError == nullptr)
			return (void)(ice() << "'on scope fail' with null atom");
	}
	bool registering = operands.label != 0;
	if (registering) {
		if (atomError == nullptr) {
			auto& any = onScopeFail.any();
			if (not any.empty())
				return (void)(error() << "error handler 'any' already defined");
			any.reset(operands.lvid, operands.label, frame->scope);
		}
		else {
			bool alreadyExists = onScopeFail.has(atomError);
			// always keep track of the error handler and to not produce
			// an error when deregistering later
			auto& handler = onScopeFail.add(atomError, operands.lvid, operands.label, frame->scope);
			if (unlikely(alreadyExists)) {
				error() << "error handler '" << atomError->caption(cdeftable) << "' already defined for the scope";
				handler.used = true; // no warning report
			}
		}
	}
	else {
		if (atomError == nullptr) {
			auto& any = onScopeFail.any();
			if (unlikely(not any.used))
				warning() << "unused error handler for 'any'";
			any.reset();
		}
		else {
			if (unlikely(not onScopeFail.hasBackTypedHandler(atomError))) {
				ice() << "the error '" << atomError->caption(cdeftable) << "' was not registered";
			}
			auto& handler = onScopeFail.back();
			if (unlikely(not handler.used))
				warning() << "unused error handler for '" << atomError->keyword() << ' ' << atomError->caption(cdeftable) << '\'';
			onScopeFail.pop_back();
		}
	}
}


} // namespace semantic
} // namespace ny
