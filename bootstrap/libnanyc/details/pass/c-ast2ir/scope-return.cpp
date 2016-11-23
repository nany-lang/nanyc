#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


bool Scope::visitASTExprReturn(AST::Node& node) {
	assert(node.rule == AST::rgReturn);
	// assert(not node.children.empty()); -- a return may be empty
	bool success = true;
	bool hasReturnValue = false;
	auto& out = sequence();
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgExpr: {
				ir::emit::ScopeLocker opscope{out};
				uint32_t localvar = 0;
				success &= visitASTExpr(child, localvar);
				// generate error on the begining of the expr and not the return itself
				emitDebugpos(child);
				ir::emit::ret(out, localvar, ir::emit::alloc(out, nextvar()));
				hasReturnValue = true;
				break;
			}
			default:
				return unexpectedNode(child, "[ir/return]");
		}
	}
	if (not hasReturnValue) {
		emitDebugpos(node);
		ir::emit::ret(out);
	}
	return success;
}


} // namespace Producer
} // namespace ir
} // namespace ny
