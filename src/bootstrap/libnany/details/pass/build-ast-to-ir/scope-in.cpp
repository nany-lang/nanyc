#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprIn(const AST::Node& node, LVID& localvar, AnyString& elementname)
	{
		// lvid representing the input container
		AST::Node* container = nullptr;
		AST::Node* predicate = nullptr;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgInVars:
				{
					if (unlikely(child.children.size() != 1))
						return error(child) << "only one cursor name is allowed in views";
					auto& identifier = child.firstChild();
					if (unlikely(identifier.rule != AST::rgIdentifier))
						return ICE(identifier) << "identifier expected";

					elementname = identifier.text;
					break;
				}

				case AST::rgInContainer:
				{
					if (unlikely(child.children.size() != 1))
						return ICE(child) << "invalid expression";

					container = &child.firstChild();
					break;
				}

				case AST::rgInPredicate:
				{
					if (unlikely(child.children.size() != 1))
						return ICE(child) << "invalid predicate expr";
					auto& expr = child.firstChild();
					if (unlikely(expr.rule != AST::rgExpr))
						return ICE(expr) << "invalid node type for predicate";

					warning(expr) << "predicates in views are not fully implemented";
					predicate = &expr;
					break;
				}
				default:
					return ICEUnexpectedNode(child, "[expr/in]");
			}
		}

		if (unlikely(!container))
			return ICE(node) << "invalid view container expr";

		if (!context.reuse.inset.node)
			context.prepareReuseForIn();

		context.reuse.inset.container->children.clear();
		context.reuse.inset.container->children.push_back(container);
		context.reuse.inset.viewname->text = "^view^default";

		if (elementname.empty())
			elementname = "_";
		context.reuse.inset.elementname->text = elementname;

		if (!predicate)
			predicate = AST::Node::Ptr::WeakPointer(context.reuse.inset.premadeAlwaysTrue);
		context.reuse.inset.predicate->children.clear();
		context.reuse.inset.predicate->children.push_back(predicate);

		return visitASTExpr(*context.reuse.inset.node, localvar);
	}


	bool Scope::visitASTExprIn(const AST::Node& node, LVID& localvar)
	{
		// Name of the target ref for each element in the container
		AnyString elementname;
		return visitASTExprIn(node, localvar, elementname);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
