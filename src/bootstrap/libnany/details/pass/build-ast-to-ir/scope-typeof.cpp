#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "libnany-config.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprTypeof(const AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgTypeof);

		// reset the value of the localvar, result of the expr
		localvar = 0;
		AST::Node* expr = nullptr;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			if (child.rule == AST::rgCall)
			{
				if (child.children.size() != 1)
				{
					error(child) << "invalid number of parameters for typeof-expression";
					return false;
				}
				auto& parameter = *(child.children[0]);
				if (parameter.rule != AST::rgCallParameter or parameter.children.size() != 1)
					return ICEUnexpectedNode(parameter, "ir/typeof/param");

				expr = AST::Node::Ptr::WeakPointer(parameter.children[0]);
			}
			else
				return ICEUnexpectedNode(child, "[ir/typeof]");
		}

		if (unlikely(nullptr == expr))
			return (ICE(node) << "invalid typeof expression");

		IR::Producer::Scope scope{*this};
		OpcodeCodegenDisabler codegen{sequence()};
		sequence().emitComment("typeof expression");
		return scope.visitASTExpr(*expr, localvar);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
