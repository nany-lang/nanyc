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
	auto& irout = ircode();
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgExpr: {
				ir::emit::ScopeLocker opscope{irout};
				uint32_t retlvid = 0;
				success &= visitASTExpr(child, retlvid);
				// generate error on the begining of the expr and not the return itself
				emitDebugpos(child);
				ir::emit::ret(irout, retlvid, ir::emit::alloc(irout, nextvar()));
				hasReturnValue = true;
				break;
			}
			default:
				return unexpectedNode(child, "[ir/return]");
		}
	}
	if (not hasReturnValue) {
		emitDebugpos(node);
		ir::emit::ret(irout);
	}
	return success;
}


} // namespace Producer
} // namespace ir
} // namespace ny
