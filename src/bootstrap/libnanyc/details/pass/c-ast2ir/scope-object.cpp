#include <yuni/yuni.h>
#include "scope.h"
#include "details/ast/ast.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace ny
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprObject(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgObject);
		// Converting the object into an anonymous class
		//
		// expr-group
		// |   expr-value
		// |       new
		// |           type-decl
		// |               class
		// |                   class-body [reuse]
		// |                       expr
		// |                       |   expr-value
		// |                       |       var (+3)
		// |                       |           var-by-value
		// |                       |           identifier: x
		// |                       |           var-property
		// |                       |               expr
		// |                       |                  ...
		// |                       expr
		// |                       |   expr-value
		// |                       |       function (+3)
		// |                       |           function-kind
		// |                       |           |   function-kind-function (+2)
		// |                       |           |       tk-func
		// |                       |           |       symbol-name
		// |                       |           |           identifier: foo
		// |                       |           func-params (+2)
		// |                       |           |   tk-parenthese-open, (
		// |                       |           |   tk-parenthese-close, )
		// |                       |           func-body
		// |                       |               return-inline
		// |                       |                  ...

		if (!context.reuse.object.node)
			context.prepareReuseForAnonymObjects();
		auto& classbody = *context.reuse.object.classbody;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgObjectEntry:
				{
					AST::Node* identifier = nullptr;
					AST::Node* expr = nullptr;
					for (auto& entry: child.children)
					{
						switch (entry.rule)
						{
							case AST::rgIdentifier: identifier = &entry; break;
							case AST::rgExpr: expr = &entry; break;
							default: return unexpectedNode(entry, "[ir/object/entry]");
						}
					}
					assert(identifier != nullptr); // invalid ast
					assert(expr != nullptr);

					auto& grandChild = expr->firstChild().firstChild();
					bool isFunc = grandChild.rule == AST::rgFunction;

					if (not isFunc)
					{
						AST::Node::Ptr var = new AST::Node{AST::rgVar};
						var->append(AST::rgVarByValue);
						var->children.push_back(identifier);
						(var->append(AST::rgVarProperty)).children.push_back(expr);
						classbody.children.push_back(var);
					}
					else
					{
						bool found = not grandChild.each(AST::rgFunctionKind, [&](AST::Node& subnode) -> bool
						{
							subnode.each(AST::rgFunctionKindFunction, [&](AST::Node& kind) -> bool {
								kind.children.clear();
								kind.append(AST::rgSymbolName).children.push_back(identifier);
								found = true;
								return false;
							});
							return false;
						});
						if (found)
							classbody.children.push_back(&grandChild);
						else
							error(child) << "failed to update function name";
					}
					break;
				}
				default:
					return unexpectedNode(child, "[ir/object]");
			}
		}

		bool success = visitASTExpr(*context.reuse.object.node, localvar);
		classbody.children.clear();
		return success;
	}




} // namespace Producer
} // namespace IR
} // namespace ny
