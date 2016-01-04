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

					if (child.rule == rgIf)
					{
						uint32_t localvar = 0;
						return visitASTExprIf(child, localvar);
					}

					if (child.rule == rgFunction)
						return visitASTFunc(child);
				}
				// no break here - same as `expr-group`
			}
			case rgExprGroup:
			{
				IR::OpcodeScopeLocker opscope{program()};
				LVID localvar;
				return visitASTExpr(node, localvar, /*allowScope:*/true);
			}

			case rgScope:  return visitASTExprScope(node);
			case rgVar:    return visitASTVar(node);
			case rgReturn: return visitASTExprReturn(node);
			case rgClass:  return visitASTClass(node);

			default: return ICEUnexpectedNode(node, "[ir/stmt]");
		}
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
