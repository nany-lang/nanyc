#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{

	bool Scope::generateIf(Node& expr, Node& thenc, Node* elsec, uint32_t jmpthen, uint32_t jmpelse)
	{
		// new scope for the condition
		IR::Producer::Scope scopeIf{*this};
		// output program
		auto& out = scopeIf.program();

		// evalation of the condition
		uint32_t condlvid = out.emitStackalloc(scopeIf.nextvar(), nyt_bool);

		bool hasElseClause = (elsec != nullptr);
		bool success = true;

		// expression
		{
			uint32_t exprEval = 0;
			success &= scopeIf.visitASTExpr(expr, exprEval, false);
			out.emitAssign(condlvid, exprEval, false);
		}

		uint32_t labelElse = (hasElseClause ? scopeIf.nextvar() : 0);
		uint32_t labelEnd  = scopeIf.nextvar();

		// jump to the 'else' clause if false (or end)
		out.emitJz(condlvid, 0, (hasElseClause ? labelElse : labelEnd));

		// if-then...
		{
			{
				IR::Producer::Scope scopeThen{scopeIf};
				scopeThen.emitDebugpos(thenc);

				for (auto& stmt: thenc.children)
					success &= scopeThen.visitASTStmt(*stmt);
			}

			// jump to the end to not execute the 'else' clause
			if (jmpthen == 0)
				out.emitJmp(labelEnd);
			else
				out.emitJmp(jmpthen);
		}

		// ...else
		if (hasElseClause)
		{
			{
				IR::Producer::Scope scopeElse{scopeIf};
				out.emitLabel(labelElse);
				scopeElse.emitDebugpos(*elsec);

				for (auto& stmt: elsec->children)
					success &= scopeElse.visitASTStmt(*stmt);
			}
			if (jmpelse != 0)
				out.emitJmp(jmpelse);
		}

		out.emitLabel(labelEnd);
		return success;
	}


	bool Scope::visitASTExprIf(Node& node, LVID& localvar)
	{
		assert(node.rule == rgIf);
		assert(not node.children.empty());

		localvar = 0;

		// the condition to evaluate
		auto& condition  = *(node.children[0]);
		auto& thenc = *(node.children[1]);

		switch (node.children.size())
		{
			case 2:
			{
				// if-then
				auto* elsec = Node::Ptr::WeakPointer(node.children[2]);
				return generateIf(condition, thenc, elsec);
			}
			case 3:
			{
				// if-then-else
				return generateIf(condition, thenc);
			}
			default:
			{
				ICE(node) << "if: invalid number of children";
				return false;
			}
		}
		return false;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
