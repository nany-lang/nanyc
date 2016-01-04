#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::visitASTExprIf(Node& node, LVID& localvar)
	{
		assert(node.rule == rgIf);
		assert(not node.children.empty());

		localvar = 0;
		uint32_t childcount = static_cast<uint32_t>(node.children.size());
		if (unlikely(childcount != 2 and childcount != 3))
		{
			ICE(node) << "if: invalid number of children";
			return false;
		}

		auto& out = program();
		bool success = true;
		// new scope for the condition
		IR::Producer::Scope scopeIf{*this};
		// evalation of the condition
		uint32_t condlvid = out.emitStackalloc(scopeIf.nextvar(), nyt_bool);

		bool hasElseClause = (childcount == 3);

		// expression
		{
			auto& condition = *(node.children[0]);
			uint32_t exprEval = 0;
			success &= scopeIf.visitASTExpr(condition, exprEval, false);
			out.emitAssign(condlvid, exprEval, false);
		}

		uint32_t labelElse = (hasElseClause ? scopeIf.nextvar() : 0);
		uint32_t labelEnd  = scopeIf.nextvar();

		// jump to the 'else' clause if false (or end)
		out.emitJz(condlvid, 0, (hasElseClause ? labelElse : labelEnd));

		// if-then...
		{
			IR::Producer::Scope scopeThen{scopeIf};
			auto& thenClause = *(node.children[1]);
			scopeThen.emitDebugpos(thenClause);

			for (auto& stmt: thenClause.children)
				success &= scopeThen.visitASTStmt(*stmt);
		}
		if (hasElseClause)
		{
			// jump to the end to not execute the 'else' clause
			out.emitJmp(labelEnd);
		}

		// ...else
		if (hasElseClause)
		{
			IR::Producer::Scope scopeElse{scopeIf};
			out.emitLabel(labelElse);
			auto& elseClause = *(node.children[2]);
			scopeElse.emitDebugpos(elseClause);

			for (auto& stmt: elseClause.children)
				success &= scopeElse.visitASTStmt(*stmt);
		}

		out.emitLabel(labelEnd);
		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
