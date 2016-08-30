#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprTypeof(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgTypeof);

		// reset the value of the localvar, result of the expr
		localvar = 0;
		AST::Node* expr = nullptr;

		for (auto& child: node.children)
		{
			if (child.rule == AST::rgCall)
			{
				if (child.children.size() != 1)
				{
					error(child) << "invalid number of parameters for typeof-expression";
					return false;
				}
				auto& parameter = child.children[0];
				if (parameter.rule != AST::rgCallParameter or parameter.children.size() != 1)
					return unexpectedNode(parameter, "ir/typeof/param");

				expr = &(parameter.children[0]);
			}
			else
				return unexpectedNode(child, "[ir/typeof]");
		}

		if (unlikely(nullptr == expr))
			return (ice(node) << "invalid typeof expression");

		IR::Producer::Scope scope{*this};
		emitDebugpos(node);
		OpcodeCodegenDisabler codegen{sequence()};
		sequence().emitComment("typeof expression");
		return scope.visitASTExpr(*expr, localvar);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
