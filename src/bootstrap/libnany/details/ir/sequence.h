#pragma once
#include "../fwd.h"
#include <yuni/core/string.h>
#include "details/ir/isa/opcodes.h"
#include "details/ir/isa/pragma.h"
#include "details/ir/isa/data.h"
#include "details/ir/instruction.h"
#include "details/utils/stringrefs.h"
#include "details/utils/clid.h"
#include <memory>
#include <cassert>

#ifdef alloca
# undef alloca
#endif



namespace Nany
{
namespace IR
{

	struct OpcodeScopeLocker;


	class Sequence final
	{
	public:
		//! \name Constructors & Destructor
		//@{
		//! Default constructor
		Sequence();
		//! Copy constructor
		Sequence(const Sequence&) = delete;
		//! Destructor
		~Sequence();
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

		//! Get the upper limit
		void invalidateCursor(const Instruction*& cusror) const;
		//! Get the upper limit
		void invalidateCursor(Instruction*& cusror) const;

		//! Go to the next label
		void jumpToLabelForward(const Instruction*& cursor, uint32_t label) const;
		//! Go to a previous label
		void jumpToLabelBackward(const Instruction*& cursor, uint32_t label) const;


		//! Emit a NOP instruction
		void emitNop();

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

		//! Allocate a new variable on the stack and get the register
		uint32_t emitStackalloc(uint32_t lvid, nytype_t);
		//! Allocate a new variable on the stack and assign a value to it and get the register
		uint32_t emitStackalloc_u64(uint32_t lvid, nytype_t, uint64_t);
		//! Allocate a new variable on the stack and assign a value to it and get the register
		uint32_t emitStackalloc_f64(uint32_t lvid, nytype_t, double);
		//! Allocate a new variable on the stack and assign a text to it and get the register
		uint32_t emitStackallocText(uint32_t lvid, const AnyString&);

		//! Copy two register
		void emitStore(uint32_t lvid, uint32_t source);
		void emitStore_u64(uint32_t lvid, uint64_t);
		void emitStore_f64(uint32_t lvid, double);
		uint32_t emitStoreText(uint32_t lvid, const AnyString&);

		//! Emit a memalloc opcode and get the register
		uint32_t emitMemalloc(uint32_t lvid, uint32_t regsize);
		//! Emit a memfree opcode and get the register
		void emitMemFree(uint32_t lvid, uint32_t regsize);

		//! Enter a namespace def
		void emitNamespace(const AnyString& name);

		//! Emit a debug filename opcode
		void emitDebugfile(const AnyString& filename);
		//! Emit debug position
		void emitDebugpos(uint32_t line, uint32_t offset);

		//! Emit allocate object
		uint32_t emitAllocate(uint32_t lvid, uint32_t atomid);
		//! Emit acquire object
		void emitRef(uint32_t lvid);
		//! Unref objct
		void emitUnref(uint32_t lvid, uint32_t atomid, uint32_t instanceid);

		//! emit Assign variable
		void emitAssign(uint32_t lhs, uint32_t rhs, bool canDisposeLHS = true);

		//! Emit a push parameter
		void emitPush(uint32_t lvid);
		//! Emit a named parameter
		void emitPush(uint32_t lvid, const AnyString& name);
		//! Emit a call opcode
		void emitCall(uint32_t lvid, uint32_t ptr2func);
		//! Emit a call opcode for atom
		void emitCall(uint32_t lvid, uint32_t atomid, uint32_t instanceid);
		//! Emit intrinsic call
		void emitIntrinsic(uint32_t lvid, const AnyString& name, uint32_t id = (uint32_t) -1);

		//! Emit a identify opcode
		void emitIdentify(uint32_t lvid, const AnyString& name, uint32_t self);
		//! Emit a self opcode
		void emitSelf(uint32_t self);

		//! Emit a sizeof opcode
		void emitSizeof(uint32_t lvid, uint32_t type);

