#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace ir {
namespace Producer {


bool Scope::visitASTStmt(AST::Node& orignode) {
	auto& node = (orignode.rule == AST::rgExpr
		and orignode.children.size() == 1 and orignode.children[0].rule == AST::rgExprValue)
		? orignode.children[0]
		: orignode;
	// invalidate the last identify opcode offset to avoid any invalid use
	lastIdentifyOpcOffset = 0;
	emitDebugpos(node);
	switch (node.rule) {
		// expressions
		case AST::rgExpr:
		case AST::rgExprValue: {
			// Special case: Standalone function
			// Normal functions are defined within a generic expression (but only from a stmt)
			//     expr
			//         function
			//             function-kind
			//             function-body
			//             [...]
			uint32_t size = node.children.size();
			AST::Node* child = nullptr;
			AST::Node* attrs = nullptr;
			switch (size) {
				case 1: {
					child = &(node.children[0]);
					break;
				}
				case 2: {
					attrs = &(node.children[0]);
					if (attrs->rule == AST::rgAttributes) {
						child = &(node.children[1]);
						if (child->rule == AST::rgExprValue and child->children.size() == 1)
							child = &(child->children[0]);
						else
							child = nullptr;
					}
					break;
				}
			}
			if (child) {
				switch (child->rule) {
					case AST::rgIf:
						return (not attrs ? true : visitASTAttributes(*attrs)) and visitASTExprIfStmt(*child);
					case AST::rgFunction:
						return (not attrs ? true : visitASTAttributes(*attrs)) and visitASTFunc(*child);
					case AST::rgSwitch:
						return (not attrs ? true : visitASTAttributes(*attrs)) and visitASTExprSwitch(*child);
					default: {
					}
				}
			}
			// no break here - same as `expr-group`
		}
		case AST::rgExprGroup: {
			ir::emit::ScopeLocker opscope{sequence()};
			uint32_t localvar = 0;
			return visitASTExpr(node, localvar, /*allowScope:*/true);
		}
		case AST::rgScope:
			return visitASTExprScope(node);
		case AST::rgWhile:
			return visitASTExprWhile(node);
		case AST::rgDoWhile:
			return visitASTExprDoWhile(node);
		case AST::rgVar:
			return visitASTVar(node);
		case AST::rgFor:
			return visitASTFor(node);
		case AST::rgReturn:
			return visitASTExprReturn(node);
		case AST::rgClass:
			return visitASTClass(node);
		case AST::rgTypedef:
			return visitASTTypedef(node);
		case AST::rgClassVisibility: /*currently ignored */
			return true;
		case AST::rgFunction:
			return visitASTFunc(node);
		default:
			return unexpectedNode(node, "[ir/stmt]");
	}
}


} // namespace Producer
} // namespace ir
} // namespace ny
