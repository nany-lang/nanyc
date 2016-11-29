#pragma once
#include <cstdint>

#ifdef alloca
# undef alloca
#endif

#define LIBNANYC_IR_PRINT_OPCODES 0
#if LIBNANYC_IR_PRINT_OPCODES != 0
#include <iostream>
#endif



namespace ny {
namespace ir {
namespace isa { //!< Instruction Set Architecture


enum class Op : std::uint32_t {
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
	assert(opc <= static_cast<uint32_t>(isa::Op::end)); \
	switch (static_cast<isa::Op>(opc)) \
	{ \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::opand) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::opor) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::opxor) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::opmod) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::eq) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::neq) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::lt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::lte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::ilt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::ilte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::gt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::gte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::igt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::igte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::flt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fgt) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::flte) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fgte) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::negation) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::add) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::sub) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::mul) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::div) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::imul) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::idiv) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fadd) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fsub) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fmul) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fdiv) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::storeConstant) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::store) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::storeText) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::stackalloc) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::jmp) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::jz) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::jnz) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::ref) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::unref) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::push) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::call) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::intrinsic) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::ret) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::allocate) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::dispose) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::stacksize) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::pragma) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::load_u64) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::load_u32) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::load_u8) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::store_u64) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::store_u32) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::store_u8) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memalloc) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memfree) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memrealloc) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memfill) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memcopy) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memmove) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memcmp) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::cstrlen) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::label) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::opassert) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::memcheckhold) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::tpush) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::follow) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::blueprint) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::classdefsizeof) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fieldget) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::fieldset) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::identify) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::identifyset) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::ensureresolved) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::commontype) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::assign) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::self) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::comment) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::debugpos) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::namealias) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::debugfile) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::scope) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::typeisobject) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::qualifiers) \
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::end) \
			\
			LIBNANYC_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, isa::Op::nop) \
	} \




	} // namespace isa
	} // namespace ir
	} // namespace ny
