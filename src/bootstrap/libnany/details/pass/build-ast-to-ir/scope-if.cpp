#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;





namespace Nany
{
namespace IR
{
namespace Producer
{


	bool Scope::generateIfStmt(const AST::Node& expr, const AST::Node& thenc, const AST::Node* elseptr, uint32_t* customjmpthenOffset)
	{
		// output sequence
		auto& out = sequence();

		if (debugmode)
			out.emitComment("*** if-stmt");

		bool hasElseClause = (elseptr != nullptr);
		bool success = true;

		// expression
		// evalation of the condition
		uint32_t condlvid = out.emitStackalloc(nextvar(), nyt_bool);
		{
			if (debugmode)
				out.emitComment("if-cond");
			OpcodeScopeLocker opscopeCond{out};
			uint32_t exprEval = 0;
			emitDebugpos(expr);
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
				out.emitComment("if-stmt-then");

			// stmt
			{
				OpcodeScopeLocker opscopeThen{out};
				emitDebugpos(thenc);

				if (unlikely(thenc.children.size() != 1))
					return (ICE(thenc) << "invalid if-then branch");
				auto& thenNode = *(thenc.children[0]);

				success &= visitASTStmt(thenNode);
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
			assert(elseptr != nullptr);
			if (debugmode)
				out.emitComment("if-stmt-else");

			// stmt
			{
				out.emitLabel(labelElse);
				OpcodeScopeLocker opscopeElse{out};
				auto& elsec = *elseptr;
				emitDebugpos(elsec);

				if (unlikely(elsec.children.size() != 1))
					return (ICE(elsec) << "invalid if-then branch");
				auto& elseNode = *(elsec.children[0]);

				success &= visitASTStmt(elseNode);
			}
		}

		out.emitLabel(labelEnd);
		return success;
	}


	bool Scope::generateIfExpr(uint32_t& ifret, const AST::Node& expr, const AST::Node& thenc, const AST::Node& elsec)
	{
		// output sequence
		auto& out = sequence();

		if (debugmode)
			out.emitComment("*** if-expr");

		// result of the expression
		ifret = out.emitStackalloc(nextvar(), nyt_any);
		out.emitQualifierRef(ifret, true);

		bool hasElseClause = true;
		bool success = true;

		// expression
		// evalation of the condition
		uint32_t condlvid = out.emitStackalloc(nextvar(), nyt_bool);
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
				out.emitComment("if-expr-then");

			{
				OpcodeScopeLocker opscopeThen{out};
				emitDebugpos(thenc);

				if (unlikely(thenc.children.size() != 1))
					return (ICE(thenc) << "invalid if-then branch");
				auto& thenNode = *(thenc.children[0]);

				uint32_t thenlvid;
				success &= visitASTExpr(thenNode, thenlvid);
				out.emitAssign(ifret, thenlvid, false);
			}

			if (hasElseClause)
				out.emitJmp(labelEnd);
		}

		// ...else
		if (hasElseClause)
		{
			if (debugmode)
				out.emitComment("if-expr-else");

			{
				out.emitLabel(labelElse);
				OpcodeScopeLocker opscopeElse{out};
				emitDebugpos(elsec);

				if (unlikely(elsec.children.size() != 1))
					return (ICE(elsec) << "invalid if-then branch");
				auto& elseNode = *(elsec.children[0]);

				uint32_t elselvid;
				success &= visitASTExpr(elseNode, elselvid);
				out.emitAssign(ifret, elselvid, false);
			}
		}

		out.emitLabel(labelEnd);
		return success;
	}








	bool Scope::visitASTExprIfStmt(const AST::Node& node)
	{
		assert(node.rule == AST::rgIf);
		assert(node.children.size() >= 2);

		// the condition to evaluate

		switch (node.children.size())
		{
			case 2:
			{
				// if-then
				auto& condition  = *(node.children[0]);
				auto& thenc = *(node.children[1]);
				return generateIfStmt(condition, thenc);
			}
			case 3:
			{
				// if-then-else
				auto& condition  = *(node.children[0]);
				auto& thenc = *(node.children[1]);
				auto* elsec = AST::Node::Ptr::WeakPointer(node.children[2]);
				return generateIfStmt(condition, thenc, elsec);
			}
			default:
			{
				ICE(node) << "if: invalid ast node";
				return false;
			}
		}
		return false;
	}


	bool Scope::visitASTExprIfExpr(const AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgIf);
		assert(node.children.size() >= 2);

		localvar = 0u;

		// the condition to evaluate

		switch (node.children.size())
		{
			case 3:
			{
				// if-then-else
				auto& condition = *(node.children[0]);
				auto& thenc     = *(node.children[1]);
				auto& elsec     = *(node.children[2]);
				return generateIfExpr(localvar, condition, thenc, elsec);
			}
			case 2:
			{
				error(node) << "'else' value expected";
				return false;
			}

			default:
			{
				ICE(node) << "if: invalid ast node";
				return false;
			}
		}
		return false;
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
