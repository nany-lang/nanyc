#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/scope-locker.h"

using namespace Yuni;





namespace ny
{
namespace ir
{
namespace Producer
{


	bool Scope::generateIfStmt(AST::Node& expr, AST::Node& thenc, AST::Node* elseptr, uint32_t* customjmpthenOffset)
	{
		// output sequence
		auto& out = sequence();
		ir::emit::trace(out, "*** if-stmt");
		emitDebugpos(expr);

		bool hasElseClause = (elseptr != nullptr);
		bool success = true;

		// expression
		// evalation of the condition
		uint32_t condlvid = ir::emit::alloc(out, nextvar(), nyt_bool);
		{
			ir::emit::trace(out, "if-cond-stmt");
			OpcodeScopeLocker opscopeCond{out};
			uint32_t exprEval = 0;
			emitDebugpos(expr);
			success &= visitASTExpr(expr, exprEval, false);
			out.emitAssign(condlvid, exprEval, false);
		}

		uint32_t labelElse = 0; // (hasElseClause ? nextvar() : 0);
		uint32_t labelEnd  = 0; // nextvar();

		// jump to the 'else' clause if false (or end) (label updated later)
		uint32_t opOffJz = out.opcodeCount();
		ir::emit::jz(out, condlvid, 0, (hasElseClause ? labelElse : labelEnd));

		// opcode offset for jumping to label 'end' after 'then' stmt
		uint32_t opOffIntermediateEnd = 0u;

		// if-then...
		{
			ir::emit::trace(out, "then-stmt");
			// stmt
			{
				OpcodeScopeLocker opscopeThen{out};
				emitDebugpos(thenc);

				if (unlikely(thenc.children.size() != 1))
					return (ice(thenc) << "invalid if-then branch");
				auto& thenNode = thenc.children[0];

				success &= visitASTStmt(thenNode);
			}
			// jump to the end to not execute the 'else' clause
			if (customjmpthenOffset == nullptr)
			{
				if (hasElseClause)
				{
					opOffIntermediateEnd = out.opcodeCount();
					ir::emit::jmp(out, labelEnd);
				}
			}
			else
			{
				*customjmpthenOffset = out.opcodeCount();
				ir::emit::jmp(out, 0); // will be filled later
			}
		}
		// ...else
		if (hasElseClause)
		{
			assert(elseptr != nullptr);
			ir::emit::trace(out, "else-stmt");

			// stmt
			{
				labelElse = ir::emit::label(out, nextvar());
				OpcodeScopeLocker opscopeElse{out};
				auto& elsec = *elseptr;
				emitDebugpos(elsec);
				if (unlikely(elsec.children.size() != 1))
					return (ice(elsec) << "invalid if-then branch");
				auto& elseNode = elsec.children[0];
				success &= visitASTStmt(elseNode);
			}
		}

		labelEnd = ir::emit::label(out, nextvar());

		// post-update label ids
		out.at<ir::ISA::Op::jz>(opOffJz).label = (hasElseClause ? labelElse : labelEnd);
		if (opOffIntermediateEnd != 0)
			out.at<ir::ISA::Op::jmp>(opOffIntermediateEnd).label = labelEnd;
		return success;
	}


	bool Scope::generateIfExpr(uint32_t& ifret, AST::Node& expr, AST::Node& thenc, AST::Node& elsec)
	{
		// output sequence
		auto& out = sequence();
		ir::emit::trace(out, "*** if-expr");
		emitDebugpos(expr);

		// result of the expression
		ifret = ir::emit::alloc(out, nextvar());
		out.emitQualifierRef(ifret, true);

		bool hasElseClause = true;
		bool success = true;

		// expression
		// evalation of the condition
		uint32_t condlvid = ir::emit::alloc(out, nextvar(), nyt_bool);
		{
			ir::emit::trace(out, "if-cond-expr");
			OpcodeScopeLocker opscopeCond{out};
			uint32_t exprEval = 0;
			success &= visitASTExpr(expr, exprEval, false);
			out.emitAssign(condlvid, exprEval, false);
		}

		uint32_t labelElse = 0; // (hasElseClause ? nextvar() : 0);
		uint32_t labelEnd  = 0; // nextvar();

		// jump to the 'else' clause if false (or end)
		uint32_t opOffJz = out.opcodeCount();
		ir::emit::jz(out, condlvid, 0, (hasElseClause ? labelElse : labelEnd));

		// opcode offset for jumping to label 'end' after 'then' stmt
		uint32_t opOffIntermediateEnd = 0u;

		// if-then...
		{
			ir::emit::trace(out, "then-expr");
			{
				OpcodeScopeLocker opscopeThen{out};
				emitDebugpos(thenc);

				if (unlikely(thenc.children.size() != 1))
					return (ice(thenc) << "invalid if-then branch");
				auto& thenNode = thenc.children[0];

				uint32_t thenlvid;
				success &= visitASTExpr(thenNode, thenlvid);
				out.emitAssign(ifret, thenlvid, false);
			}
			if (hasElseClause)
			{
				opOffIntermediateEnd = out.opcodeCount();
				ir::emit::jmp(out, labelEnd);
			}
		}
		// ...else
		if (hasElseClause)
		{
			ir::emit::trace(out, "else-expr");
			{
				labelElse = ir::emit::label(out, nextvar());
				OpcodeScopeLocker opscopeElse{out};
				emitDebugpos(elsec);

				if (unlikely(elsec.children.size() != 1))
					return (ice(elsec) << "invalid if-then branch");
				auto& elseNode = elsec.children[0];

				uint32_t elselvid;
				success &= visitASTExpr(elseNode, elselvid);
				out.emitAssign(ifret, elselvid, false);
			}
		}

		labelEnd = ir::emit::label(out, nextvar());

		// post-update label ids
		out.at<ir::ISA::Op::jz>(opOffJz).label = (hasElseClause ? labelElse : labelEnd);
		if (opOffIntermediateEnd)
			out.at<ir::ISA::Op::jmp>(opOffIntermediateEnd).label = labelEnd;

		return success;
	}


	bool Scope::visitASTExprIfStmt(AST::Node& node)
	{
		assert(node.rule == AST::rgIf);
		AST::Node* condition = nullptr;
		AST::Node* ifthen = nullptr;
		AST::Node* ifelse = nullptr;

		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgExpr: condition = &child; break;
				case AST::rgIfThen:  ifthen = &child; break;
				case AST::rgIfElse:  ifelse = &child; break;
				default: return unexpectedNode(child, "[if-stmt]");
			}
		}

		if (unlikely(!condition or !ifthen))
			return (error(node) << "invalid if-then node");

		return generateIfStmt(*condition, *ifthen, ifelse);
	}


	bool Scope::visitASTExprIfExpr(AST::Node& node, LVID& localvar)
	{
		assert(node.rule == AST::rgIf);
		localvar = 0u;
		AST::Node* condition = nullptr;
		AST::Node* ifthen = nullptr;
		AST::Node* ifelse = nullptr;
		for (auto& child: node.children)
		{
			switch (child.rule)
			{
				case AST::rgExpr: condition = &child; break;
				case AST::rgIfThen:  ifthen = &child; break;
				case AST::rgIfElse:  ifelse = &child; break;
				default: return unexpectedNode(child, "[if-stmt]");
			}
		}
		if (unlikely(!condition or !ifthen))
			return (error(node) << "invalid if-then node");
		if (unlikely(!ifelse))
			return (error(node) << "'else' clause is required for a conditional expression");
		return generateIfExpr(localvar, *condition, *ifthen, *ifelse);
	}




} // namespace Producer
} // namespace ir
} // namespace ny
