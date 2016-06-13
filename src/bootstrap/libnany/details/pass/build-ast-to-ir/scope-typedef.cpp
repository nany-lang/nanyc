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

	bool Scope::visitASTTypedef(const AST::Node& node)
	{
		assert(node.rule == AST::rgTypedef);
		AnyString typedefname;
		const AST::Node* typeexpr = nullptr;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case AST::rgIdentifier:
				{
					typedefname = child.text;
					bool ok = checkForValidIdentifierName(child, typedefname, false, true);
					if (unlikely(not ok))
						return false;
					break;
				}
				case AST::rgType:
				{
					typeexpr = &child;
					break;
				}

				default:
					return unexpectedNode(child, "[ir/typedef]");
			}
		}

		if (unlikely(nullptr == typeexpr))
			return (ice(node) << "invalid typedef definition");

		IR::Producer::Scope scope{*this};
		uint32_t localvar;
		if (not scope.visitASTType(*typeexpr, localvar))
			return false;

		scope.sequence().emitBlueprintTypealias(typedefname, localvar);
		return true;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
