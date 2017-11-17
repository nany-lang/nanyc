#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"

using namespace Yuni;

namespace ny {
namespace ir {
namespace Producer {

bool Scope::generateIfStmt(AST::Node& expr, AST::Node& thenc, AST::Node* elseptr,
		uint32_t* customjmpthenOffset) {
	auto& irout = ircode();
	ir::emit::trace(irout, "*** if-stmt");
	emitDebugpos(expr);
	bool hasElseClause = (elseptr != nullptr);
	bool success = true;
	// expression
	// evalation of the condition
	uint32_t condlvid = ir::emit::alloc(irout, nextvar(), CType::t_bool);
	{
		ir::emit::trace(irout, "if-cond-stmt");
		ir::emit::ScopeLocker opscopeCond{irout};
		uint32_t exprEval = 0;
		emitDebugpos(expr);
		success &= visitASTExpr(expr, exprEval, false);
		ir::emit::assign(irout, condlvid, exprEval, false);
	}
	uint32_t labelElse = 0; // (hasElseClause ? nextvar() : 0);
	uint32_t labelEnd  = 0; // nextvar();
	// jump to the 'else' clause if false (or end) (label updated later)
	uint32_t opOffJz = irout.opcodeCount();
	ir::emit::jz(irout, condlvid, 0, (hasElseClause ? labelElse : labelEnd));
	// opcode offset for jumping to label 'end' after 'then' stmt
	uint32_t opOffIntermediateEnd = 0u;
	// if-then...
	{
		ir::emit::trace(irout, "then-stmt");
		// stmt
		{
			ir::emit::ScopeLocker opscopeThen{irout};
			emitDebugpos(thenc);
			if (unlikely(thenc.children.size() != 1))
				return (ice(thenc) << "invalid if-then branch");
			auto& thenNode = thenc.children[0];
			success &= visitASTStmt(thenNode);
		}
		// jump to the end to not execute the 'else' clause
		if (customjmpthenOffset == nullptr) {
			if (hasElseClause) {
				opOffIntermediateEnd = irout.opcodeCount();
				ir::emit::jmp(irout, labelEnd);
			}
		}
		else {
			*customjmpthenOffset = irout.opcodeCount();
			ir::emit::jmp(irout, 0); // will be filled later
		}
	}
	// ...else
	if (hasElseClause) {
		assert(elseptr != nullptr);
		ir::emit::trace(irout, "else-stmt");
		// stmt
		{
			labelElse = ir::emit::label(irout, nextvar());
			ir::emit::ScopeLocker opscopeElse{irout};
			auto& elsec = *elseptr;
			emitDebugpos(elsec);
			if (unlikely(elsec.children.size() != 1))
				return (ice(elsec) << "invalid if-then branch");
			auto& elseNode = elsec.children[0];
			success &= visitASTStmt(elseNode);
		}
	}
	labelEnd = ir::emit::label(irout, nextvar());
	// post-update label ids
	irout.at<ir::isa::Op::jz>(opOffJz).label = (hasElseClause ? labelElse : labelEnd);
	if (opOffIntermediateEnd != 0)
		irout.at<ir::isa::Op::jmp>(opOffIntermediateEnd).label = labelEnd;
	return success;
}

bool Scope::generateIfExpr(uint32_t& ifret, AST::Node& expr, AST::Node& thenc, AST::Node& elsec) {
	// output sequence
	auto& irout = ircode();
	ir::emit::trace(irout, "*** if-expr");
	emitDebugpos(expr);
	// result of the expression
	ifret = ir::emit::alloc(irout, nextvar());
	ir::emit::type::qualifierRef(irout, ifret, true);
	bool hasElseClause = true;
	bool success = true;
	// expression
	// evalation of the condition
	uint32_t condlvid = ir::emit::alloc(irout, nextvar(), CType::t_bool);
	{
		ir::emit::trace(irout, "if-cond-expr");
		ir::emit::ScopeLocker opscopeCond{irout};
		uint32_t exprEval = 0;
		success &= visitASTExpr(expr, exprEval, false);
		ir::emit::assign(irout, condlvid, exprEval, false);
	}
	uint32_t labelElse = 0; // (hasElseClause ? nextvar() : 0);
	uint32_t labelEnd  = 0; // nextvar();
	// jump to the 'else' clause if false (or end)
	uint32_t opOffJz = irout.opcodeCount();
	ir::emit::jz(irout, condlvid, 0, (hasElseClause ? labelElse : labelEnd));
	// opcode offset for jumping to label 'end' after 'then' stmt
	uint32_t opOffIntermediateEnd = 0u;
	// if-then...
	{
		ir::emit::trace(irout, "then-expr");
		{
			ir::emit::ScopeLocker opscopeThen{irout};
			emitDebugpos(thenc);
			if (unlikely(thenc.children.size() != 1))
				return (ice(thenc) << "invalid if-then branch");
			auto& thenNode = thenc.children[0];
			uint32_t thenlvid;
			success &= visitASTExpr(thenNode, thenlvid);
			ir::emit::assign(irout, ifret, thenlvid, false);
		}
		if (hasElseClause) {
			opOffIntermediateEnd = irout.opcodeCount();
			ir::emit::jmp(irout, labelEnd);
		}
	}
	// ...else
	if (hasElseClause) {
		ir::emit::trace(irout, "else-expr");
		{
			labelElse = ir::emit::label(irout, nextvar());
			ir::emit::ScopeLocker opscopeElse{irout};
			emitDebugpos(elsec);
			if (unlikely(elsec.children.size() != 1))
				return (ice(elsec) << "invalid if-then branch");
			auto& elseNode = elsec.children[0];
			uint32_t elselvid;
			success &= visitASTExpr(elseNode, elselvid);
			ir::emit::assign(irout, ifret, elselvid, false);
		}
	}
	labelEnd = ir::emit::label(irout, nextvar());
	// post-update label ids
	irout.at<ir::isa::Op::jz>(opOffJz).label = (hasElseClause ? labelElse : labelEnd);
	if (opOffIntermediateEnd)
		irout.at<ir::isa::Op::jmp>(opOffIntermediateEnd).label = labelEnd;
	return success;
}

bool Scope::visitASTExprIfStmt(AST::Node& node) {
	assert(node.rule == AST::rgIf);
	AST::Node* condition = nullptr;
	AST::Node* ifthen = nullptr;
	AST::Node* ifelse = nullptr;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgExpr:
				condition = &child;
				break;
			case AST::rgIfThen:
				ifthen = &child;
				break;
			case AST::rgIfElse:
				ifelse = &child;
				break;
			default:
				return unexpectedNode(child, "[if-stmt]");
		}
	}
	if (unlikely(!condition or !ifthen))
		return (error(node) << "invalid if-then node");
	return generateIfStmt(*condition, *ifthen, ifelse);
}

bool Scope::visitASTExprIfExpr(AST::Node& node, uint32_t& localvar) {
	assert(node.rule == AST::rgIf);
	localvar = 0u;
	AST::Node* condition = nullptr;
	AST::Node* ifthen = nullptr;
	AST::Node* ifelse = nullptr;
	for (auto& child : node.children) {
		switch (child.rule) {
			case AST::rgExpr:
				condition = &child;
				break;
			case AST::rgIfThen:
				ifthen = &child;
				break;
			case AST::rgIfElse:
				ifelse = &child;
				break;
			default:
				return unexpectedNode(child, "[if-stmt]");
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
