#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprReturn(const AST::Node& node)
	{
		assert(node.rule == AST::rgReturn);
		// assert(not node.children.empty()); -- a return may be empty

		bool success = true;
		bool hasReturnValue = false;
		auto& out = sequence();

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case AST::rgExpr:
				{
					IR::OpcodeScopeLocker opscope{out};
					LVID localvar;
					success &= visitASTExpr(child, localvar);

					// generate error on the begining of the expr and not the return itself
					emitDebugpos(child);

					uint32_t tmplvid = sequence().emitStackalloc(nextvar(), nyt_any);
					out.emitReturn(localvar, tmplvid);

					hasReturnValue = true;
					break;
				}

				default:
					return ICEUnexpectedNode(child, "[ir/return]");
			}
		}

		if (not hasReturnValue)
		{
			emitDebugpos(node);
			out.emitReturn();
		}
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
