#pragma once
#include <yuni/yuni.h>
#include "context.h"
#include "details/ir/fwd.h"
#include <yuni/core/flags.h>



namespace ny
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
		Attributes(AST::Node& node): node(node) {}

		//! Attributes presence
		Yuni::Flags<Flag> flags;
		//! builtinalias: or | and | ...
		AST::Node* builtinAlias = nullptr;
		//! The original attribute AST node
		AST::Node& node;
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
		bool visitAST(AST::Node&);
		bool visitASTStmt(AST::Node&);
		bool visitASTFunc(AST::Node&);
		bool visitASTClass(AST::Node&, LVID* localvar = nullptr);
		bool visitASTType(AST::Node&, LVID& localvar);
		bool visitASTTypedef(AST::Node&);
		bool visitASTFor(AST::Node&);
		bool visitASTAttributes(AST::Node&);
		bool visitASTVar(AST::Node&);
		bool visitASTVarInClass();
		bool visitASTExpr(AST::Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIntrinsic(AST::Node&, LVID& localvar);
		bool visitASTExprReturn(AST::Node&);
		bool visitASTExprContinuation(AST::Node&, LVID& localvar, bool allowScope = false);
		bool visitASTExprIdentifier(AST::Node&, LVID& localvar);
		bool visitASTExprIdOperator(AST::Node& node, LVID& localvar);
		bool visitASTExprCall(AST::Node*, LVID& localvar, AST::Node* parent = nullptr); // func call
		bool visitASTExprCallParameters(AST::Node&, ShortcircuitUpdate* shortcircuit = nullptr); // parameters of a func call
		bool visitASTExprSubDot(AST::Node&, LVID& localvar);
		bool visitASTExprScope(AST::Node&);
		bool visitASTExprNumber(AST::Node&, LVID& localvar);
		bool visitASTExprString(AST::Node&, LVID& localvar);
		bool visitASTExprStringLiteral(AST::Node&, LVID& localvar);
		bool visitASTExprChar(AST::Node&, LVID& localvar);
		bool visitASTExprNew(AST::Node&, LVID& localvar);
		bool visitASTExprTypeDecl(AST::Node&, LVID& localvar);
		bool visitASTExprTypeof(AST::Node&, LVID& localvar);
		bool visitASTExprIfStmt(AST::Node&);
		bool visitASTExprIfExpr(AST::Node&, LVID& localvar);
		bool visitASTExprWhile(AST::Node&);
		bool visitASTExprDoWhile(AST::Node&);
		bool visitASTExprSwitch(AST::Node&);
		bool visitASTExprIn(AST::Node&, LVID& localvar);
		bool visitASTExprIn(AST::Node&, LVID& localvar, Yuni::ShortString128& elementname);
		bool visitASTExprClosure(AST::Node&, uint32_t& localvar);
		bool visitASTExprObject(AST::Node&, uint32_t& localvar);
		bool visitASTExprTemplate(AST::Node&, LVID& localvar);
		bool visitASTDeclGenericTypeParameters(AST::Node&);
		bool visitASTUnitTest(AST::Node&);
		bool visitASTArray(AST::Node&, uint32_t& localvar);
		//@}

		//! \name Typeinfo / variables
		//@{
		//! Create a new local builtin float
		LVID createLocalBuiltinFloat(AST::Node&, nytype_t, double value);
		//! Create a new local builtin integer
		LVID createLocalBuiltinInt(AST::Node&, nytype_t, yuint64 value);
		//! Create a new local builtin 'void'
		LVID createLocalBuiltinVoid(AST::Node&);
		//! Create a new local variable 'any'
		LVID createLocalBuiltinAny(AST::Node&);

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
		void emitDebugpos(AST::Node& node);
		void emitDebugpos(AST::Node* node);
		void addDebugCurrentFilename();
		void addDebugCurrentFilename(const AnyString& filename);
		void addDebugCurrentPosition(uint line, uint offset);

		bool generateIfStmt(AST::Node& expr, AST::Node& thenc, AST::Node* elsec = nullptr, uint32_t* customjmpthenOffset = nullptr);
		bool generateIfExpr(uint32_t& ifret, AST::Node& expr, AST::Node& thenc, AST::Node& elsec);


		//! \name Utilities
		//@{
		//! Acquire a string
		AnyString acquireString(const AnyString& string);
		//! The attached sequence
		Sequence& sequence();

		//! Get if debuginfo should be used
		bool hasDebuginfo() const;

		//! Get the name from a 'symbol-name' node (empty if error)
		AnyString getSymbolNameFromASTNode(AST::Node& node);

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
		Context& context;
		//! Next local variable
		LVID nextVarID = 0u;
		//! Kind
		Kind kind = Kind::undefined;
		//! Opcode offset of the last identify opcode
		// (to allow to promote identify:get to identify:set)
		uint32_t lastIdentifyOpcOffset = 0u;
		//! For template parameters
		std::unique_ptr<std::vector<std::pair<uint32_t, AnyString>>> lastPushedTmplParams;
		//! Expression attributes
		std::unique_ptr<Attributes> attributes;
		//! Parent scope (if any)
		Scope* parentScope = nullptr;
		//! BroadcastNextVarID
		bool broadcastNextVarID = true;
		//! Nakama
		friend class Context;

	private:
		void emitTmplParametersIfAny();
		void doEmitTmplParameters();
		void emitExprAttributes(uint32_t& localvar);

	}; // class Scope






} // namespace Producer
} // namespace IR
} // namespace ny

#include "scope.hxx"
