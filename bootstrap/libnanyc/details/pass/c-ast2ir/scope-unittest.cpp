#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"
#include "details/ast/ast.h"
#include <iostream>

using namespace Yuni;




namespace ny
{
namespace ir
{
namespace Producer
{


	bool Scope::visitASTUnitTest(AST::Node& node)
	{
		assert(node.rule == AST::rgUnittest);

		// invalidate the last identify opcode offset to avoid any invalid use
		lastIdentifyOpcOffset = 0;
		AST::Node* scope  = nullptr;
		String testname;
		testname.reserve(512);
		testname += "^unittest^module:";

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgEntity:
				{
					if (not AST::appendEntityAsString(testname, child))
						return (error(child) << "invalid unittest name");
					break;
				}
				case AST::rgScope:
				{
					scope = &child;
					break;
				}
				default:
					return unexpectedNode(child, "[unittest]");
			}
		}

		if (unlikely(!scope))
			return (ice(node) << "invalid unittest ast declaration");
		if (unlikely(testname.empty()))
			return (error(node) << "invalid empty unittest name");

		if (context.cf.on_unittest)
		{
			AnyString name{testname, 17};
			context.cf.on_unittest(context.cf.userdata, "<nomodule>", 10, name.c_str(), name.size());
		}
		if (unlikely(context.ignoreAtoms))
			return true;

		if (!context.reuse.unittest.node)
			context.prepareReuseForUnittest();

		context.reuse.unittest.funcname->text = testname;
		context.reuse.unittest.funcbody->children.push_back(scope);

		bool success = visitASTFunc(*(context.reuse.unittest.node));

		// avoid crap in the debugger
		context.reuse.unittest.funcbody->children.clear();
		context.reuse.unittest.funcname->text.clear();
		return success;
	}




} // namespace Producer
} // namespace ir
} // namespace ny
