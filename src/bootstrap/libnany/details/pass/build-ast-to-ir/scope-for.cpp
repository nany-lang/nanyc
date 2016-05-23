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


	bool Scope::visitASTFor(const AST::Node& node)
	{
		// Name of the target ref for each element in the container
		ShortString128 elementname;
		// output sequence
		auto& out = sequence();
		// lvid of the view
		uint32_t viewlvid = 0;
		// 'do' clause
		AST::Node* forDoClause = nullptr;


		if (debugmode)
			out.emitComment("for");

		OpcodeScopeLocker opscopeElse{out};

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgIn:
				{
					//
					// Get the view
					//
					bool ok = visitASTExprIn(child, viewlvid, elementname);
					if (unlikely(not ok))
						return false;

					if (unlikely(viewlvid == 0 or viewlvid == (uint32_t) -1))
						return ICE(child) << "invalid container lvid";
					break;
				}
				case AST::rgForDo:
				{
					forDoClause = &child;
					break;
				}
				default:
					return ICEUnexpectedNode(child, "[for]");
			}
		}

		if (unlikely(elementname.empty()))
			return error(node) << "unnamed iterator currently not supported";

		if (!context.reuse.loops.node)
			context.prepareReuseForLoops();

		emitDebugpos(node);
		ShortString16 lvidstr;
		lvidstr << viewlvid;
		for (auto*& mnode: context.reuse.loops.viewlvid)
			mnode->text = lvidstr;

		ShortString32 cursorname;
		cursorname << "%cursor_" << lvidstr;
		for (auto*& mnode: context.reuse.loops.cursorname)
			mnode->text = cursorname;

		context.reuse.loops.elementname->text = elementname;

		if (forDoClause)
			context.reuse.loops.scope->children = forDoClause->children;
		else
			context.reuse.loops.scope->children.clear();

		return visitASTStmt(*context.reuse.loops.node);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
