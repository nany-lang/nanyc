#pragma once
#include "libnanyc.h"
#include <yuni/core/string/string.h>
#include "details/context/build.h"
#include "details/atom/classdef-table-view.h"
#include "details/ir/isa/data.h"
#include "details/ir/sequence.h"
#include "details/errors/errors.h"
#include "details/compiler/compdb.h"
#include "stack-frame.h"
#include "func-overload-match.h"
#include <vector>
#include <tuple>


namespace ny {
namespace semantic {


struct Settings;
class OverloadedFuncCallResolver;


struct IndexedParameter final {
	IndexedParameter(uint32_t lvid, uint line, uint offset)
		: lvid(lvid), line(line), offset(offset) {
	}
	uint32_t lvid;
	uint32_t line;
	uint32_t offset;
};


struct NamedParameter final {
	NamedParameter(const AnyString& name, uint32_t lvid, uint line, uint offset)
		: name(name), lvid(lvid), line(line), offset(offset) {
	}
	AnyString name;
	uint32_t lvid;
	uint32_t line;
	uint32_t offset;
};


struct OnScopeFailHandlers final {
	struct Handler {
		Handler() = default;
		Handler(uint32_t lvid, uint32_t label, uint32_t scope)
			: lvid(lvid)
			, label(label)
			, scope(scope) {
		}

		void reset(uint32_t newlvid, uint32_t newlabel, uint32_t newscope) {
			lvid  = newlvid;
			label = newlabel;
			used  = false;
			scope = newscope;
		}
		void reset() {
			label = 0;
		}
		bool empty() const { return label == 0; }

		bool operator != (const Handler& other) const {
			return  lvid != other.lvid;
		}

		bool operator == (const Handler& other) const {
			return  lvid == other.lvid;
		}

		uint32_t lvid = 0;
		uint32_t label = 0;
		bool used = false;
		uint32_t scope = 0;
	};

	auto& any() { return m_any; }
	auto& any() const { return m_any; }

	bool empty() const {
		return m_handlers.empty() and m_any.empty();
	}

	Handler* find(const Atom* atom) {
		if (empty())
			return nullptr;
		auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [&](auto& item) {
			return std::get<const Atom*>(item) == atom;
		});
		return it != m_handlers.end() ? &std::get<Handler>(*it) : (m_any.empty() ? nullptr : &m_any);
	}

	const Handler* find(const Atom* atom) const {
		if (empty())
			return nullptr;
		auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [&](auto& item) {
			return std::get<const Atom*>(item) == atom;
		});
		return it != m_handlers.end() ? &std::get<Handler>(*it) : (m_any.empty() ? nullptr : &m_any);
	}

	bool markAsUsed(const Atom& atom) {
		auto* handler = find(&atom);
		if (likely(handler != nullptr)) {
			handler->used = true;
			return true;
		}
		return false;
	}

	bool has(const Atom* atom) const {
		return find(atom) != nullptr;
	}

	auto& add(const Atom* atom, uint32_t lvid, uint32_t label, uint32_t scope) {
		m_handlers.emplace_back(atom, Handler(lvid, label, scope));
		return std::get<Handler>(m_handlers.back());
	}

	auto& back() {
		assert(not m_handlers.empty());
		return std::get<Handler>(m_handlers.back());
	}

	void pop_back() {
		m_handlers.pop_back();
	}

	bool hasBackTypedHandler(const Atom* atom) const {
		return not m_handlers.empty()
			and std::get<const Atom*>(m_handlers.back()) == atom;
	}

	uint32_t scope() const {
		assert(not empty());
		auto& handler = (not m_any.empty()) ?  m_any : std::get<Handler>(m_handlers[0]);
		return handler.scope;
	}

	template<class T> void eachTypedErrorHandler(const T& callback) const {
		for (auto& item: m_handlers)
			callback(std::get<Handler>(item), std::get<const Atom*>(item));
	}

private:
	// all typed errors handlers
	std::vector<std::tuple<const Atom*, Handler>> m_handlers;
	//! Label for any error handler
	Handler m_any;
};


struct Analyzer final {
	Analyzer(Logs::Report, ClassdefTableView&, ny::compiler::Compdb&,
		ir::Sequence* out, ir::Sequence&, Analyzer* parent = nullptr);
	~Analyzer();

