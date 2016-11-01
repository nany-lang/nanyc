#pragma once
#include <cstdint>

#ifdef alloca
# undef alloca
#endif

#define LIBNANYC_IR_PRINT_OPCODES 0
#if LIBNANYC_IR_PRINT_OPCODES != 0
#include <iostream>
#endif



namespace Nany
{
namespace IR
{
namespace ISA //!< Instruction Set Architecture
{


	enum class Op: std::uint32_t
	{
		//! nop opcode (or unknown)
		nop = 0,

		//! and
		opand,
		//! or
		opor,
		//! xor
		opxor,
		//! mod
		opmod,
		//! equal
		eq,
		//! not equal
		neq,
		//! less than
		lt,
		//! less than (signed)
		ilt,
		//! less than or equal
		lte,
		//! less than or equal (signed)
		ilte,
		//! greater than
		gt,
		//! greater than (signed)
		igt,
		//! greater than or equal
		gte,
		//! greaterthan or equal (signed)
		igte,
		//! less than (floating point bumber)
		flt,
		//! less than or equal (floating point bumber)
		flte,
		//! greater than (floating point bumber)
		fgt,
		//! greater than or equal (floating point bumber)
		fgte,

		//! not
		negation,

		//! +
		add,
		//! -
		sub,
		//! *
		mul,
		//! /
		div,
		//! * (signed)
		imul,
		//! / (signed)
		idiv,

		//! +
		fadd,
		//! -
		fsub,
		//! *
		fmul,
		//! /
		fdiv,


		//! Store a constant value in a register
		storeConstant,
		//! Store value from another register
		store,
		//! Store a context text
		storeText,
		//! alloca (on the stack)
		stackalloc,

		//! unconditional jump
		jmp,
		//! jump if local variable is zero
		jz,
		//! jump if local variable is not zero
		jnz,


		//! acquire object
		ref,
		//! release object
		unref,
		//! push indexed parameter
		push,
		//! call a function
		call,
		//! call an intrinsic function
		intrinsic,
		//! return value
		ret,
		//! Allocate object
		allocate,
		//! Release an object (no ref counting)
		dispose,

		//! Increase stack size
		stacksize,
		//! pragma
		pragma,

		//! Read 64bits from memory
		load_u64,
		//! Read 32bits from emmory
		load_u32,
		//! Read 8bits from emmory
		load_u8,
		//! Write 64bits into memory
		store_u64,
		//! Write 32bits into memory
		store_u32,
		//! Write 8bits into memory
		store_u8,
		//! Allocate a region of memory
		memalloc,
		//! Free a region of memory previously allocated by 'memalloc'
		memfree,
		//! Reallocate a pointer
		memrealloc,
		//! Fill a memory with a pattern
		memfill,
		//! Memcopy
		memcopy,
		//! Memcopy
		memmove,
		//! Memcmp
		memcmp,
		//! strlen (32/64)
		cstrlen,

		//! label
		label,

		//! Assert expression
		opassert,
		//! Register a valid pointer (used for MemChecker)
		memcheckhold,


		// --- opcodes for compilation only

		//! try to identity an identifier (partially or completely from its referer)
		identify,
		//! try to identity an id setter if ambigous
		// (this opcode is strictly identical to 'identify')
		identifyset,
		//! try to identify completely the type of sub expression
		// (if not already done by 'identity')
		ensureresolved,
		//! common type
		commontype,
		//! assign a variable to another
		assign,
		//! declare a register as 'self'
		self,
		//! push a generic type parameter
		tpush,
		//! follow
		follow,
		//! Blueprint
		blueprint,
		//! sizeof
		classdefsizeof,
		//! field read
		fieldget,
		//! field write
		fieldset,
		//! the current source file
		debugfile,
		//! the current position in the current source file
		debugpos,
		//! named alias
		namealias,
		//! Comment within the sequence
		comment,
		//! begining of a new scope
		scope,
		//! Check if a given type is an object
		typeisobject,
		//! Qualifiers
		qualifiers,
		//! inherit
		inherit,
		//! end of a scope
		end,
	};





	#if LIBNANYC_IR_PRINT_OPCODES != 0
	#define __LIBNANYC_IR_PRINT_OPCODE(OPCODE) std::cout << " -- opc " << (void*) this << " -- " << opc << " as " << #OPCODE << std::endl;
	#else
	#define __LIBNANYC_IR_PRINT_OPCODE(OPCODE)
	#endif

	#define LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, OPCODE) \
		case OPCODE: \
		{ \
			__LIBNANYC_IR_PRINT_OPCODE(OPCODE) \
			(VISITOR).visit(reinterpret_cast<PREFIX<(OPCODE)>&>((IT))); \
			break; \
		}


	#define LIBNANYC_IR_VISIT_SEQUENCE(PREFIX,VISITOR, IT) \
		uint32_t opc = (IT).opcodes[0]; \
		assert(opc <= static_cast<uint32_t>(ISA::Op::end)); \
		switch (static_cast<ISA::Op>(opc)) \
		{ \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opand) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opor) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opxor) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opmod) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::eq) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::neq) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::lt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::lte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ilt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ilte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::gt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::gte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::igt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::igte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::flt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fgt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::flte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fgte) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::negation) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::add) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::sub) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::mul) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::div) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::imul) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::idiv) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fadd) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fsub) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fmul) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fdiv) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::storeConstant) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::storeText) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::stackalloc) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::jmp) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::jz) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::jnz) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ref) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::unref) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::push) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::call) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::intrinsic) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ret) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::allocate) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::dispose) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::stacksize) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::pragma) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::load_u64) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::load_u32) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::load_u8) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store_u64) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store_u32) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store_u8) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memalloc) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memfree) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memrealloc) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memfill) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memcopy) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memmove) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memcmp) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::cstrlen) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::label) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opassert) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memcheckhold) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::tpush) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::follow) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::blueprint) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::classdefsizeof) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fieldget) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fieldset) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::identify) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::identifyset) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ensureresolved) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::commontype) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::assign) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::self) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::comment) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::debugpos) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::namealias) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::debugfile) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::scope) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::typeisobject) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::qualifiers) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::inherit) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::end) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::nop) \
		} \




} // namespace ISA
} // namespace IR
} // namespace Nany
