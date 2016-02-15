#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include "libnany-config.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	inline bool Scope::visitASTDeclSingleGenericTypeParameter(const Node& node)
	{
		assert(node.rule == rgFuncParam);
		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgIdentifier:
				{
					const AnyString& name = child.text;
					if (not checkForValidIdentifierName(report(), child, name))
						return false;
					sequence().emitBlueprintGenericTypeParam(nextvar(), name);
					break;
				}
				case rgVarType:
				{
					error(child)
						<< "type definition for generic type parameters is currently not supported";
					return false;
				}
				case rgVarAssign:
				{
					error(child)
						<< "default value for generic type parameters not implemented";
					return false;
				}
				default:
					return ICEUnexpectedNode(child, "[gen-type-param]");
			}
		}
		return true;
	}


	bool Scope::visitASTDeclGenericTypeParameters(const Node& node)
	{
		assert(node.rule == rgClassTemplateParams);
		if (unlikely(node.children.empty()))
			return true;

		auto& out = sequence();
		if (debugmode)
			out.emitComment("generic type parameters");

		bool success = true;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgFuncParam:
				{
					success &= visitASTDeclSingleGenericTypeParameter(child);
					break;
				}
				default:
					return ICEUnexpectedNode(child, "[gen-type-params]");
			}
		}
		return success;
	}





	inline bool Scope::visitASTExprTemplateParameter(const Node& node)
	{
		assert(node.rule == rgCallTemplateParameter or node.rule == rgCallTemplateNamedParameter);

		const Node* type = nullptr;
		AnyString name;

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;

			switch (child.rule)
			{
				case rgType:
				{
					type = &child;
					break;
				}
				case rgIdentifier:
				{
					name = child.text;
					break;
				}
				default:
					return ICEUnexpectedNode(child, "[expr-template-param/call]");
			}
		}

		if (unlikely(type == nullptr))
			return (ICE(node) << "invalid node type");

		uint32_t localvar = 0;
		if (not visitASTType(*type, localvar))
			return false;

		if (unlikely(localvar == (uint32_t) -1))
			return (error(*type) << "'any' is not accepted as parameter");

		if (name.empty())
			sequence().emitTPush(localvar);
		else
			sequence().emitTPush(localvar, name);
		return true;
	}



	bool Scope::visitASTExprTemplate(const Node& node, LVID& localvar)
	{
		assert(node.rule == rgExprTemplate);

		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgCallTemplateParameters:
				{
					bool success = true;
					for (auto& ptr: child.children)
					{
						auto& param = *ptr;
						switch (param.rule)
						{
							case rgCallTemplateParameter:
							case rgCallTemplateNamedParameter:
							{
								success &= visitASTExprTemplateParameter(param);
								break;
							}
							default:
								return ICEUnexpectedNode(param, "[expr-template-param]");
						}
					}

					if (unlikely(not success))
						return false;
					break;
				}
				case rgExprSubDot:
				{
					bool e = visitASTExprSubDot(child, localvar);
					if (unlikely(not e))
						return false;
					break;
				}
				case rgCall:
				{
					bool e = visitASTExprCall(&child, localvar, &node);
					if (unlikely(not e))
						return false;
					break;
				}
				default:
					return ICEUnexpectedNode(child, "[expr-template]");
			}
		}
		return true;
	}



} // namespace Producer
} // namespace IR
} // namespace Nany
