#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"

using namespace Yuni;

namespace ny::ir::Producer {

bool Scope::visitASTExprScope(AST::Node& node) {
	if (unlikely(kind != Kind::kfunc)) {
		switch (kind) {
			case Kind::kclass:
				error(node) << "scopes not allowed in class definition";
				break;
			default:
				error(node) << "unexpected scope declaration";
		}
		return false;
	}
	ir::emit::ScopeLocker opscope{ircode()};
	ir::Producer::Scope scope{*this};
	bool success = true;
	for (auto& child : node.children)
		success &= scope.visitASTStmt(child);
	return success;
}

} // ny::ir::Producer
