#pragma once
#include "details/pass/build-ast-to-ir/scope.h"
#include "details/fwd.h"
#include "libnanyc-config.h"




namespace Nany
{
namespace IR
{
namespace Producer
{

	inline Scope::Scope(Context& context)
		: context(context)
		, broadcastNextVarID(false)
	{}


	inline Scope::Scope(Scope& scope)
		: context(scope.context)
		, pNextVarID(scope.pNextVarID)
		, kind(scope.kind)
		, parentScope(&scope)
	{}


	inline Scope::~Scope()
	{
		if (broadcastNextVarID)
		{
			assert(parentScope != nullptr); // broadcast new values to the parent
			parentScope->pNextVarID = pNextVarID;
		}
		if (unlikely(!!pAttributes))
			checkForUnknownAttributes();
	}


	inline Sequence& Scope::sequence()
	{
		return context.sequence;
	}


	inline AnyString Scope::acquireString(const AnyString& string)
	{
		return context.sequence.stringrefs.refstr(string);
	}


	inline bool Scope::visitAST(AST::Node& node)
	{
		return visitASTStmt(node);
	}


	inline void Scope::comment(const AnyString& text)
	{
		context.sequence.emitComment(text);
	}


	inline void Scope::comment()
	{
		context.sequence.emitComment();
	}


	inline void Scope::addDebugCurrentPosition(uint line, uint offset)
	{
		if (context.debuginfo and
			(not Config::removeRedundantDbgOffset or offset != context.pPreviousDbgOffset or line != context.pPreviousDbgLine))
		{
			context.sequence.emitDebugpos(line, offset);
			context.pPreviousDbgOffset = offset;
			context.pPreviousDbgLine = line;
		}
	}


	inline void Scope::addDebugCurrentFilename(const AnyString& filename)
	{
		context.sequence.emitDebugfile(filename);
	}


	inline void Scope::addDebugCurrentFilename()
	{
		addDebugCurrentFilename(context.dbgSourceFilename);
	}


	inline void Scope::resetLocalCounters(LVID localvarStart)
	{
		// 0: null
		// 1: reserved for namespace lookup
		pNextVarID = localvarStart;

		// force debug info
		context.pPreviousDbgOffset = 0;
		context.pPreviousDbgLine = 0;
	}

	inline uint32_t Scope::nextvar()
	{
		return ++pNextVarID;
	}

	inline uint32_t Scope::reserveLocalVariable()
	{
		return ++pNextVarID;
	}

	inline LVID Scope::createLocalBuiltinVoid(const AST::Node& node)
	{
		emitDebugpos(node);
		return context.sequence.emitStackalloc(nextvar(), nyt_void);
	}


	inline LVID Scope::createLocalBuiltinAny(const AST::Node& node)
	{
		emitDebugpos(node);
		return context.sequence.emitStackalloc(nextvar(), nyt_any);
	}


	inline LVID Scope::createLocalBuiltinFloat(const AST::Node& node, nytype_t type, double value)
	{
		emitDebugpos(node);
		return context.sequence.emitStackalloc_f64(nextvar(), type, value);
	}


	inline LVID Scope::createLocalBuiltinInt(const AST::Node& node, nytype_t type, yuint64 value)
	{
		emitDebugpos(node);
		return context.sequence.emitStackalloc_u64(nextvar(), type, value);
	}


	inline bool Scope::hasDebuginfo() const
	{
		return context.debuginfo;
	}


	inline bool Scope::isWithinClass() const
	{
		Scope* scope = parentScope;
		while (scope != nullptr)
		{
			switch (scope->kind)
			{
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


	inline void Scope::emitDebugpos(const AST::Node* node)
	{
		if (node)
			emitDebugpos(*node);
	}


	inline bool Scope::visitASTExprSubDot(const AST::Node& node, LVID& localvar)
	{
		emitTmplParametersIfAny();
		context.sequence.emitEnsureTypeResolved(localvar);
		return visitASTExprContinuation(node, localvar);
	}


	inline void Scope::emitTmplParametersIfAny()
	{
		if (!!lastPushedTmplParams)
			doEmitTmplParameters();
	}


	inline Attributes* Scope::attributes()
	{
		return pAttributes.get();
	}

	inline void Scope::moveAttributes(Scope& scope)
	{
		pAttributes = nullptr;
		std::swap(pAttributes, scope.pAttributes);
	}




} // namespace Producer
} // namespace IR
} // namespace Nany
