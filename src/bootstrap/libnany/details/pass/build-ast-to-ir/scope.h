#pragma once
#include <yuni/yuni.h>
#include "details/pass/build-ast-to-ir/context.h"
#include "details/ir/fwd.h"
#include <yuni/core/flags.h>



namespace Nany
{
namespace IR
{
namespace Producer
{

	/*!
	** \brief List of all attributes
	**\
	** The attributes used by the code must be reset to avoid error reporting
	*/
	struct Attributes final
	{
		enum class Flag {
			//! Call builtin instead of func implementation
			builtinAlias,
			//! Do not suggest this function for error reporting
			doNotSuggest,
			//! Use shortcircuit evaluation for the snd parameter
			shortcircuit,
			//! Allow pushing a synthetic object (most likely a type)
			pushSynthetic,
			//! Notify that a function might be called by another thread
			threadproc,

			// when adding a new flag here, do not forget to add err reporting
			// in Scope::checkForUnknownAttributes()
		};

		//! Ctor
		Attributes(const AST::Node& node): node(node) {}

		//! Attributes presence
		Yuni::Flags<Flag> flags;
		//! builtinalias: or | and | ...
		const AST::Node* builtinAlias = nullptr;
		//! The original attribute AST node
		const AST::Node& node;
	};


	/*!
	** \brief Scope for IR generation (requires a context or another scope)
	*/
	class Scope final
	{
	public:
		enum class Kind: uint32_t
		{
			undefined,
			kfunc,
			kclass,
		};
		struct ShortcircuitUpdate final
		{
			uint32_t offsetPragma = 0;
			uint32_t offsetStackalloc = 0;
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

		bool visitASTAttributes(const AST::Node&);

		bool visitASTVar(const AST::Node&);
		bool visitASTVarValueInitialization(LVID&, const AST::Node&, const AST::Node&, const AnyString&);

		bool visitASTExpr(const AST::Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIntrinsic(const AST::Node&, LVID& localvar);
		bool visitASTExprReturn(const AST::Node&);
		bool visitASTExprContinuation(const AST::Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIdentifier(const AST::Node&, LVID& localvar);
		bool visitASTExprIdOperator(const AST::Node& node, LVID& localvar);
		bool visitASTExprRegister(const AST::Node&, LVID& localvar);
		bool visitASTExprCall(const AST::Node*, LVID& localvar, const AST::Node* parent = nullptr); // func call
		bool visitASTExprCallParameters(const AST::Node&, ShortcircuitUpdate* shortcircuit = nullptr); // parameters of a func call
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
		bool visitASTExprIn(const AST::Node&, LVID& localvar, Yuni::ShortString128& elementname);
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

		//! Get the attributes
		Attributes* attributes();
		//! Move attributes
		void moveAttributes(Scope&);
		//@}


		//! \name Error management
		//@{
		void checkForUnknownAttributes() const;
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

		//! Emit generic type parameters push
		void emitTmplParametersIfAny();
		void doEmitTmplParameters();

		void prepareClosureNodeExpr(AST::Node::Ptr& out);
		void emitExprAttributes(uint32_t& localvar);


	private:
		//! Next local variable
		LVID pNextVarID = 0u;
		//! Kind
		Kind kind = Kind::undefined;

		//! For template parameters
		std::unique_ptr<std::vector<std::pair<uint32_t, AnyString>>> lastPushedTmplParams;
		//! Expression attributes
		std::unique_ptr<Attributes> pAttributes;

		//! Parent scope (if any)
		Scope* parentScope = nullptr;
		//! BroadcastNextVarID
		bool broadcastNextVarID = true;

		//! Nakama
		friend class Context;

	}; // class Scope






} // namespace Producer
} // namespace IR
} // namespace Nany

#include "scope.hxx"
