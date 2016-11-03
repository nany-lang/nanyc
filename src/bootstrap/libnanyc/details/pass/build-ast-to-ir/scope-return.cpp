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


	bool Scope::visitASTExprReturn(AST::Node& node)
	{
		assert(node.rule == AST::rgReturn);
		// assert(not node.children.empty()); -- a return may be empty

		bool success = true;
		bool hasReturnValue = false;
		auto& out = sequence();

		for (auto& child: node.children)
		{
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
					return unexpectedNode(child, "[ir/return]");
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
