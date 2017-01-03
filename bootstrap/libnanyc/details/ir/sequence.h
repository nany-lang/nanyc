#pragma once
#include "libnanyc.h"
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



namespace ny {
namespace ir {


struct Sequence final {
	Sequence() = default;
	Sequence(const Sequence&) = delete;
	~Sequence();

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
	template<isa::Op O> isa::Operand<O>& at(uint32_t offset);
	//! Fetch an instruction at a given offset (const)
	template<isa::Op O> const isa::Operand<O>& at(uint32_t offset) const;
	//! Fetch an instruction at a given offset
	const Instruction& at(uint32_t offset) const;
	//! Fetch an instruction at a given offset
	Instruction& at(uint32_t offset);

	//! emit a new Instruction
	template<isa::Op O> isa::Operand<O>& emit();
	//! emit a new Instruction (without reserving data if needed)
	template<isa::Op O> isa::Operand<O>& emitraw();

	//! Get the offset of an instruction within the sequence
	template<isa::Op O> uint32_t offsetOf(const isa::Operand<O>& instr) const;
	//! Get the offset of an instruction within the sequence
	uint32_t offsetOf(const Instruction& instr) const;
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
	void grow(uint32_t instrCount);

private:
	//! Size of the sequence
	uint32_t m_size = 0u;
	//! Capacity of the sequence
	uint32_t m_capacity = 0u;
	//! m_body of the sequence
	Instruction* m_body = nullptr;

}; // Sequence


} // namespace ir
} // namespace ny

#include "sequence.hxx"
