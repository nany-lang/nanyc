#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/utils/check-for-valid-identifier-name.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	namespace {


	bool declSingleGenericTypeParameter(Scope& scope, AST::Node& node)
	{
		assert(node.rule == AST::rgFuncParam);
		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgIdentifier:
				{
					const AnyString& name = child.text;
					if (not checkForValidIdentifierName(child, name))
						return false;
					scope.emitDebugpos(child);
					scope.sequence().emitBlueprintGenericTypeParam(scope.nextvar(), name);
					break;
				}
				case AST::rgVarType:
				{
					error(child)
						<< "type definition for generic type parameters is currently not supported";
					return false;
				}
				case AST::rgVarAssign:
				{
					error(child)
						<< "default value for generic type parameters not implemented";
					return false;
				}
				default:
					return unexpectedNode(child, "[gen-type-param]");
			}
		}
		return true;
	}


	bool exprTemplateParameter(Scope& scope, AST::Node& node)
	{
		assert(node.rule == AST::rgCallTemplateParameter or node.rule == AST::rgCallTemplateNamedParameter);

		AST::Node* type = nullptr;
		AnyString name;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgType:
				{
					type = &child;
					break;
				}
				case AST::rgIdentifier:
				{
					name = child.text;
					break;
				}
				default:
					return unexpectedNode(child, "[expr-template-param/call]");
			}
		}

		if (unlikely(type == nullptr))
			return (ice(node) << "invalid node type");

		uint32_t localvar = 0;
		if (not scope.visitASTType(*type, localvar))
			return false;

		if (unlikely(localvar == (uint32_t) -1))
			return (error(*type) << "'any' is not accepted as parameter");

		scope.lastPushedTmplParams->emplace_back(localvar, name);
		return true;
	}


	} // namespace




	bool Scope::visitASTDeclGenericTypeParameters(AST::Node& node)
	{
		assert(node.rule == AST::rgClassTemplateParams);
		if (unlikely(node.children.empty()))
			return true;

		if (debugmode)
			sequence().emitComment("generic type parameters");

		bool success = true;
		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgFuncParam:
				{
					success &= declSingleGenericTypeParameter(*this, child);
					break;
				}
				default:
					return unexpectedNode(child, "[gen-type-params]");
			}
		}
		return success;
	}


	bool Scope::visitASTExprTemplate(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgExprTemplate or node.rule == AST::rgExprTypeTemplate);

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgCallTemplateParameters:
				{
					lastPushedTmplParams = std::make_unique<std::vector<std::pair<uint32_t,AnyString>>>();
					lastPushedTmplParams->reserve(child.children.size());
					bool success = true;
					for (auto& param: child.children)
					{
						switch (param.rule)
						{
							case AST::rgCallTemplateParameter:
							case AST::rgCallTemplateNamedParameter:
							{
								success &= exprTemplateParameter(*this, param);
								break;
							}
							default:
								return unexpectedNode(param, "[expr-template-param]");
						}
					}
					if (unlikely(not success))
						return false;
					break;
				}
				case AST::rgExprSubDot:
				case AST::rgTypeSubDot:
				{
					bool e = visitASTExprSubDot(child, localvar);
					if (unlikely(not e))
						return false;
					break;
				}
				case AST::rgCall:
				{
					bool e = visitASTExprCall(&child, localvar, &node);
					if (unlikely(not e))
						return false;
					break;
				}
				default:
					return unexpectedNode(child, "[expr-template]");
			}
		}
		return true;
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
