#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::generateIf(Node& expr, Node& thenc, Node* elsec, uint32_t* customjmpthenOffset)
	{
		// output program
		auto& out = program();

		if (debugmode)
			out.emitComment("if");

		OpcodeScopeLocker opscopeIf{out};

		// evalation of the condition
		uint32_t condlvid = out.emitStackalloc(nextvar(), nyt_bool);

		bool hasElseClause = (elsec != nullptr);
		bool success = true;

		// expression
		{
			if (debugmode)
				out.emitComment("if-cond");
			OpcodeScopeLocker opscopeCond{out};
			uint32_t exprEval = 0;
			success &= visitASTExpr(expr, exprEval, false);
			out.emitAssign(condlvid, exprEval, false);
		}

		uint32_t labelElse = (hasElseClause ? nextvar() : 0);
		uint32_t labelEnd  = nextvar();

		// jump to the 'else' clause if false (or end)
		out.emitJz(condlvid, 0, (hasElseClause ? labelElse : labelEnd));

		// if-then...
		{
			if (debugmode)
				out.emitComment("if-then");

			{
				OpcodeScopeLocker opscopeThen{out};
				emitDebugpos(thenc);

				for (auto& stmt: thenc.children)
					success &= visitASTStmt(*stmt);
			}

			// jump to the end to not execute the 'else' clause
			if (customjmpthenOffset == nullptr)
			{
				if (hasElseClause)
					out.emitJmp(labelEnd);
			}
			else
			{
				*customjmpthenOffset = out.opcodeCount();
				out.emitJmp(0); // will be filled later
			}
		}

		// ...else
		if (hasElseClause)
		{
			if (debugmode)
				out.emitComment("if-else");

			{
				OpcodeScopeLocker opscopeElse{out};
				out.emitLabel(labelElse);
				emitDebugpos(*elsec);

				for (auto& stmt: elsec->children)
					success &= visitASTStmt(*stmt);
			}
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
