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

	inline bool Scope::visitASTExprIdentifier(Node& node, LVID& localvar)
	{
		// value fetching
		emitDebugpos(node);
		uint32_t rid = program().emitStackalloc(nextvar(), nyt_any);
		program().emitIdentify(rid, node.text, localvar);
		localvar = rid;

		return visitASTExprContinuation(node, localvar);
	}


	inline bool Scope::visitASTExprSubDot(Node& node, LVID& localvar)
	{
		return visitASTExprContinuation(node, localvar);
	}


	bool Scope::visitASTExprContinuation(Node& node, LVID& localvar, bool allowScope)
	{
		bool success = true;
		for (auto& childptr: node.children)
		{
			auto& child = *childptr;
			switch (child.rule)
			{
				case rgIdentifier: success &= visitASTExprIdentifier(child, localvar); break;
				case rgExprGroup:  success &= visitASTExpr(child, localvar); break;
				case rgCall:       success &= visitASTExprCall(&child, localvar, &node); break;
				case rgExprSubDot: success &= visitASTExprSubDot(child, localvar); break;
				case rgNumber:     success &= visitASTExprNumber(child, localvar); break;
				case rgNew:        success &= visitASTExprNew(child, localvar); break;

				// typing - same as std expr
				case rgTypeSubDot: success &= visitASTExprSubDot(child, localvar); break;

				// strings
				case rgStringLiteral: success &= visitASTExprStringLiteral(child, localvar); break;

				// special stuff
				case rgTypeof:     success &= visitASTExprTypeof(child, localvar); break;
				case rgIntrinsic:  success &= visitASTExprIntrinsic(child, localvar); break;

				case rgIf:         success &= visitASTExprIf(child, localvar); break;

				// scope may appear in expr (when expr are actually statements)
				case rgScope:  if (allowScope) { success &= visitASTExprScope(child); break; }; // if not > unexpected
				default:
					success = ICEUnexpectedNode(child, "[ir/expr/continuation]");
			}
		}
		return success;
	}


	bool Scope::visitASTExpr(Node& node, LVID& localvar, bool allowScope)
	{
		assert(node.rule == rgExpr or node.rule == rgExprGroup or node.rule == rgTypeDecl);
		assert(not node.children.empty());

		// reset the value of the localvar, result of the expr
		localvar = 0;

		// always creating a new scope for a expr
		IR::Producer::Scope scope{*this};
		scope.emitDebugpos(node);
		return scope.visitASTExprContinuation(node, localvar, allowScope);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
