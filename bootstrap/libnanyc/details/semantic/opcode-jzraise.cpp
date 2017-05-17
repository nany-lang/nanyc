#include "semantic-analysis.h"

using namespace Yuni;


namespace ny {
namespace semantic {


void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::jzraise>& operands) {
	uint32_t label = operands.label;
	ir::emit::jzraise(out, label);
	bool hasErrorHandler = not onScopeFail.empty();
	if (hasErrorHandler) {
		releaseScopedVariables(onScopeFail.scope());
		onScopeFail.eachTypedErrorHandler([&](const OnScopeFailHandlers::Handler& handler, const Atom* atom) {
			uint32_t errlabel = handler.label;
			ir::emit::jmperrhandler(out, atom->atomid, errlabel);
		});
		if (not onScopeFail.any().empty())
			ir::emit::jmperrhandler(out, 0, onScopeFail.any().label);
	}
	else {
		releaseAllScopedVariables();
		ir::emit::ret(out);
	}
	ir::emit::label(out, label);
}


} // namespace semantic
} // namespace ny
