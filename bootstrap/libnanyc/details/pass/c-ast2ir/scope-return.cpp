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
	uint32_t retlvid = 0;
	auto& irout = ircode();
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgExpr: {
				ir::emit::ScopeLocker opscope{irout};
				success &= visitASTExpr(child, retlvid);
				// generate error on the begining of the expr and not the return itself
				emitDebugpos(child);
				ir::emit::ret(irout, retlvid, ir::emit::alloc(irout, nextvar()));
				break;
			}
			default:
				return unexpectedNode(child, "[ir/return]");
		}
	}
	if (retlvid == 0) {
		emitDebugpos(node);
		ir::emit::ret(irout); // return void
	}
	return success;
}


} // namespace Producer
} // namespace ir
} // namespace ny
