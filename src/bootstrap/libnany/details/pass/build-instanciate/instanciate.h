#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include "nany/nany.h"
#include "details/reporting/report.h"
#include "details/atom/signature.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/atom/atom.h"
#include "details/atom/func-overload-match.h"
#include "details/intrinsic/intrinsic-table.h"
#include "details/ir/sequence.h"
#include "overloaded-func-call-resolution.h"
#include "stack-frame.h"
#include <vector>



namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	struct InstanciateData;


	struct IndexedParameter final
	{
		IndexedParameter(LVID lvid, uint line, uint offset)
			: lvid(lvid), line(line), offset(offset)
		{}

		uint32_t lvid;
		uint32_t line;
		uint32_t offset;
	};

	struct NamedParameter final
	{
		NamedParameter(const AnyString& name, LVID lvid, uint line, uint offset)
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
		SequenceBuilder(Logs::Report report, ClassdefTableView&, nycontext_t& context,
			IR::Sequence& out, IR::Sequence& sequence);

		//! Prepare the first local registers according the given signature
		void pushParametersFromSignature(LVID atomid, const Signature&);

		//! Destructor
		~SequenceBuilder();
		//@}


		bool readAndInstanciate(uint32_t offset);

		void disableCodeGeneration();
		bool canGenerateCode() const;

		void printClassdefTable(Logs::Report trace, const AtomStackFrame& frame) const;

		void instanciateInstrinsicCall();
		bool instanciateUserDefinedIntrinsic(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands);
		bool instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid, bool canComplain = true);
		bool instanciateIntrinsicFieldset(uint32_t lvid);
		bool instanciateIntrinsicRef(uint32_t lvid);
		bool instanciateIntrinsicUnref(uint32_t lvid);
		bool instanciateIntrinsicAddressof(uint32_t lvid);
		bool instanciateIntrinsicSizeof(uint32_t lvid);
		bool instanciateIntrinsicMemalloc(uint32_t lvid);
		bool instanciateIntrinsicMemFree(uint32_t lvid);

		template<nytype_t R, bool AcceptBool, bool AcceptInt, bool AcceptFloat,
			void (IR::Sequence::* M)(uint32_t, uint32_t, uint32_t)>
		bool instanciateIntrinsicOperator(uint32_t lvid, const char* const name);

		bool instanciateIntrinsicAND(uint32_t);
		bool instanciateIntrinsicOR(uint32_t);
		bool instanciateIntrinsicXOR(uint32_t);
		bool instanciateIntrinsicMOD(uint32_t);
		bool instanciateIntrinsicADD(uint32_t);
		bool instanciateIntrinsicSUB(uint32_t);
		bool instanciateIntrinsicIDIV(uint32_t);
		bool instanciateIntrinsicIMUL(uint32_t);
		bool instanciateIntrinsicDIV(uint32_t);
		bool instanciateIntrinsicMUL(uint32_t);
		bool instanciateIntrinsicFADD(uint32_t);
		bool instanciateIntrinsicFSUB(uint32_t);
		bool instanciateIntrinsicFDIV(uint32_t);
		bool instanciateIntrinsicFMUL(uint32_t);
		bool instanciateIntrinsicEQ(uint32_t);
		bool instanciateIntrinsicNEQ(uint32_t);
		bool instanciateIntrinsicFLT(uint32_t);
		bool instanciateIntrinsicFLTE(uint32_t);
		bool instanciateIntrinsicFGT(uint32_t);
		bool instanciateIntrinsicFGTE(uint32_t);
		bool instanciateIntrinsicLT(uint32_t);
		bool instanciateIntrinsicLTE(uint32_t);
		bool instanciateIntrinsicILT(uint32_t);
		bool instanciateIntrinsicILTE(uint32_t);
		bool instanciateIntrinsicGT(uint32_t);
		bool instanciateIntrinsicGTE(uint32_t);
		bool instanciateIntrinsicIGT(uint32_t);
		bool instanciateIntrinsicIGTE(uint32_t);


	public:
		// exit status
		mutable bool success = true;
		//! cursor
		IR::Instruction** cursor = nullptr;
		//! Flag to determine weather sub atoms can be instanciated in the same time
		uint32_t layerDepthLimit = (uint32_t) -1;


	private:
		//! \name Visitors for all supported opcodes
		//@{
		void visit(const IR::ISA::Operand<IR::ISA::Op::scope>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::end>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::ret>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::intrinsic>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::call>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::stackalloc>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::allocate>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::push>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::classdefsizeof>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::follow>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::identify>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::debugfile>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::debugpos>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::pragma>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::blueprint>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::stacksize>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::namealias>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::storeConstant>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::store>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::storeText>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::typeisobject>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::ref>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::unref>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::assign>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::self>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::qualifiers>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::inherit>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::nop>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::label>&);

		void visit(const IR::ISA::Operand<IR::ISA::Op::jmp>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::jz>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::jnz>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::comment>&);

		//! visitor - fallback
		template<IR::ISA::Op O> void visit(const IR::ISA::Operand<O>&);
		//@}


		//! perform type resolution and fetch data (local variable, func...)
		bool identify(const IR::ISA::Operand<IR::ISA::Op::identify>& operands);

		Atom& resolveTypeAlias(Atom& atom, bool& success);

		bool pragmaBlueprint(const IR::ISA::Operand<IR::ISA::Op::pragma>& operands);
		void pragmaBodyStart();

		bool instanciateAtomClass(Atom& atom);
		bool instanciateAtomClassDestructor(Atom& atom, uint32_t self);
		bool instanciateAtomClassClone(Atom& atom, uint32_t self, uint32_t rhs);

		bool instanciateAtomFunc(uint32_t& instanceid, Atom& funcAtom, uint32_t retlvid, uint32_t p1 = 0, uint32_t p2 = 0);


		bool emitFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands);

		bool instanciateAssignment(const IR::ISA::Operand<IR::ISA::Op::call>& operands);
		bool instanciateAssignment(AtomStackFrame& frame, LVID lhs, LVID rhs, bool canDisposeLHS = true,
			bool checktype = true, bool forceDeepcopy = false);

		//! Declare a named variable (and checks for multiple declarations)
		void declareNamedVariable(const AnyString& name, LVID lvid, bool autorelease = true);


		void releaseScopedVariables(int scope, bool forget = false);

		void adjustSettingsNewFuncdefOperator(const AnyString& name);
		void generateMemberVarDefaultInitialization();
		void generateMemberVarDefaultDispose();
		void generateMemberVarDefaultClone();

		bool generateShortCircuitInstrs(uint32_t retlvid);

		//! \name Help for memory management
		//@{
		//! Get if a value from a register can be acquired (ref counting)
		bool canBeAcquired(LVID lvid) const;
		//! Get if a type definition can be acquired (ref. counting)
		bool canBeAcquired(const CLID& clid) const;
		//! Get if a type definition can be acquired (ref. counting)
		bool canBeAcquired(const Classdef& cdef) const;

		template<class T> void tryToAcquireObject(LVID lvid, const T& type);
		void tryToAcquireObject(LVID lvid);
		void acquireObject(LVID lvid); // without checking

		void tryUnrefObject(uint32_t lvid);

		//! Create 'count' new local variables and return the first lvid
		uint32_t createLocalVariables(uint32_t count = 1);
		//@}


		//! \name Debugging
		//@{
		void printClassdefTable();
		//@}


		//! \name Errors
		//@{
		//! Emit a new error entry
		Logs::Report error() const;
		//! Emit a new warning entry (or error)
		Logs::Report warning() const;
		//! Emit a new ICE log entry
		Logs::Report ICE() const;
		//! Emit a new ICE log entry (ICE on classdef)
		YString ICE(const Classdef&, const AnyString& msg) const;


		bool checkForIntrinsicParamCount(const AnyString& name, uint32_t count);


		bool complainUnknownIdentifier(const Atom* self, const Atom& atom, const AnyString& name);

		bool complainInvalidType(const Classdef& from, const Classdef& to);

		bool complainMultipleOverloads(LVID lvid, const std::vector<std::reference_wrapper<Atom>>& solutions,
			const OverloadedFuncCallResolver& resolver);

		bool complainRedeclared(const AnyString& name, uint32_t previousDeclaration);

		/*!
		** \brief Complain about an unknown intrinsic
		** \param name Intrinsic name
		*/
		bool complainUnknownIntrinsic(const AnyString& name);

		/*!
		** \brief Restriction on builtin intrinsics
		*/
		bool complainBuiltinIntrinsicDoesNotAccept(const AnyString& name, const AnyString& what);

		/*!
		** \brief Complain about named parameters used with an intrinsic
		*/
		bool complainIntrinsicWithNamedParameters(const AnyString& name);

		bool complainIntrinsicParameter(const AnyString& name, uint32_t pindex, const Classdef& got,
			const AnyString& expected = "");

		void complainTypealiasCircularRef(const Atom& original, const Atom& responsible);
		void complainTypedefDeclaredAfter(const Atom& original, const Atom& responsible);
		void complainTypedefUnresolved(const Atom& original);

		/*!
		**
		*/
		bool complainMultipleDefinitions(const Atom&, const AnyString& funcOrOpName);

		//! Emit a new error message with additional information on the given operand
		bool complainOperand(const IR::Instruction& operands, AnyString msg = nullptr);

		//! Emit a new warning/error for an unused variable
		void complainUnusedVariable(const AtomStackFrame&, uint32_t lvid) const;

		//! Emit a new error on a member request on a non-class type
		bool complainInvalidMemberRequestNonClass(const AnyString& name, nytype_t) const;

		//! Unknown builtin type
		bool complainUnknownBuiltinType(const AnyString& name) const;

		//! invalid self reference for var assignment
		bool complainInvalidSelfRefForVariableAssignment(uint32_t lvid) const;

		bool complainMissingOperator(Atom&, const AnyString& name) const;
		//@}


	private:
		bool doInstanciateAtomFunc(Logs::Message::Ptr& subreport, InstanciateData& info, uint32_t retlvid);

	private:
		// Blueprint root element
		std::vector<AtomStackFrame> atomStack;

		// Isolate
		ClassdefTableView cdeftable;
		//! Context
		nycontext_t& context;
		//! intrinsics
		const IntrinsicTable& intrinsics;
		// New opcode sequence
		IR::Sequence& out;
		// Current sequence
		IR::Sequence& currentSequence;

		//! Flag to prevent code generation when != 0 (used for typeof for example)
		uint32_t codeGenerationLock = 0;
		//! Flag to prevent error generation when != 0
		uint32_t errorGenerationLock = 0;

		// Last pushed indexed parameters
		std::vector<IndexedParameter> lastPushedIndexedParameters;
		// Last pushed named parameters
		std::vector<NamedParameter> lastPushedNamedParameters;

		// Helper for resolving func overloads (reused by each opcode 'call')
		FuncOverloadMatch overloadMatch;

		mutable Logs::Report report;

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

		uint32_t lastOpcodeStacksizeOffset = (uint32_t) -1;

		//! Flag to skip code instanciation as soon as the opcode blueprint size is encountered
		bool shouldSkipCurrentAtom = false;

		struct {
			uint32_t label = 0;
			bool compareTo = false;
		} shortcircuit;

		friend class Nany::IR::Sequence;

	}; // class SequenceBuilder




	struct InstanciateData final
	{
		InstanciateData(Logs::Message::Ptr& report, Atom& atom, ClassdefTableView& cdeftable, nycontext_t& context,
			decltype(FuncOverloadMatch::result.params)& params);

		Logs::Message::Ptr& report;
		//! The atom to instanciate
		Atom& atom;
		//! The parent atom, if any
		Atom* parentAtom = nullptr;
		//! Instance
		uint32_t instanceid = (uint32_t) -1;
		//! The original view to the classdef table
		ClassdefTableView& cdeftable;
		//! Context
		nycontext_t& context;

		//! Parameters used for instanciation
		decltype(FuncOverloadMatch::result.params)& params;
		//!
		Classdef returnType;

		//! Flag to determine whether the code can be generated or not
		bool canGenerateCode = true;
		//! Flag to determine whether the code generate errors or not
		bool canGenerateErrors = true;

		//! Make the layer persistent
		bool shouldMergeLayer = false;
	};

	IR::Sequence* InstanciateAtom(InstanciateData& info);






} // namespace Instanciate
} // namespace Pass
} // namespace Nany

#include "instanciate.hxx"
