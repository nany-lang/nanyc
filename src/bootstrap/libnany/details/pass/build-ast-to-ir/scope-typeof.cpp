#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{

	bool Scope::visitASTExprTypeof(Node& node, LVID& localvar)
	{
		assert(node.rule == rgTypeof);

		// reset the value of the localvar, result of the expr
		localvar = 0;
		Node* expr = nullptr;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgCall:
				{
					if (child.children.size() != 1)
					{
						error(child) << "invalid number of parameters for typeof-expression";
						return false;
					}
					auto& parameter = *(child.children[0]);
					if (parameter.rule != rgCallParameter or parameter.children.size() != 1)
						return ICEUnexpectedNode(parameter, "ir/typeof/param");

					expr = Node::Ptr::WeakPointer(parameter.children[0]);
					break;
				}

				default:
					return ICEUnexpectedNode(child, "[ir/typeof]");
			}
		}

		if (nullptr == expr)
		{
			error(node) << "invalid typeof expression";
			return false;
		}

		IR::Producer::Scope scope{*this};
		OpcodeCodegenDisabler codegen{program()};
		program().emitComment("typeof expression");
		return scope.visitASTExpr(*expr, localvar);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
