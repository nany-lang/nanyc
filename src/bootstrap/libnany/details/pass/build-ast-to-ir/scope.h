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
		explicit Scope(Context& context);
		//! Default constructor from another scope
		Scope(Scope& scope);
		//! Destructor
		~Scope();
		//@}


		//! \name AST::Node visitor
		//@{
		//! Visit an unknown node
		bool visitAST(AST::Node&);

		bool visitASTStmt(const AST::Node&);
		bool visitASTFunc(const AST::Node&);
		bool visitASTClass(const AST::Node&, LVID* localvar = nullptr);
		bool visitASTType(const AST::Node&, LVID& localvar);
		bool visitASTTypedef(const AST::Node&);
		bool visitASTFor(const AST::Node&);

		bool visitASTVar(const AST::Node&);
		bool visitASTVarValueInitialization(LVID&, const AST::Node&, const AST::Node&, const AnyString&);

		bool visitASTExpr(const AST::Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIntrinsic(const AST::Node&, LVID& localvar);
		bool visitASTExprReturn(const AST::Node&);
		bool visitASTExprContinuation(const AST::Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIdentifier(const AST::Node&, LVID& localvar);
		bool visitASTExprRegister(const AST::Node&, LVID& localvar);
		bool visitASTExprCall(const AST::Node*, LVID& localvar, const AST::Node* parent = nullptr); // func call
		bool visitASTExprCallParameters(const AST::Node&, uint32_t shortcircuitlabel = 0); // parameters of a func call
		bool visitASTExprSubDot(const AST::Node&, LVID& localvar);
		bool visitASTExprScope(const AST::Node&);
		bool visitASTExprNumber(const AST::Node&, LVID& localvar);
		bool visitASTExprString(const AST::Node&, LVID& localvar);
		bool visitASTExprStringLiteral(const AST::Node&, LVID& localvar);
		bool visitASTExprNew(const AST::Node&, LVID& localvar);
		bool visitASTExprTypeDecl(const AST::Node&, LVID& localvar);
		bool visitASTExprTypeof(const AST::Node&, LVID& localvar);
		bool visitASTExprIfStmt(const AST::Node&);
		bool visitASTExprIfExpr(const AST::Node&, LVID& localvar);
		bool visitASTExprWhile(const AST::Node&);
		bool visitASTExprDoWhile(const AST::Node&);
		bool visitASTExprSwitch(const AST::Node&);
		bool visitASTExprIn(const AST::Node&, LVID& localvar);
		bool visitASTExprIn(const AST::Node&, LVID& localvar, AnyString& elementname);
		bool visitASTExprClosure(const AST::Node&, uint32_t& localvar);

		bool visitASTExprTemplate(const AST::Node&, LVID& localvar);
		bool visitASTExprTemplateParameter(const AST::Node& node);
		bool visitASTDeclGenericTypeParameters(const AST::Node&);
		bool visitASTDeclSingleGenericTypeParameter(const AST::Node&);
		//@}


		//! \name Typeinfo / variables
		//@{
		//! Create a new local builtin float
		LVID createLocalBuiltinFloat64(const AST::Node&, nytype_t type, double value);
		//! Create a new local builtin integer
		LVID createLocalBuiltinInt64(const AST::Node&, nytype_t type, yuint64 value);
		//! Create a new local builtin 'void'
		LVID createLocalBuiltinVoid(const AST::Node&);
		//! Create a new local variable 'any'
		LVID createLocalBuiltinAny(const AST::Node&);

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
		void emitDebugpos(const AST::Node& node);
		void emitDebugpos(const AST::Node* node);
		void addDebugCurrentFilename();
		void addDebugCurrentFilename(const AnyString& filename);
		void addDebugCurrentPosition(uint line, uint offset);
		void fetchLineAndOffsetFromNode(const AST::Node& node, yuint32& line, yuint32& offset) const;

		bool generateIfStmt(const AST::Node& expr, const AST::Node& thenc, const AST::Node* elsec = nullptr, uint32_t* customjmpthenOffset = nullptr);
		bool generateIfExpr(uint32_t& ifret, const AST::Node& expr, const AST::Node& thenc, const AST::Node& elsec);


		//! \name Utilities
		//@{
		//! Acquire a string
		AnyString acquireString(const AnyString& string);
		//! The attached sequence
		Sequence& sequence();

		//! Get if debuginfo should be used
		bool hasDebuginfo() const;

		//! Get the name from a 'symbol-name' node (empty if error)
		AnyString getSymbolNameFromASTNode(const AST::Node& node);
		//@}


		//! \name Error management
		//@{
		//! Get the reporting
		Logs::Report& report();
		//! Generate an error "ICE: unexecpected node..."
		// \return Always false
		bool ICEUnexpectedNode(const AST::Node& node, const AnyString& location) const;
		//! Generate an error "ICE: <msg>"
		// \return Always false
		Logs::Report ICE(const AST::Node& node) const;

		Logs::Report error(const AST::Node& node);
		Logs::Report warning(const AST::Node& node);

		void setErrorFrom(Logs::Report& report, const AST::Node& node) const;
		void setErrorFrom(const AST::Node& node) const;
		//@}


		//! \name Operators
		//@{
		//! assignment
		Scope& operator = (const Scope&) = delete;
		//@}


	public:
		//! Context
		Context& context;


	private:
		bool generateInitFuncForClassVar(const AnyString& varname, LVID, const AST::Node& varAssign);
		bool generateTypeofForClassVar(LVID&, const AST::Node& varAssign);
		template<bool BuiltinT, class DefT> bool generateNumberCode(uint32_t& localvar, const DefT& numdef, const AST::Node&);
		void emitTmplParametersIfAny();

		void prepareClosureNodeExpr(AST::Node::Ptr& out);


	private:
		//! Next local variable
		LVID pNextVarID = 0u;
		//! Kind
		Kind kind = Kind::undefined;

		//! Parent scope (if any)
		Scope* parentScope = nullptr;
		//! For template parameters
		std::vector<std::pair<uint32_t, AnyString>> lastPushedTmplParams;

		//! BroadcastNextVarID
		bool broadcastNextVarID = true;
		//! Nakama
		friend class Context;

	}; // class Scope






} // namespace Producer
} // namespace IR
} // namespace Nany

#include "scope.hxx"
