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


	bool Scope::visitASTFor(AST::Node& node)
	{
		// Name of the target ref for each element in the container
		ShortString128 elementname;
		// output sequence
		auto& out = sequence();
		// lvid of the view
		uint32_t viewlvid = 0;
		// 'do' clause
		AST::Node* forDoClause = nullptr;
		AST::Node* forElseClause = nullptr;

		if (debugmode)
			out.emitComment("for");

		OpcodeScopeLocker opscopeElse{out};

		for (auto& child: node.children)
		{
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
						return ice(child) << "invalid container lvid";
					break;
				}
				case AST::rgForDo:
				{
					forDoClause = &child;
					break;
				}
				case AST::rgForElse:
				{
					forElseClause = &child;
					break;
				}
				default:
					return unexpectedNode(child, "[for]");
			}
		}

		if (unlikely(elementname.empty()))
			return error(node) << "unnamed iterator currently not supported";

		if (!context.reuse.loops.node)
			context.prepareReuseForLoops();

		emitDebugpos(node);
		ShortString16 lvidstr;
		lvidstr << viewlvid;
		context.reuse.loops.viewlvid->text = lvidstr;

		ShortString32 cursorname;
		cursorname << "%cursor_" << lvidstr;
		AnyString crname = cursorname;
		for (auto*& mnode: context.reuse.loops.cursorname)
			mnode->text = crname;

		context.reuse.loops.elementname->text = elementname;

		auto& scopeChildren = context.reuse.loops.scope->children;
		if (forDoClause)
			scopeChildren = forDoClause->children;

		if (forElseClause)
		{
			context.reuse.loops.ifnode->children.push_back(context.reuse.loops.elseClause);
			context.reuse.loops.elseScope->children = forElseClause->children;
		}


		bool success = visitASTStmt(*context.reuse.loops.node);

		// cleanup
		if (forDoClause)
			scopeChildren.clear();
		if (forElseClause)
		{
			context.reuse.loops.ifnode->children.pop_back();
			context.reuse.loops.elseScope->children.clear();
		}

		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
