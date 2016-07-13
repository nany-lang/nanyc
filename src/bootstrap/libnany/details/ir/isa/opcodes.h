#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <type_traits>
#include <yuni/core/static/if.h>

#ifdef alloca
# undef alloca
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

		//! label
		label,

		//! Assert expression
		opassert,


		// --- opcodes for compilation only

		//! try to identity an identifier (partially or completely from its referer)
		identify,
		//! try to identity an id setter if ambigous
		// (this opcode is strictly identical to 'identify')
		identifyset,
		//! try to identify completely the type of sub expression
		// (if not already done by 'identity')
		ensureresolved,
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






	#define LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, OPCODE) \
		case OPCODE: \
		{ \
			(VISITOR).visit(reinterpret_cast<PREFIX<(OPCODE)>&>((IT))); \
			break; \
		}


	#define LIBNANY_IR_VISIT_SEQUENCE(PREFIX,VISITOR, IT) \
	do \
	{ \
		uint32_t opc = (IT).opcodes[0]; \
		if (opc <= static_cast<uint32_t>(ISA::Op::end)) \
		{ \
			switch ((ISA::Op) opc) \
			{ \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opand) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opor) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opxor) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opmod) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::eq) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::neq) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::lt) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::lte) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ilt) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ilte) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::gt) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::gte) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::igt) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::igte) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::flt) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fgt) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::flte) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fgte) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::negation) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::add) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::sub) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::mul) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::div) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::imul) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::idiv) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fadd) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fsub) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fmul) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fdiv) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::storeConstant) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::storeText) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::stackalloc) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::jmp) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::jz) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::jnz) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ref) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::unref) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::push) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::call) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::intrinsic) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ret) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::allocate) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::dispose) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::stacksize) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::pragma) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memalloc) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memfree) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memrealloc) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memfill) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::memcopy) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::load_u64) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::load_u32) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::load_u8) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store_u64) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store_u32) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store_u8) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::label) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::opassert) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::tpush) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::follow) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::blueprint) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::classdefsizeof) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fieldget) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fieldset) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::identify) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::identifyset) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::ensureresolved) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::assign) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::self) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::comment) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::debugpos) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::namealias) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::debugfile) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::scope) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::typeisobject) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::qualifiers) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::inherit) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::end) \
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::nop) \
			} \
		} \
		else \
		{ \
			assert(false and "invalid opcode"); \
		} \
	} \
	while (0)




} // namespace ISA
} // namespace IR
} // namespace Nany
