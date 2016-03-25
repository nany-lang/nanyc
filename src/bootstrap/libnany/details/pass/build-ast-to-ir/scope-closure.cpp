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


	bool Scope::visitASTExprClosure(const AST::Node& node, uint32_t& localvar)
	{
		bool success = true;
		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				default:
					success = ICEUnexpectedNode(child, "[closure]");
			}
		}

		if (!context.reuse.closure.node)
			context.prepareReuseForClosures();

		AST::Node::Ptr expr = context.reuse.closure.node;

		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
