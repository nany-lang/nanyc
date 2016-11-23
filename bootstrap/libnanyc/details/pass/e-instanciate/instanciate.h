#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string/string.h>
#include "details/context/build.h"
#include "details/atom/classdef-table-view.h"
#include "details/ir/isa/data.h"
#include "details/ir/sequence.h"
#include "details/errors/errors.h"
#include "stack-frame.h"
#include "func-overload-match.h"
#include <vector>



namespace ny
{
	class IntrinsicTable;
	class Build;
}


namespace ny
{
namespace Pass
{
namespace Instanciate
{

	struct InstanciateData;
	class OverloadedFuncCallResolver;


	struct IndexedParameter final
	{
		IndexedParameter(uint32_t lvid, uint line, uint offset)
			: lvid(lvid), line(line), offset(offset)
		{}
		uint32_t lvid;
		uint32_t line;
		uint32_t offset;
	};

	struct NamedParameter final
	{
		NamedParameter(const AnyString& name, uint32_t lvid, uint line, uint offset)
			: name(name), lvid(lvid), line(line), offset(offset)
		{}
		AnyString name;
		uint32_t lvid;
		uint32_t line;
		uint32_t offset;
	};






	class SequenceBuilder final
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor (importSignature must be called after)
		SequenceBuilder(Logs::Report, ClassdefTableView&, Build&,
			ir::Sequence* out, ir::Sequence&, SequenceBuilder* parent = nullptr);

		//! Destructor
		~SequenceBuilder();
		//@}


		//! \name Visitors for all supported opcodes
		//@{
		void visit(const ir::ISA::Operand<ir::ISA::Op::scope>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::end>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::ret>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::intrinsic>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::call>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::stackalloc>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::allocate>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::push>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::tpush>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::classdefsizeof>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::follow>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::identify>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::identifyset>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::ensureresolved>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::debugfile>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::debugpos>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::pragma>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::blueprint>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::stacksize>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::namealias>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::storeConstant>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::store>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::storeText>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::typeisobject>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::ref>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::unref>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::assign>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::self>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::qualifiers>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::nop>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::label>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::jmp>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::jz>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::jnz>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::comment>&);
		void visit(const ir::ISA::Operand<ir::ISA::Op::commontype>&);
		//! visitor - fallback
		template<ir::ISA::Op O> void visit(const ir::ISA::Operand<O>&);

		//! Walk through all opcodes generated from the AST
		bool readAndInstanciate(uint32_t offset);
		//@}


		//! \name Type checking
		//@{
		/*!
		** \brief Try to resolve a type alias
		**
		** \param atom A type alias atom
		** \param[out] cdefout The resolved classdef. null if failed
		*/
		Atom& resolveTypeAlias(Atom& atom, const Classdef*& cdefout);

		//! Get if a value from a register can be acquired (ref counting)
		bool canBeAcquired(uint32_t lvid) const;
		//! Get if a type definition can be acquired (ref. counting)
		bool canBeAcquired(const CLID& clid) const;
		//! Get if a type definition can be acquired (ref. counting)
		bool canBeAcquired(const Classdef& cdef) const;

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
		//! Emit 'ref' opcode if 'type' can be acquired (e.g. is not a builtin type)
		template<class T> void tryToAcquireObject(uint32_t lvid, const T& type);
		//! Emit 'ref' opcode if 'lvid' can be acquired (e.g. is not a builtin type)
		void tryToAcquireObject(uint32_t lvid);
		//! Emit 'ref' opcode whether 'lvid' can be acquired or not (no check)
		void acquireObject(uint32_t lvid);
		//! Emit 'unref' opcode if 'lvid' can be acquired
		void tryUnrefObject(uint32_t lvid);

		//! Create 'count' new local variables and return the first lvid
		uint32_t createLocalVariables(uint32_t count = 1);

		/*!
		** \brief Emit 'unref' opcodes to release all variables available from a given scope depth
		**
		** \param scope Scope's Depth
		** \param forget Flag to avoid (or not) to release those variables again
		*/
		void releaseScopedVariables(int scope, bool forget = false);
		//@}


		//! \name Misc Code generation
		//@{
		//! Get if opcodes can be emitted (not within a type definition for example)
		bool canGenerateCode() const;

