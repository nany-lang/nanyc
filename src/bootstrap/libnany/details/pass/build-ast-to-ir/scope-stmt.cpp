#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTStmt(const AST::Node& orignode)
	{
		auto& node = (orignode.rule == AST::rgExpr
			and orignode.children.size() == 1 and orignode.children[0]->rule == AST::rgExprValue)
			? *(orignode.children[0])
			: orignode;

		emitDebugpos(node);
		switch (node.rule)
		{
			// expressions
			case AST::rgExpr:
			case AST::rgExprValue:
			{
				// Special case: Standalone function
				// Normal functions are defined within a generic expression (but only from a stmt)
				//     expr
				//         function
				//             function-kind
				//             function-body
				//             [...]
				if (node.children.size() == 1)
				{
					auto& child = *(node.children[0]);
					switch (child.rule)
					{
						case AST::rgIf:       return visitASTExprIfStmt(child);
						case AST::rgFunction: return visitASTFunc(child);
						case AST::rgSwitch:   return visitASTExprSwitch(child);
						default: {}
					}
				}
				// no break here - same as `expr-group`
			}
			case AST::rgExprGroup:
			{
				IR::OpcodeScopeLocker opscope{sequence()};
				LVID localvar;
				return visitASTExpr(node, localvar, /*allowScope:*/true);
			}

			case AST::rgScope:   return visitASTExprScope(node);
			case AST::rgWhile:   return visitASTExprWhile(node);
			case AST::rgDoWhile: return visitASTExprDoWhile(node);
			case AST::rgVar:     return visitASTVar(node);
			case AST::rgFor:     return visitASTFor(node);
			case AST::rgReturn:  return visitASTExprReturn(node);
			case AST::rgClass:   return visitASTClass(node);
			case AST::rgTypedef: return visitASTTypedef(node);

			case AST::rgClassVisibility: /*currently ignored */ return true;

			default: return ICEUnexpectedNode(node, "[ir/stmt]");
		}
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
