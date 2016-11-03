#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprIn(AST::Node& node, LVID& localvar, ShortString128& elementname)
	{
		// lvid representing the input container
		AST::Node* container = nullptr;
		AST::Node* predicate = nullptr;
		AnyString requestedViewName = "default";
		AST::Node* additionalParams = nullptr;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgInVars:
				{
					if (unlikely(child.children.size() != 1))
						return error(child) << "only one cursor name is allowed in views";
					auto& identifier = child.firstChild();
					if (unlikely(identifier.rule != AST::rgIdentifier))
						return ice(identifier) << "identifier expected";
					if (unlikely(not checkForValidIdentifierName(child, identifier.text)))
						return false;
					elementname = identifier.text;
					break;
				}
				case AST::rgInContainer:
				{
					if (unlikely(child.children.size() != 1))
						return ice(child) << "invalid expression";
					container = &child.firstChild();
					break;
				}
				case AST::rgInViewName:
				{
					for (auto& vnnode: child.children)
					{
						switch (vnnode.rule)
						{
							case AST::rgIdentifier:
							{
								requestedViewName = vnnode.text;
								auto& children = vnnode.children;
								if (not children.empty())
								{
									if (children.size() == 1 and children[0].rule == AST::rgCall)
									{
										additionalParams = &(children[0]);
									}
									else
										return unexpectedNode(vnnode, "[expr/in/view-name/params]");
								}
								break;
							}
							default:
								return unexpectedNode(vnnode, "[expr/in/view-name]");
						}
					}
					break;
				}
				case AST::rgInPredicate:
				{
					if (unlikely(child.children.size() != 1))
						return ice(child) << "invalid predicate expr";
					auto& expr = child.firstChild();
					if (unlikely(expr.rule != AST::rgExpr))
						return ice(expr) << "invalid node type for predicate";
					predicate = &expr;
					break;
				}
				default:
					return unexpectedNode(child, "[expr/in]");
			}
		}

		if (unlikely(!container))
			return ice(node) << "invalid view container expr";

		if (!context.reuse.inset.node)
			context.prepareReuseForIn();

		ShortString128 viewname;
		viewname << "^view^" << requestedViewName;

		context.reuse.inset.container->children.clear();
		context.reuse.inset.container->children.push_back(container);
		context.reuse.inset.viewname->text = viewname;

		if (elementname.empty())
			elementname << "%vr" << nextvar();
		context.reuse.inset.elementname->text = elementname;

		if (!predicate)
			predicate = AST::Node::Ptr::WeakPointer(context.reuse.inset.premadeAlwaysTrue);
		context.reuse.inset.predicate->children.clear();
		context.reuse.inset.predicate->children.push_back(predicate);

		if (additionalParams)
		{
			auto& vec = context.reuse.inset.call->children;
			for (auto& paramnode: additionalParams->children)
				vec.push_back(&paramnode);
		}
		emitDebugpos(node);
		bool success = visitASTExpr(*context.reuse.inset.node, localvar);

		if (additionalParams) // remove the additional parameters
		{
			auto& vec = context.reuse.inset.call->children;
			AST::Node::Ptr firstNode = &(vec.front());
			vec.clear();
			vec.push_back(firstNode);
		}
		// avoid crap in the debugger
		context.reuse.inset.elementname->text.clear();
		context.reuse.inset.viewname->text.clear();
		return success;
	}


	bool Scope::visitASTExprIn(AST::Node& node, LVID& localvar)
	{
		// Name of the target ref for each element in the container (ignored here)
		ShortString128 elementname;
		return visitASTExprIn(node, localvar, elementname);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