		//! Emit an opcode to create a name alias
		void emitNameAlias(uint32_t lvid, const AnyString& name);

		//! Add a new comment
		void emitComment(const AnyString& text);
		//! Add a new empty line
		void emitComment();

		//! Emit opcode for type checking
		void emitTypeIsObject(uint32_t lvid);

		//! Emit opcode for adding/removing ref qualifier
		void emitQualifierRef(uint32_t lvid, bool flag);
		//! Emit opcode for adding/removing const qualifier
		void emitQualifierConst(uint32_t lvid, bool flag);

		//! Emit a blueprint unit opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintUnit();

		//! Emit a blueprint class opcode
		void emitBlueprintClass(const AnyString& name, uint32_t atomid);
		//! Emit a blueprint class opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintClass();
		//! Emit a blueprint typealias opcode
		void emitBlueprintTypealias(const AnyString& name, uint32_t lvid);

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
		uint32_t emitBlueprintTmplParam(LVID, const AnyString&);
		//! Emit a blueprint template param opcode and give the offset of the instruction in the sequence
		uint32_t emitBlueprintTmplParam(LVID);

		//! Emit a blueprint vardef opcode
		void emitBlueprintVardef(LVID, const AnyString&);

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
		//! Emit pragma builtinalias
		void emitPragmaBuiltinAlias(const AnyString& name);
		//! Emit pragma suggest
		void emitPragmaSuggest(bool onoff);
		//! Emit visibility opcode
		void emitVisibility(nyvisibility_t);

		//! Emit qualifiers copy
		void emitInheritQualifiers(uint32_t lhs, uint32_t rhs);

		//! Emit a label
		void emitLabel(uint32_t labelid);

		//! Emit a new scope
		void emitScope();
		//! Emit the end of a scope
		void emitEnd();

		//! Read a field
		void emitFieldget(uint32_t lvid, uint32_t self, uint32_t varid);
		//! Write a field
		void emitFieldset(uint32_t lvid, uint32_t self, uint32_t varid);

		//! Emit a return opcode
		void emitReturn(uint32_t lvid = 0);

		//! Emit an unconditional jump
		void emitJmp(uint32_t label);
		//! Emit jump if zero
		void emitJz(uint32_t lvid, uint32_t result, uint32_t label);
		//! Emit jump if not zero
		void emitJnz(uint32_t lvid, uint32_t result, uint32_t label);


		//! Visit each instruction (const)
		template<class T> void each(T& visitor, uint32_t offset = 0);
		template<class T> void each(T& visitor, uint32_t offset = 0) const;
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
		//! Shrink the memory used by the pBody
		void shrink();
		//! Reserve enough memory for N instructions
		void reserve(uint32_t N);
		//@}


		//! \name Atom relationship
		//@{
		//! Get if the sequence is attached to an atom
		bool hasAtomParent() const;
		//@}


		//! \name Debug
		//@{
		//! Print the sequence to a string
		void print(Yuni::String& out, const AtomMap* = nullptr) const;
		//! Print the sequence to a clob
		void print(Yuni::Clob& out, const AtomMap* = nullptr) const;

		//! Get the raw pointer of an offset in memory (for "watching" changes from gdb)
		const void* pointer(uint32_t offset) const;

		//! Get the gdb command to watch changes of a given offset
		Yuni::String gdbMemoryWatch(uint32_t offset) const;
		//@}

	public:
		//! All strings
		StringRefs stringrefs;


	private:
		//! grow to accept N instructions
		void grow(uint32_t newcapa);

	private:
		//! Size of the sequence
		uint32_t pSize = 0;
		//! Capacity of the sequence
		uint32_t pCapacity = 0;
		//! pBody of the sequence
		std::unique_ptr<Instruction, decltype(std::free)*> pBody;
		//! Attaced Atom, if any
		Atom* pAtom = nullptr;

	}; // class Sequence





} // namespace IR
} // namespace Nany

#include "sequence.hxx"
#include "scope-locker.h"
