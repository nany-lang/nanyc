#pragma once
#include "scope.h"
#include "libnanyc-config.h"
#include "details/ir/emit.h"

namespace ny::ir::Producer {

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
	if (not onScopeFailExitLabels.empty())
		updateOnScopeFailExitLabels();
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
		(not config::removeRedundantDbgOffset or offset != context.m_previousDbgOffset
		 or line != context.m_previousDbgLine)) {
		ir::emit::dbginfo::position(context.ircode, line, offset);
		context.m_previousDbgOffset = offset;
		context.m_previousDbgLine = line;
	}
}

inline void Scope::resetLocalCounters(uint32_t localvarStart) {
	// 0: null
	// 1: reserved for namespace lookup
	nextVarID = localvarStart;
	// force debug info
	context.m_previousDbgOffset = 0;
	context.m_previousDbgLine = 0;
}

inline uint32_t Scope::nextvar() {
	return ++nextVarID;
}

inline uint32_t Scope::reserveLocalVariable() {
	return ++nextVarID;
}

inline uint32_t Scope::createLocalBuiltinVoid(AST::Node& node) {
	emitDebugpos(node);
	return ir::emit::alloc(context.ircode, nextvar(), CType::t_void);
}

inline uint32_t Scope::createLocalBuiltinAny(AST::Node& node) {
	emitDebugpos(node);
	return ir::emit::alloc(context.ircode, nextvar());
}

inline uint32_t Scope::createLocalBuiltinFloat(AST::Node& node, CType type, double value) {
	emitDebugpos(node);
	return ir::emit::allocf64(context.ircode, nextvar(), type, value);
}

inline uint32_t Scope::createLocalBuiltinInt(AST::Node& node, CType type, yuint64 value) {
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

inline bool Scope::visitASTAttributes(AST::Node& node) {
	return node.children.empty() or fetchAttributes(node);
}

} // ny::ir::Producer
