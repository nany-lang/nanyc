#pragma once
#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/context.h"
#include "details/ir/fwd.h"





namespace Nany
{
namespace IR
{
namespace Producer
{

	/*!
	** \brief Scope for IR generation (requires a context or another scope)
	*/
	class Scope final
	{
	public:
		enum class Kind
		{
			undefined,
			kfunc,
			kclass,
		};

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor from a producer context
		Scope(Context& context);
		//! Default constructor from another scope
		Scope(Scope& scope);
		//! Destructor
		~Scope();
		//@}


		//! \name Node visitor
		//@{
		//! Visit an unknown node
		bool visitAST(Node&);

		bool visitASTStmt(Node&);
		bool visitASTFunc(Node&);
		bool visitASTClass(Node&);
		bool visitASTType(Node&, LVID& localvar);

		bool visitASTVar(Node&);
		bool visitASTVarValueInitialization(LVID&, Node&, const Node&, const AnyString&);

		bool visitASTExpr(Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIntrinsic(Node&, LVID& localvar);
		bool visitASTExprReturn(Node&);
		bool visitASTExprContinuation(Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIdentifier(Node&, LVID& localvar);
		bool visitASTExprCall(const Node*, LVID& localvar, const Node* parent = nullptr); // func call
		bool visitASTExprCallParameters(const Node&, uint32_t shortcircuitlabel = 0); // parameters of a func call
		bool visitASTExprSubDot(Node&, LVID& localvar);
		bool visitASTExprScope(Node&);
		bool visitASTExprNumber(Node&, LVID& localvar);
		bool visitASTExprNew(const Node&, LVID& localvar);
		bool visitASTExprTypeDecl(Node&, LVID& localvar);
		bool visitASTExprTypeof(Node&, LVID& localvar);
		bool visitASTExprStringLiteral(Node&, LVID& localvar);
		bool visitASTExprIf(Node&, LVID& localvar);
		bool visitASTExprWhile(Node&);
		bool visitASTExprDoWhile(Node&);
		bool visitASTExprSwitch(Node&);
		//@}


		//! \name Typeinfo / variables
		//@{
		//! Create a new local builtin float
		LVID createLocalBuiltinFloat64(const Node&, nytype_t type, double value);
		//! Create a new local builtin integer
		LVID createLocalBuiltinInt64(const Node&, nytype_t type, yuint64 value);
		//! Create a new local builtin 'void'
		LVID createLocalBuiltinVoid(const Node&);
		//! Create a new local variable 'any'
		LVID createLocalBuiltinAny(const Node&);

		//! Reserve a new variable id
		LVID reserveLocalVariable();
		uint32_t nextvar();

		//! Reset the internal counter for creating local classdef and local variables
		void resetLocalCounters(LVID localvarStart = 0u);

		//! Get if the scope is inside a class
		bool isWithinClass() const;
		//@}


		//! \name Debug infos
		//@{
		//! Add a comment within the IR code
		void comment(const AnyString& text);
		//! Add an empty comment line within the IR code
		void comment();

		//! Emit opcode related to the current position (in the current source)
		void emitDebugpos(const Node& node);
		void emitDebugpos(const Node* node);
		void addDebugCurrentFilename();
		void addDebugCurrentFilename(const AnyString& filename);
		void addDebugCurrentPosition(uint line, uint offset);
		void fetchLineAndOffsetFromNode(const Node& node, yuint32& line, yuint32& offset) const;
		bool generateIf(Node& expr, Node& thenc, Node* elsec = nullptr, uint32_t* customjmpthenOffset = nullptr);


		//! \name Utilities
		//@{
		//! Acquire a string
		AnyString acquireString(const AnyString& string);
		//! The attached program
		Program& program();

		//! Get if debuginfo should be used
		bool hasDebuginfo() const;

		//! Get the name from a 'symbol-name' node (empty if error)
		AnyString getSymbolNameFromASTNode(Node& node);
		//@}


		//! \name Error management
		//@{
		//! Get the reporting
		Logs::Report& report();
		//! Generate an error "ICE: unexecpected node..."
		// \return Always false
		bool ICEUnexpectedNode(Node& node, const AnyString& location) const;
		//! Generate an error "ICE: <msg>"
		// \return Always false
		Logs::Report ICE(Node& node) const;

		Logs::Report error(const Node& node);
		Logs::Report warning(const Node& node);

		void setErrorFrom(Logs::Report& report, const Node& node) const;
		void setErrorFrom(const Node& node) const;
		//@}


		//! \name Operators
		//@{
		//! assignment
		Scope& operator = (const Scope&) = delete;
		//@}


	private:
		bool generateInitFuncForClassVar(const AnyString& varname, LVID, const Node& varAssign);
		bool generateTypeofForClassVar(LVID&, const Node& varAssign);

	private:
		//! Context
		Context& pContext;
		//! Next local variable
		LVID pNextVarID = 0u;
		//! Parent scope (if any)
		Scope* parentScope = nullptr;
		//! Kind
		Kind kind = Kind::undefined;
		//! BroadcastNextVarID
		bool broadcastNextVarID = true;
		//! Nakama
		friend class Context;

	}; // class Scope






} // namespace Producer
} // namespace IR
} // namespace Nany

#include "scope.hxx"
