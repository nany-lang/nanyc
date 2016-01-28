#pragma once
#include "../../fwd.h"
#include <vector>
#include <utility>
#include "details/atom/atom.h"
#include "details/reporting/report.h"
#include "details/ir/isa/opcodes.h"
#include "details/context/isolate.h"




namespace Nany
{
namespace Pass
{
namespace Mapping
{

	//! Stack frame per atom definition (class, func)
	struct AtomStackFrame final
	{
		AtomStackFrame(Atom& atom): atom(atom) {}
		//! The current atom
		Atom& atom;
		//! The current scope depth for the current stack frame
		uint scope = 0;
		uint parameterIndex = 0;
		//! Convenient classdefs alias
		std::vector<CLID> classdefs;
	};



	class SequenceMapping final
	{
	public:
		SequenceMapping(Logs::Report& report, Isolate& isolate, IR::Sequence& sequence);

		bool map(Atom& root, uint32_t offset = 0);


	public:
		//! Isolate
		Isolate& isolate;
		//! Blueprint root element
		std::vector<AtomStackFrame> atomStack;
		//! Last lvid (for pushed parameters)
		LVID lastLVID;
		//! Last pushed indexed parameters
		std::vector<LVID> lastPushedIndexedParameters;
		//! Last pushed named parameters
		std::vector<std::pair<AnyString, LVID>> lastPushedNamedParameters;
		//! exit status
		bool success;
		//! Current sequence
		IR::Sequence& currentSequence;
		//! Step
		Logs::Report report;

		const char* currentFilename;
		uint currentLine;
		uint currentOffset;
		bool needAtomDbgFileReport = false;
		bool needAtomDbgOffsetReport = false;

		IR::Instruction** cursor = nullptr;

		//! Flag to evaluate the whole sequence, or only a portion of it
		bool evaluateWholeSequence = true;


	public:
		void visit(IR::ISA::Operand<IR::ISA::Op::pragma>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::stacksize>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::scope>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::end>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::self>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::identify>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::push>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::call>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::ret>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::follow>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::intrinsic>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::debugfile>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::debugpos>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::qualifiers>&);
		template<IR::ISA::Op O> void visit(IR::ISA::Operand<O>& operands);


	private:
		template<IR::ISA::Op O> void printError(const IR::ISA::Operand<O>& operands, AnyString msg = nullptr);
		template<IR::ISA::Op O> bool checkForLVID(const IR::ISA::Operand<O>& operands, LVID lvid);
		void attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>&);
		Logs::Report error();
		void resetClassdefOriginFromCurrentPosition(Classdef& cdef);

	}; // class SequenceMapping






} // namespace Mapping
} // namespace Pass
} // namespace Nany
