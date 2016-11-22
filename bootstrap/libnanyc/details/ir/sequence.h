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

		//! emit Assign variable
		void emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS = true);

		//! Emit a blueprint typealias opcode
		uint32_t emitBlueprintTypealias(const AnyString& name, uint32_t atomid = static_cast<uint32_t>(-1));

		//! Emit a blueprint func opcode
		void emitBlueprintFunc(const AnyString& name, uint32_t atomid);
		//! Emit a blueprint func opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintFunc();
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
