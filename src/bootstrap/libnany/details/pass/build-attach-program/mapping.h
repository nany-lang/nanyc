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
		AtomStackFrame(Atom& atom, std::unique_ptr<AtomStackFrame>& next)
			: atom(atom), next(std::move(next))
		{}
		//! The current atom
		Atom& atom;
		//! The current scope depth for the current stack frame
		uint32_t scope = 0u;
		uint32_t parameterIndex = 0u;
		//! Convenient classdefs alias
		std::vector<CLID> classdefs;
		// Next frame
		std::unique_ptr<AtomStackFrame> next;

	public:
		Atom& currentAtomNotUnit()
		{
			return (not atom.isUnit()) ? atom : (*(atom.parent));
		}

	}; // class AtomStackFrame



	class SequenceMapping final
	{
	public:
		SequenceMapping(Logs::Report& report, Isolate& isolate, IR::Sequence& sequence);

		bool map(Atom& root, uint32_t offset = 0);


	public:
		//! Isolate
		Isolate& isolate;
		//! Blueprint root element
		std::unique_ptr<AtomStackFrame> atomStack;
		//! Last lvid (for pushed parameters)
		LVID lastLVID = 0;
		//! Last pushed indexed parameters
		std::vector<LVID> lastPushedIndexedParameters;
		//! Last pushed named parameters
		std::vector<std::pair<AnyString, LVID>> lastPushedNamedParameters;
		//! exit status
		bool success = false;
		//! Current sequence
		IR::Sequence& currentSequence;
		//! Step
		Logs::Report report;

		const char* currentFilename = nullptr;
		uint32_t currentLine = 0;
		uint32_t currentOffset = 0;
		bool needAtomDbgFileReport = false;
		bool needAtomDbgOffsetReport = false;

		//! Flag to evaluate the whole sequence, or only a portion of it
		bool evaluateWholeSequence = true;

		//! cursor for iterating through all opcocdes
		IR::Instruction** cursor = nullptr;


	public:
		void visit(IR::ISA::Operand<IR::ISA::Op::pragma>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::blueprint>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::stacksize>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::scope>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::end>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::stackalloc>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::self>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::identify>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::push>&);
		void visit(IR::ISA::Operand<IR::ISA::Op::tpush>&);
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
		void printError(const IR::Instruction& operands, AnyString msg = nullptr);
		template<IR::ISA::Op O> bool checkForLVID(const IR::ISA::Operand<O>& operands, LVID lvid);
		void attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>&);
		Logs::Report error();
		void resetClassdefOriginFromCurrentPosition(Classdef& cdef);

		void pushNewFrame(Atom& atom);
		void deleteAllFrames();

	}; // class SequenceMapping






} // namespace Mapping
} // namespace Pass
} // namespace Nany