	void visit(const ir::isa::Operand<ir::isa::Op::allocate>&);
	void visit(const ir::isa::Operand<ir::isa::Op::assign>&);
	void visit(const ir::isa::Operand<ir::isa::Op::blueprint>&);
	void visit(const ir::isa::Operand<ir::isa::Op::call>&);
	void visit(const ir::isa::Operand<ir::isa::Op::classdefsizeof>&);
	void visit(const ir::isa::Operand<ir::isa::Op::comment>&);
	void visit(const ir::isa::Operand<ir::isa::Op::commontype>&);
	void visit(const ir::isa::Operand<ir::isa::Op::debugfile>&);
	void visit(const ir::isa::Operand<ir::isa::Op::debugpos>&);
	void visit(const ir::isa::Operand<ir::isa::Op::end>&);
	void visit(const ir::isa::Operand<ir::isa::Op::ensureresolved>&);
	void visit(const ir::isa::Operand<ir::isa::Op::follow>&);
	void visit(const ir::isa::Operand<ir::isa::Op::identify>&);
	void visit(const ir::isa::Operand<ir::isa::Op::identifyset>&);
	void visit(const ir::isa::Operand<ir::isa::Op::intrinsic>&);
	void visit(const ir::isa::Operand<ir::isa::Op::jmp>&);
	void visit(const ir::isa::Operand<ir::isa::Op::jnz>&);
	void visit(const ir::isa::Operand<ir::isa::Op::jz>&);
	void visit(const ir::isa::Operand<ir::isa::Op::jzraise>&);
	void visit(const ir::isa::Operand<ir::isa::Op::label>&);
	void visit(const ir::isa::Operand<ir::isa::Op::namealias>&);
	void visit(const ir::isa::Operand<ir::isa::Op::nop>&);
	void visit(const ir::isa::Operand<ir::isa::Op::onscopefail>&);
	void visit(const ir::isa::Operand<ir::isa::Op::pragma>&);
	void visit(const ir::isa::Operand<ir::isa::Op::push>&);
	void visit(const ir::isa::Operand<ir::isa::Op::qualifiers>&);
	void visit(const ir::isa::Operand<ir::isa::Op::raise>&);
	void visit(const ir::isa::Operand<ir::isa::Op::ref>&);
	void visit(const ir::isa::Operand<ir::isa::Op::ret>&);
	void visit(const ir::isa::Operand<ir::isa::Op::scope>&);
	void visit(const ir::isa::Operand<ir::isa::Op::self>&);
	void visit(const ir::isa::Operand<ir::isa::Op::stackalloc>&);
	void visit(const ir::isa::Operand<ir::isa::Op::stacksize>&);
	void visit(const ir::isa::Operand<ir::isa::Op::store>&);
	void visit(const ir::isa::Operand<ir::isa::Op::storeConstant>&);
	void visit(const ir::isa::Operand<ir::isa::Op::storeText>&);
	void visit(const ir::isa::Operand<ir::isa::Op::tpush>&);
	void visit(const ir::isa::Operand<ir::isa::Op::typeisobject>&);
	void visit(const ir::isa::Operand<ir::isa::Op::unref>&);
	template<ir::isa::Op O> void visit(const ir::isa::Operand<O>&);

	bool translateOpcodes(uint32_t offset);


	//! \name Type checking
	//@{
	/*!
	** \brief Try to resolve a type alias
	**
	** \param atom A type alias atom
	** \param[out] cdefout The resolved classdef. null if failed
	*/
	Atom& resolveTypeAlias(Atom& atom, const Classdef*& cdefout);


	/*!
	** \brief Determine if an atom is fully typed and retrieve its return type into the signature
	**
	** Used for recursive functions
	** \return True if fully typed, false if can not be used as a recursive function
	*/
	bool getReturnTypeForRecursiveFunc(const Atom& atom, Classdef&) const;
	//@}


	//! \name Helpers for propgram memory management
	//@{

	//! Create 'count' new local variables and return the first lvid
	uint32_t createLocalVariables(uint32_t count = 1);

	/*!
	** \brief Emit 'unref' opcodes to release all variables available from a given scope depth
	**
	** \param scope Scope's Depth
	** \param forget Flag to avoid (or not) to release those variables again
	*/
	void releaseScopedVariables(int scope, bool forget = false);

	//! Emit 'unred' opcodes for all variables available in all scopes
	void releaseAllScopedVariables();
	//@}


	//! \name Misc Code generation
	//@{
	//! Get if opcodes can be emitted (not within a type definition for example)
	bool canGenerateCode() const;

	bool instanciateAssignment(const ir::isa::Operand<ir::isa::Op::call>& operands);

	bool instanciateAssignment(AtomStackFrame& frame, uint32_t lhs, uint32_t rhs, bool canDisposeLHS = true,
		bool checktype = true, bool forceDeepcopy = false);

	//! perform type resolution and fetch data (local variable, func...)
	bool identify(const ir::isa::Operand<ir::isa::Op::identify>& operands, const AnyString& name,
		bool firstChance = true);
	bool identifyCapturedVar(const ir::isa::Operand<ir::isa::Op::identify>& operands, const AnyString& name);

	//! Try to capture variables from a list of potentiel candidates created by the mapping
	void captureVariables(Atom& atom);

	Atom* instanciateAtomClass(Atom& atom);
	bool instanciateAtomClassDestructor(Atom& atom, uint32_t self);
	bool instanciateAtomClassClone(Atom& atom, uint32_t self, uint32_t rhs);

	bool instanciateAtomFunc(uint32_t& instanceid, Atom& funcAtom, uint32_t retlvid, uint32_t p1 = 0,
		uint32_t p2 = 0);

	bool pushCapturedVarsAsParameters(const Atom& atomclass);

	//! Declare a named variable (and checks for multiple declarations)
	void declareNamedVariable(const AnyString& name, uint32_t lvid, bool autorelease = true);

