#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


bool Scope::visitASTExprSwitch(AST::Node& node) {
	assert(node.rule == AST::rgSwitch);
	bool success = true;
	auto& out = sequence();
	ir::emit::trace(out, "switch");
	ir::emit::ScopeLocker opscopeSwitch{out};
	emitDebugpos(node);
	// the variable id of the initial condition
	uint32_t valuelvid = 0;
	// a temporary variable to compute if a 'case' value matches or not
	// this variable is reused for each 'case'
	uint32_t casecondlvid = nextvar();
	ir::emit::alloc(out, casecondlvid, nyt_bool);
	// the current implementation generates a 'if' statement for each 'case'
	// these variables are for simulating an AST node
	auto exprCase = make_ref<AST::Node>(AST::rgExpr);
	auto cond = AST::createNodeIdentifier("^==");
	exprCase->children.push_back(cond);
	auto call = make_ref<AST::Node>(AST::rgCall);
	cond->children.push_back(call);
	// lhs
	auto lhs = make_ref<AST::Node>(AST::rgCallParameter);
	call->children.push_back(lhs);
	ShortString16 lvidstr;
	auto lhsExpr = make_ref<AST::Node>(AST::rgExpr);
	lhs->children.push_back(lhsExpr);
	auto lhsValue = make_ref<AST::Node>(AST::rgRegister);
	lhsExpr->children.push_back(lhsValue);
	auto rhs = make_ref<AST::Node>(AST::rgCallParameter);
	call->children.push_back(rhs);
	// using a scope for the body to have proper variable scoping
	AST::Node bodyScope{AST::rgScope};
	//! list of labels to update (to jump at the end of the switch-case when a cond matches)
	auto labels = std::make_unique<uint32_t[]>(node.children.size());
	uint32_t labelCount = 0;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgSwitchCase: {
				ir::emit::trace(out, "case");
				if (unlikely(valuelvid == 0))
					return (ice(child) << "switch: unexpected lvid value");
				if (unlikely(child.children.size() != 2))
					return unexpectedNode(child, "[ir/switch/case]");
				if (success) {
					ir::emit::ScopeLocker opscopeCase{out};
					rhs->children.clear();
					rhs->children.push_back(&(child.children[0]));
					bodyScope.children.clear();
					bodyScope.children.push_back(&(child.children[1]));
					success &= generateIfStmt(*exprCase, bodyScope, /*else*/nullptr, &(labels[labelCount]));
					++labelCount;
				}
				break;
			}
			case AST::rgSwitchExpr: {
				if (child.children.size() == 1) {
					auto& condition = child.children[0];
					emitDebugpos(condition);
					success &= visitASTExpr(condition, valuelvid, false);
					// updating lhs for operator ==
					lvidstr = valuelvid;
					lhsValue->text = lvidstr;
					break;
				}
				// do not break
			}
			default:
				return unexpectedNode(child, "[ir/switch]");
		}
	}
	emitDebugpos(node);
	uint32_t labelEnd = ir::emit::label(out, nextvar());
	// update all labels for jumping to the end
	for (uint32_t i = 0 ; i != labelCount; ++i)
		out.at<ir::isa::Op::jmp>(labels[i]).label = labelEnd;
	return success;
}


} // namespace Producer
} // namespace ir
} // namespace ny
