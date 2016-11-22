#pragma once
#include "../fwd.h"
#include <yuni/core/string.h>
#include "details/ir/isa/opcodes.h"
#include "details/ir/isa/data.h"
#include "details/ir/instruction.h"
#include "details/utils/stringrefs.h"
#include "details/utils/clid.h"
#include <memory>
#include <cassert>

#ifdef alloca
# undef alloca
#endif



namespace ny
{
namespace ir
{


	class Sequence final
	{
	public:
		//! \name Constructors & Destructor
		//@{
		//! Default constructor
		Sequence() = default;
		//! Copy constructor
		Sequence(const Sequence&) = delete;
		//! Destructor
		~Sequence();
		//@}


		//! \name Cursor manipulation
		//@{
		//! Get if a cursor is valid
		bool isCursorValid(const Instruction& instr) const;

		//! Get the upper limit
		void invalidateCursor(const Instruction*& cusror) const;
		//! Get the upper limit
		void invalidateCursor(Instruction*& cusror) const;

		//! Go to the next label
		bool jumpToLabelForward(const Instruction*& cursor, uint32_t label) const;
		//! Go to a previous label
		bool jumpToLabelBackward(const Instruction*& cursor, uint32_t label) const;

		//! Move the cursor at the end of the blueprint
		void moveCursorFromBlueprintToEnd(Instruction*& cursor) const;
		//! Move the cursor at the end of the blueprint
		void moveCursorFromBlueprintToEnd(const Instruction*& cursor) const;
		//@}


		//! \name Opcodes
		//@{
		//! Fetch an instruction at a given offset
		template<ISA::Op O> ISA::Operand<O>& at(uint32_t offset);
		//! Fetch an instruction at a given offset (const)
		template<ISA::Op O> const ISA::Operand<O>& at(uint32_t offset) const;
		//! Fetch an instruction at a given offset
		const Instruction& at(uint32_t offset) const;
		//! Fetch an instruction at a given offset
		Instruction& at(uint32_t offset);

		//! emit a new Instruction
		template<ISA::Op O> ISA::Operand<O>& emit();
		//! emit a new Instruction (without reserving data if needed)
		template<ISA::Op O> ISA::Operand<O>& emitraw();

		//! Get the offset of an instruction within the sequence
		template<ISA::Op O> uint32_t offsetOf(const ISA::Operand<O>& instr) const;
		//! Get the offset of an instruction within the sequence
		uint32_t offsetOf(const Instruction& instr) const;

		//! Emit opcode to hold a pointer for the MemChecker
		void emitMemcheckhold(uint32_t lvid, uint32_t size);

