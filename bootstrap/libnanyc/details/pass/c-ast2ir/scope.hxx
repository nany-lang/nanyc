#pragma once
#include "scope.h"
#include "libnanyc-config.h"
#include "details/ir/emit.h"


namespace ny {
namespace ir {
namespace Producer {


inline Scope::Scope(Context& context)
	: context(context)
	, broadcastNextVarID(false) {
}


inline Scope::Scope(Scope& scope)
	: context(scope.context)
	, nextVarID(scope.nextVarID)
	, kind(scope.kind)
	, parentScope(&scope) {
}


inline Scope::~Scope() {
	if (broadcastNextVarID) {
		assert(parentScope != nullptr); // broadcast new values to the parent
		parentScope->nextVarID = nextVarID;
	}
	if (unlikely(!!attributes))
		checkForUnknownAttributes();
}


inline Sequence& Scope::ircode() {
	return context.ircode;
}


inline AnyString Scope::acquireString(const AnyString& string) {
	return context.ircode.stringrefs.refstr(string);
}


inline bool Scope::visitAST(AST::Node& node) {
	return visitASTStmt(node);
}


inline void Scope::addDebugCurrentPosition(uint line, uint offset) {
	if (context.debuginfo and
		(not config::removeRedundantDbgOffset or offset != context.pPreviousDbgOffset
		 or line != context.pPreviousDbgLine)) {
		ir::emit::dbginfo::position(context.ircode, line, offset);
		context.pPreviousDbgOffset = offset;
		context.pPreviousDbgLine = line;
	}
}


inline void Scope::resetLocalCounters(uint32_t localvarStart) {
	// 0: null
	// 1: reserved for namespace lookup
	nextVarID = localvarStart;
	// force debug info
	context.pPreviousDbgOffset = 0;
	context.pPreviousDbgLine = 0;
}


inline uint32_t Scope::nextvar() {
	return ++nextVarID;
}


inline uint32_t Scope::reserveLocalVariable() {
	return ++nextVarID;
}


inline uint32_t Scope::createLocalBuiltinVoid(AST::Node& node) {
	emitDebugpos(node);
	return ir::emit::alloc(context.ircode, nextvar(), nyt_void);
}


inline uint32_t Scope::createLocalBuiltinAny(AST::Node& node) {
	emitDebugpos(node);
	return ir::emit::alloc(context.ircode, nextvar());
}


inline uint32_t Scope::createLocalBuiltinFloat(AST::Node& node, nytype_t type, double value) {
	emitDebugpos(node);
	return ir::emit::allocf64(context.ircode, nextvar(), type, value);
}


inline uint32_t Scope::createLocalBuiltinInt(AST::Node& node, nytype_t type, yuint64 value) {
	emitDebugpos(node);
	return ir::emit::allocu64(context.ircode, nextvar(), type, value);
}


inline bool Scope::hasDebuginfo() const {
	return context.debuginfo;
}


inline bool Scope::isWithinClass() const {
	Scope* scope = parentScope;
	while (scope != nullptr) {
		switch (scope->kind) {
			case Kind::kclass:
				return true;
			case Kind::kfunc:
				return false;
			case Kind::undefined:
				break;
		}
		// go to previous
		scope = scope->parentScope;
	}
	return false;
}


inline void Scope::emitDebugpos(AST::Node* node) {
	if (node)
		emitDebugpos(*node);
}


inline bool Scope::visitASTExprSubDot(AST::Node& node, uint32_t& localvar) {
	emitTmplParametersIfAny();
	ir::emit::type::ensureResolved(context.ircode, localvar);
	return visitASTExprContinuation(node, localvar);
}


inline void Scope::emitTmplParametersIfAny() {
	if (lastPushedTmplParams.get())
		doEmitTmplParameters();
}


inline void Scope::moveAttributes(Scope& scope) {
	attributes = nullptr;
	std::swap(attributes, scope.attributes);
}


} // namespace Producer
} // namespace ir
} // namespace ny
