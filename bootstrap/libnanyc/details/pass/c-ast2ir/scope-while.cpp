#include "scope.h"
#include "details/grammar/nany.h"
#include "details/ir/emit.h"

using namespace Yuni;

namespace ny {
namespace ir {
namespace Producer {

bool Scope::visitASTExprWhile(AST::Node& node) {
	assert(node.rule == AST::rgWhile);
	assert(not node.children.empty());
	uint32_t childcount = node.children.size();
	if (unlikely(childcount != 2)) {
		ice(node) << "while: invalid number of children";
		return false;
	}
	auto& irout = ircode();
	bool success = true;
	ir::emit::trace(irout, "while-do");
	// new scope for the 'while' statement
	ir::emit::ScopeLocker opscopeWhile{irout};
	emitDebugpos(node);
	// the label at the very begining, to loop back
	uint32_t labelWhile = ir::emit::label(irout, nextvar());
	ir::emit::trace(irout, "while-condition");
	// a temporary variable for the result of the condition evaluation
	uint32_t condlvid = nextvar();
	ir::emit::alloc(irout, condlvid, CType::t_bool);
	// generating the code for the condition itself
	{
		ir::emit::ScopeLocker opscopeCond{irout};
		auto& condition = node.children[0];
		emitDebugpos(condition);
		uint32_t exprEval = 0;
		success &= visitASTExpr(condition, exprEval, false);
		ir::emit::assign(irout, condlvid, exprEval, false);
	}
	// jump at the end of the 'while' statement if false
	uint32_t jumpOffset = irout.opcodeCount();
	ir::emit::jz(irout, condlvid, 0, /*labelEnd*/ 0); // set later, to preserve strict ordering
	// 'while' body
	{
		ir::emit::trace(irout, "while-body");
		ir::emit::ScopeLocker opscopeBody{irout};
		success &= visitASTStmt(node.children[1]);
	}
	emitDebugpos(node);
	// loop back to the condition evaluation
	ir::emit::jmp(irout, labelWhile);
	// end of while, if the condition evaluation failed
	// (and update the label id in the original jump)
	uint32_t labelEnd = ir::emit::label(irout, nextvar());
	irout.at<ir::isa::Op::jz>(jumpOffset).label = labelEnd;
	return success;
}


bool Scope::visitASTExprDoWhile(AST::Node& node) {
	assert(node.rule == AST::rgDoWhile);
	assert(not node.children.empty());
	uint32_t childcount = node.children.size();
	if (unlikely(childcount != 2)) {
		ice(node) << "do-while: invalid number of children";
		return false;
	}
	auto& irout = ircode();
	bool success = true;
	ir::emit::trace(irout, "do-whilte");
	// new scope for the 'while' statement
	ir::emit::ScopeLocker opscopeWhile{irout};
	// the label at the very begining, to loop back
	uint32_t labelWhile = ir::emit::label(irout, nextvar());
	emitDebugpos(node);
	// 'while' body
	{
		ir::emit::trace(irout, "do-whilte-body");
		ir::emit::ScopeLocker opscopeBody{irout};
		success &= visitASTStmt(node.children[0]);
	}
	ir::emit::trace(irout, "do-whilte-condition");
	// a temporary variable for the result of the condition evaluation
	uint32_t condlvid = nextvar();
	ir::emit::alloc(irout, condlvid, CType::t_bool);
	// generating the code for the condition itself
	{
		ir::emit::ScopeLocker opscopeCond{irout};
		auto& condition = node.children[1];
		uint32_t exprEval = 0;
		emitDebugpos(condition);
		success &= visitASTExpr(condition, exprEval, false);
		ir::emit::assign(irout, condlvid, exprEval, false);
	}
	emitDebugpos(node);
	// jump to the begining if true (non zero)
	ir::emit::jnz(irout, condlvid, 0, labelWhile);
	return success;
}

} // namespace Producer
} // namespace ir
} // namespace ny
