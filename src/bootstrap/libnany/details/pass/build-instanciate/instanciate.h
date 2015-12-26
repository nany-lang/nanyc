#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include "details/reporting/report.h"
#include "details/atom/signature.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/atom/atom.h"
#include "details/atom/func-overload-match.h"
#include "details/intrinsic/intrinsic-table.h"
#include "details/ir/program.h"
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






	class ProgramBuilder final
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor (importSignature must be called after)
		ProgramBuilder(Logs::Report report, ClassdefTableView&, const IntrinsicTable& intrinsics,
			IR::Program& out, IR::Program& program);

		//! Prepare the first local registers according the given signature
		void pushParametersFromSignature(LVID atomid, const Signature&);

		//! Destructor
		~ProgramBuilder();
		//@}


		bool readAndInstanciate(uint32_t offset);

		void disableCodeGeneration();
		bool canGenerateCode() const;

		void printClassdefTable(Logs::Report trace, const AtomStackFrame& frame) const;


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
		void visit(const IR::ISA::Operand<IR::ISA::Op::comment>&);
		void visit(const IR::ISA::Operand<IR::ISA::Op::pragma>&);
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
		//! visitor - fallback
		template<enum IR::ISA::Op O> void visit(const IR::ISA::Operand<O>&);
		//@}


		//! perform type resolution and fetch data (local variable, func...)
		bool identify(const IR::ISA::Operand<IR::ISA::Op::identify>& operands);

		bool pragmaBlueprint(const IR::ISA::Operand<IR::ISA::Op::pragma>& operands);
		void pragmaBodyStart();

		bool instanciateAtomClass(Atom& atom);
		bool instanciateAtomClassDestructor(Atom& atom, uint32_t self);
		bool instanciateAtomClassClone(Atom& atom, uint32_t self, uint32_t rhs);

		bool instanciateAtomFunc(uint32_t& instanceid, Atom& funcAtom, uint32_t retlvid, uint32_t p1 = 0, uint32_t p2 = 0);


		bool instanciateFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>& operands);

		bool instanciateAssignment(const IR::ISA::Operand<IR::ISA::Op::call>& operands);
		bool instanciateAssignment(AtomStackFrame& frame, LVID lhs, LVID rhs, bool canDisposeLHS = true,
			bool checktype = true, bool forceDeepcopy = false);

		//! Declare a named variable (and checks for multiple declarations)
		void declareNamedVariable(const AnyString& name, LVID lvid, bool autorelease = true);

		bool complainUnknownIdentifier(const Atom* self, const Atom& atom, const AnyString& name);

		void releaseScopedVariables(int scope, bool forget = false);

		void adjustSettingsNewFuncdefOperator(const AnyString& name);
		void generateMemberVarDefaultInitialization();
		void generateMemberVarDefaultDispose();
		void generateMemberVarDefaultClone();

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

		void instanciateInstrinsicCall();
		bool instanciateUserDefinedIntrinsic(const IR::ISA::Operand<IR::ISA::Op::intrinsic>& operands);
		bool instanciateBuiltinIntrinsic(const AnyString& name, uint32_t lvid);
		bool instanciateIntrinsicFieldset(uint32_t lvid);
		bool instanciateIntrinsicRef(uint32_t lvid);
		bool instanciateIntrinsicUnref(uint32_t lvid);
		bool instanciateIntrinsicAddressof(uint32_t lvid);
		bool instanciateIntrinsicSizeof(uint32_t lvid);
		bool instanciateIntrinsicMemalloc(uint32_t lvid);
		bool instanciateIntrinsicMemFree(uint32_t lvid);
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

		bool complainMultipleOverloads(LVID lvid, const std::vector<std::reference_wrapper<Atom>>& solutions,
			const OverloadedFuncCallResolver& resolver);

		bool complainRedeclared(const AnyString& name, uint32_t previousDeclaration);

		bool complainIntrinsicParameter(const AnyString& name, uint32_t pindex, const Classdef& got, const AnyString& expected);

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
		//! intrinsics
		const IntrinsicTable& intrinsics;
		// New opcode program
		IR::Program& out;
		// Current program
		IR::Program& currentProgram;

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

		friend class Nany::IR::Program;

	}; // class ProgramBuilder




	struct InstanciateData final
	{
		InstanciateData(Logs::Message::Ptr& report, Atom& atom, ClassdefTableView& cdeftable, const IntrinsicTable& intrinsics,
						decltype(FuncOverloadMatch::result.params)& params)
			: report(report)
			  , atom(atom)
			  , cdeftable(cdeftable)
			  , intrinsics(intrinsics)
			  , params(params)
		{
			returnType.mutateToAny();
		}

		Logs::Message::Ptr& report;
		//! The atom to instanciate
		Atom& atom;
		//! The parent atom, if any
		Atom* parentAtom = nullptr;
		//! Instance
		uint32_t instanceid = (uint32_t) -1;
		//! The original view to the classdef table
		ClassdefTableView& cdeftable;
		//! Alias to the intrinsic table
		const IntrinsicTable& intrinsics;

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

	IR::Program* InstanciateAtom(InstanciateData& info);






} // namespace Instanciate
} // namespace Pass
} // namespace Nany

#include "instanciate.hxx"
