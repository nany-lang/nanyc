#pragma once
#include "../../fwd.h"
#include <yuni/thread/mutex.h>
#include <vector>
#include <utility>
#include "details/atom/atom.h"
#include "details/errors/errors.h"
#include "details/ir/isa/opcodes.h"
#include "details/ir/instruction.h"




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

		//! Information for capturing variables
		struct CaptureVariables {
			//! Get if allowed to capture variables
			bool enabled() const { return atom != nullptr; }
			void enabled(Atom*);

			//! Try to list of all unknown identifiers, potential candidates for capture
			Atom* atom = nullptr;
			//! all local named variables (name / scope)
			std::unordered_map<AnyString, uint32_t> knownVars;
		}
		capture;

		Atom& currentAtomNotUnit()
		{
			return (not atom.isUnit()) ? atom : (*(atom.parent));
		}

	}; // class AtomStackFrame



	class SequenceMapping final
	{
	public:
		SequenceMapping(ClassdefTable& cdeftable, Yuni::Mutex& mutex, IR::Sequence& sequence);

		bool map(Atom& parentAtom, uint32_t offset = 0);


	public:
		//! The classdef table (must be protected by 'mutex' in some passes)
		ClassdefTable& cdeftable;
		//! Mutex for the cdeftable
		Yuni::Mutex& mutex;

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

		const char* currentFilename = nullptr;
		uint32_t currentLine = 0;
		uint32_t currentOffset = 0;

		bool needAtomDbgFileReport = false;
		bool needAtomDbgOffsetReport = false;
		//! Flag to evaluate the whole sequence, or only a portion of it
		bool evaluateWholeSequence = true;

		//! The first atom created by the mapping
		// This value might be used when a mapping is done on the fly
		// (while instanciating code for example)
		Atom* firstAtomCreated = nullptr;

		//! Prefix to prepend for the first atom created by the mapping
		AnyString prefixNameForFirstAtomCreated;

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
		void visit(IR::ISA::Operand<IR::ISA::Op::identifyset>&);
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

		void printError(const IR::Instruction& operands, AnyString msg = nullptr);

	private:
		void attachFuncCall(const IR::ISA::Operand<IR::ISA::Op::call>&);

		void deleteAllFrames();
		static void retriveReportMetadata(void* self, Logs::Level, const AST::Node*, Yuni::String&, uint32_t&, uint32_t&);

		void mapBlueprintFuncdefOrTypedef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands);
		void mapBlueprintClassdef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands);
		void mapBlueprintParam(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands);
		void mapBlueprintVardef(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands);
		void mapBlueprintNamespace(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands);
		void mapBlueprintUnit(IR::ISA::Operand<IR::ISA::Op::blueprint>& operands);

	private:
		//! Error reporting
		Logs::MetadataHandler localMetadataHandler;

	}; // class SequenceMapping






} // namespace Mapping
} // namespace Pass
} // namespace Nany