		//! Emit equal
		void emitEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit not equal
		void emitNEQ(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit LT
		void emitLT(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit LTE
		void emitLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit LT
		void emitILT(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit LTE
		void emitILTE(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit GT
		void emitGT(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit GTE
		void emitGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit GT
		void emitIGT(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit GTE
		void emitIGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit FLT
		void emitFLT(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit FLTE
		void emitFLTE(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit FGT
		void emitFGT(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit FGTE
		void emitFGTE(uint32_t lvid, uint32_t lhs, uint32_t rhs);

		//! Emit AND
		void emitAND(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit AND
		void emitOR(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit AND
		void emitXOR(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit AND
		void emitMOD(uint32_t lvid, uint32_t lhs, uint32_t rhs);

		//! Emit not
		void emitNOT(uint32_t lvid, uint32_t lhs);

		//! Emit +
		void emitADD(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit -
		void emitSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit *
		void emitMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit * (signed)
		void emitIMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit /
		void emitDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit / (signed)
		void emitIDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit +
		void emitFADD(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit -
		void emitFSUB(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit *
		void emitFMUL(uint32_t lvid, uint32_t lhs, uint32_t rhs);
		//! Emit * (signed)
		void emitFDIV(uint32_t lvid, uint32_t lhs, uint32_t rhs);

		//! Enter a namespace def
		void emitNamespace(const AnyString& name);

		//! emit Assign variable
		void emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS = true);

		//! Emit intrinsic call
		void emitIntrinsic(uint32_t lvid, const AnyString& name, uint32_t id = (uint32_t) -1);

		//! Emit a blueprint unit opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintUnit(const AnyString& filename);

		//! Emit a blueprint class opcode
		void emitBlueprintClass(const AnyString& name, uint32_t atomid);
		//! Emit a blueprint class opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintClass(uint32_t lvid = 0);
		//! Emit a blueprint typealias opcode
		uint32_t emitBlueprintTypealias(const AnyString& name, uint32_t atomid = static_cast<uint32_t>(-1));

		//! Emit a blueprint func opcode
		void emitBlueprintFunc(const AnyString& name, uint32_t atomid);
		//! Emit a blueprint func opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintFunc();
		//! Emit a blueprint size opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintSize();
		//! Emit a blueprint param opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintParam(LVID, const AnyString&);
		//! Emit a blueprint param opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintParam(LVID);
		//! Emit a blueprint template param opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintGenericTypeParam(LVID, const AnyString&);
		//! Emit a blueprint template param opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintGenericTypeParam(LVID);

		//! Emit a blueprint vardef opcode
		void emitBlueprintVardef(LVID, const AnyString& name);

		//! Emit a stack size increase opcode and give the offset of the instruction in the sequence
		uint32_t emitStackSizeIncrease();
		//! Emit a stack size increase opcode
		uint32_t emitStackSizeIncrease(uint32_t size);

		//! Emit opcode to disable code generation
		void emitPragmaAllowCodeGeneration(bool enabled);
		//! Emit opcode that indicates the begining of a func body
		void emitPragmaFuncBody();

		//! Emit pragma shortcircuit
		void emitPragmaShortcircuit(bool evalvalue);
		//! Emit pragma shortcircuit opcode 'nop' offset
		void emitPragmaShortcircuitMetadata(uint32_t label);
		//! Emit pragma shortcircuit opcode
		void emitPragmaShortcircuitMutateToBool(uint32_t lvid, uint32_t  source);

		//! Emit pragma builtinalias
		void emitPragmaBuiltinAlias(const AnyString& name);
		//! Emit pragma suggest
		void emitPragmaSuggest(bool onoff);
		//! Emit pragma synthetic
		void emitPragmaSynthetic(uint32_t lvid, bool onoff = false);

		//! Emit visibility opcode
		void emitVisibility(nyvisibility_t);

		//! Read a field
		void emitFieldget(uint32_t lvid, uint32_t self, uint32_t varid);
		//! Write a field
		void emitFieldset(uint32_t lvid, uint32_t self, uint32_t varid);
		//@}


		//! \name Iteration
		//@{
		//! Visit each instruction
		template<class T> void each(T& visitor, uint32_t offset = 0);
		//! Visit each instruction (const)
		template<class T> void each(T& visitor, uint32_t offset = 0) const;
		//@}


		//! \name Opcode utils
		//@{
		/*!
		** \brief Iterate through all opcodes and increment all lvid
		**
		** This method is used to insert a posteriori new lvid (for capturing variables for ex.)
		** \param inc The value to add to all lvid
		** \param greaterThan Increment only lvid strictly greater than X
		** \param offset First opcode
		*/
		void increaseAllLVID(uint32_t inc, uint32_t greaterThan, uint32_t offset = 0);
		//@}


		//! \name Memory Management
		//@{
		//! Get how many instructions the sequence has
		uint32_t opcodeCount() const;
		//! Get the capacity of the sequence (in instructions)
		uint32_t capacity() const;
		//! Get the amount of memory in bytes used by the sequence
		size_t sizeInBytes() const;

		//! Clear the sequence
		void clear();
		//! Reserve enough memory for N instructions
		void reserve(uint32_t count);
		//@}


		//! \name Debug
		//@{
		//! Print the sequence to a string
		void print(Yuni::String& out, const AtomMap* = nullptr, uint32_t offset = 0) const;
		//@}


		//! \name Operators
		//@{
		//! Copy operator
		Sequence& operator = (const Sequence&) = delete;
		//@}


	public:
		//! All strings
		StringRefs stringrefs;


	private:
		//! grow to accept N instructions
		void grow(uint32_t instrCount);

	private:
		//! Size of the sequence
		uint32_t m_size = 0u;
		//! Capacity of the sequence
		uint32_t m_capacity = 0u;
		//! m_body of the sequence
		Instruction* m_body = nullptr;

	}; // class Sequence





} // namespace ir
} // namespace ny

#include "sequence.hxx"