		//! Generate the function to initialize the class variables to their default values
		void generateMemberVarDefaultInitialization();
		//! Generate the destructor of the current class
		void generateMemberVarDefaultDispose();
		//! Generate the clone function of the current class
		void generateMemberVarDefaultClone();


		bool instanciateAssignment(const ir::ISA::Operand<ir::ISA::Op::call>& operands);

		bool instanciateAssignment(AtomStackFrame& frame, uint32_t lhs, uint32_t rhs, bool canDisposeLHS = true,
			bool checktype = true, bool forceDeepcopy = false);

		//! perform type resolution and fetch data (local variable, func...)
		bool identify(const ir::ISA::Operand<ir::ISA::Op::identify>& operands, const AnyString& name, bool firstChance = true);
		bool identifyCapturedVar(const ir::ISA::Operand<ir::ISA::Op::identify>& operands, const AnyString& name);

		//! Try to capture variables from a list of potentiel candidates created by the mapping
		void captureVariables(Atom& atom);

		Atom* instanciateAtomClass(Atom& atom);
		bool instanciateAtomClassDestructor(Atom& atom, uint32_t self);
		bool instanciateAtomClassClone(Atom& atom, uint32_t self, uint32_t rhs);

		bool instanciateAtomFunc(uint32_t& instanceid, Atom& funcAtom, uint32_t retlvid, uint32_t p1 = 0, uint32_t p2 = 0);

		bool pushCapturedVarsAsParameters(const Atom& atomclass);

		//! Declare a named variable (and checks for multiple declarations)
		void declareNamedVariable(const AnyString& name, uint32_t lvid, bool autorelease = true);

		void instanciateInstrinsicCall();
		bool instanciateUserDefinedIntrinsic(const ir::ISA::Operand<ir::ISA::Op::intrinsic>& operands);
		Yuni::Tribool::Value instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid, bool canComplain = true);
		Yuni::Tribool::Value instanciateBuiltinIntrinsicSpecific(const AnyString& name, uint32_t lvid, bool canProduceError);
		//@}


		//! \name Errors
		//@{
		bool checkForIntrinsicParamCount(const AnyString& name, uint32_t count);
		bool complainInvalidType(const char* origin, const Classdef& from, const Classdef& to);
		void complainReturnTypeMultiple(const Classdef& expected, const Classdef& usertype, uint32_t line = 0, uint32_t offset = 0);
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
		bool doInstanciateAtomFunc(Logs::Message::Ptr& subreport, InstanciateData& info, uint32_t retlvid);
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

		//! Build context
		Build& build;
		//! intrinsics
		const IntrinsicTable& intrinsics;

		//! All pushed parameters
		struct PushedParameters final
		{
			struct Catalog final
			{
				bool empty() const {return indexed.empty() and named.empty();}
				void clear() { indexed.clear(); named.clear(); }
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
		//! Flag to generate variable member destruction after opcode 'stack size'
		bool generateClassVarsAutoInit = false;
		//! Flag to generate variable member initialization after opcode 'stack size'
		bool generateClassVarsAutoRelease = false;
		//! Flag to generate variable member cloning after opcode 'stack size'
		bool generateClassVarsAutoClone = false;
		//! Flag to skip code instanciation as soon as the opcode blueprint size is encountered
		bool shouldSkipCurrentAtom = false;

		struct {
			uint32_t label = 0;
			bool compareTo = false;
		} shortcircuit;

		// exit status
		mutable bool success = true;

		//! Previous sequence builder
		SequenceBuilder* parent = nullptr;
		//! Error reporting
		Logs::Handler localErrorHandler;
		Logs::MetadataHandler localMetadataHandler;
		//! Current report
		mutable Logs::Report report;
		//! Flag to determine weather sub atoms can be instanciated in the same time
		uint32_t layerDepthLimit = (uint32_t) -1;
		bool signatureOnly = false;
		//! Force mapping from an atomid to another one
		// This mapping is required instnaciating the code from a
		// forked atom and to avoid invalid references
		uint32_t mappingBlueprintAtomID[2] = {0, 0};
		//! cursor
		ir::Instruction** cursor = nullptr;

	}; // class SequenceBuilder




} // namespace Instanciate
} // namespace Pass
} // namespace ny

#include "instanciate.hxx"
