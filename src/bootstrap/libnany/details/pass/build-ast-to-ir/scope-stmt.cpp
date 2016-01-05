#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTStmt(Node& node)
	{
		switch (node.rule)
		{
			// expressions
			case rgExpr:
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
						case rgIf:
						{
							return visitASTExprIfStmt(child);
						}
						case rgFunction:
						{
							return visitASTFunc(child);
						}
						case rgSwitch:
						{
							return visitASTExprSwitch(child);
						}
						default:
						{}
					}
				}
				// no break here - same as `expr-group`
			}
			case rgExprGroup:
			{
				IR::OpcodeScopeLocker opscope{program()};
				LVID localvar;
				return visitASTExpr(node, localvar, /*allowScope:*/true);
			}

			case rgScope:   return visitASTExprScope(node);
			case rgWhile:   return visitASTExprWhile(node);
			case rgDoWhile: return visitASTExprDoWhile(node);
			case rgVar:     return visitASTVar(node);
			case rgReturn:  return visitASTExprReturn(node);
			case rgClass:   return visitASTClass(node);

			default: return ICEUnexpectedNode(node, "[ir/stmt]");
		}
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
