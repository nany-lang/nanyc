#include "scope.h"
#include "details/ir/emit.h"

using namespace Yuni;

namespace ny {
namespace ir {
namespace Producer {

namespace {

bool onScopeExit(Scope& scope, AST::Node& node, AST::Node& scopeNode) {
	if (!scope.context.reuse.scope.exit.node)
		scope.context.reuse.prepareReuseForScopeExit();
	auto& irout = scope.ircode();
	scope.emitDebugpos(node);
	uint32_t varlvid = ir::emit::alloc(irout, scope.nextvar());
	ir::emit::type::qualifierRef(irout, varlvid, /*ref*/ true);
	{
		auto& body = *(scope.context.reuse.scope.exit.body);
		body.children.push_back(&scopeNode);
		uint32_t rhs = 0;
		ir::emit::ScopeLocker opscope{irout};
		bool success = scope.visitASTExpr(*scope.context.reuse.scope.exit.node, rhs);
		body.clear();
		if (unlikely(not success))
			return false;
		ir::emit::assign(irout, varlvid, rhs, false);
	}
	return true;
}

bool onScope(Scope& scope, AST::Node& node) {
	AST::Node* scopeNode = nullptr;
	AST::Node* scopeFail = nullptr;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgScope: scopeNode = &child; break;
			case AST::rgOnScopeFail: scopeFail = &child; break;
			default: return unexpectedNode(child, "[on/scope]");
		}
	}
	if (unlikely(scopeNode == nullptr))
		return ice(node) << "ast node 'scope' expected";
	return (scopeFail != nullptr)
		? scope.visitASTExprOnScopeFail(*scopeNode, *scopeFail)
		: onScopeExit(scope, node, *scopeNode);
}

} // namespace

bool Scope::visitASTExprOn(AST::Node& node, uint32_t& /*localvar*/, bool isStmt) {
	assert(node.rule == AST::rgOn);
	bool success = true;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgOnScope: {
				if (unlikely(not isStmt))
					return error(child) << "'on scope' cannot be used in an expression";
				success &= onScope(*this, child);
				break;
			}
			default:
				return unexpectedNode(child, "[on]");
		}
	}
	return success;
}

} // namespace Producer
} // namespace ir
} // namespace ny