	void instanciateInstrinsicCall();
	//@}


	//! \name Errors
	//@{
	void complainNoErrorHandler(const Atom& type, const Atom* call, const std::vector<AtomRaisedErrors::Origin>&);
	bool checkForIntrinsicParamCount(const AnyString& name, uint32_t count);
	bool complainInvalidType(const char* origin, const Classdef& from, const Classdef& to);
	void complainReturnTypeMultiple(const Classdef& expected, const Classdef& usertype, uint32_t line = 0,
		uint32_t offset = 0);
	//! Restriction on builtin intrinsics
	bool complainBuiltinIntrinsicDoesNotAccept(const AnyString& name, const AnyString& what);
	//! Complain about named parameters used with an intrinsic
	bool complainIntrinsicWithNamedParameters(const AnyString& name);
	//! Complain about generic type parameters used with an intrinsic
	bool complainIntrinsicWithGenTypeParameters(const AnyString& name);
	bool complainIntrinsicParameterCount(const AnyString& name, uint32_t count);
	bool complainIntrinsicParameter(const AnyString& name, uint32_t pindex, const Classdef& got,
		const AnyString& expected = nullptr);
	//! Emit a new error message with additional information on the given operand
	bool complainOperand(const ir::Instruction& operands, AnyString msg = nullptr);
	//! Emit a new warning/error for an unused variable
	void complainUnusedVariable(const AtomStackFrame&, uint32_t lvid) const;
	//! Emit a new error on a member request on a non-class type
	bool complainInvalidMemberRequestNonClass(const AnyString& name, nytype_t) const;
	//! Unknown builtin type
	bool complainUnknownBuiltinType(const AnyString& name) const;
	//! invalid self reference for var assignment
	bool complainInvalidSelfRefForVariableAssignment(uint32_t lvid) const;
	bool complainMissingOperator(Atom&, const AnyString& name) const;
	bool complainCannotCall(Atom& atom, FuncOverloadMatch& overloadMatch);
	void complainPushedSynthetic(const CLID&, uint32_t paramindex, const AnyString& paramname = nullptr);
	void complainInvalidParametersAfterSignatureMatching(Atom&, FuncOverloadMatch& overloadMatch);
	//@}


public:
	bool doInstanciateAtomFunc(std::shared_ptr<Logs::Message>& subreport, Settings& info, uint32_t retlvid);
	void pushNewFrame(Atom& atom);
	void popFrame();

	// Current stack frame (current func / class...)
	AtomStackFrame* frame = nullptr;
	// Isolate
	ClassdefTableView cdeftable;
	// New opcode sequence
	ir::Sequence* out = nullptr;
	// Current sequence
	ir::Sequence& currentSequence;
	//! Flag to prevent code generation when != 0 (used for typeof for example)
	uint32_t codeGenerationLock = 0;
	//! Flag to prevent error generation when != 0
	uint32_t errorGenerationLock = 0;

	ny::compiler::Compdb& compdb;
	//! intrinsics
	const ny::intrinsic::Catalog& intrinsics;

	//! All pushed parameters
	struct PushedParameters final {
		struct Catalog final {
			bool empty() const {
				return indexed.empty() and named.empty();
			}
			void clear() {
				indexed.clear();
				named.clear();
			}
			std::vector<IndexedParameter> indexed;
			std::vector<NamedParameter> named;
		};

		//! All pushed parameters for func call
		Catalog func;
		//! All pushed parameters for generic type parameters
		Catalog gentypes;

		void clear();
	}
	pushedparams;

	// Helper for resolving func overloads (reused by each opcode 'call')
	FuncOverloadMatch overloadMatch;

	const char* currentFilename = nullptr;
	uint32_t currentLine = 1;
	uint32_t currentOffset = 1;
	//! Results for opcode 'resolve'
	std::vector<std::reference_wrapper<Atom>> multipleResults;
	struct final {
		//! Flag to produce default member vars initialization (called from ctor)
		bool memberVarsInit = false;
		//! Flag to produce default member vars release (called from dtor)
		bool memberVarsRelease = false;
		//! Flag to produce default member vars cloning (called from copy-ctor)
		bool memberVarsClone = false;
	}
	bodystart;

	OnScopeFailHandlers onScopeFail;
	Atom* lastCallWithRaisedError = nullptr;

	struct {
		uint32_t label = 0;
		bool compareTo = false;
	} shortcircuit;

	// exit status
	mutable bool success = true;

	//! Previous sequence builder
	Analyzer* parent = nullptr;
	//! Error reporting
	Logs::Handler localErrorHandler;
	Logs::MetadataHandler localMetadataHandler;
	//! Current report
	mutable Logs::Report report;
	//! Flag to determine weather sub atoms can be instanciated in the same time
	uint32_t layerDepthLimit = (uint32_t) - 1;
	bool signatureOnly = false;
	ir::Instruction** cursor = nullptr;

}; // struct Analyzer


} // namespace semantic
} // namespace ny

#include "semantic-analysis.hxx"
