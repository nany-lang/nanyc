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

	opand,          ///< and
	opor,           ///< or
	opxor,          ///< xor
	opmod,          ///< mod
	eq,             ///< equal
	neq,            ///< not equal
	lt,             ///< less than
	ilt,            ///< less than (signed)
	lte,            ///< less than or equal
	ilte,           ///< less than or equal (signed)
	gt,             ///< greater than
	igt,            ///< greater than (signed)
	gte,            ///< greater than or equal
	igte,           ///< greaterthan or equal (signed)
	flt,            ///< less than (floating point number)
	flte,           ///< less than or equal (floating point number)
	fgt,            ///< greater than (floating point number)
	fgte,           ///< greater than or equal (floating point number)

	negation,       ///< not

	add,            ///< +
	sub,            ///< -
	mul,            ///< *
	div,            ///< /
	imul,           ///< * (signed)
	idiv,           ///< / (signed)
	fadd,           ///< + (floating point number)
	fsub,           ///< - (floating point number)
	fmul,           ///< * (floating point number)
	fdiv,           ///< / (floating point number)

	storeConstant,  ///< set a local variable
	store,          ///< set a local variable
	storeText,      ///< string literal
	stackalloc,     ///< allocate a local variable on the stack

	jmp,            ///< unconditional jump
	jz,             ///< jump if local variable is zero
	jnz,            ///< jump if local variable is not zero

	ref,            ///< increment the reference count
	unref,          ///< decrement the reference count (release it if reaches 0)
	push,           ///< push indexed or named parameter for next function call
	call,           ///< function call
	intrinsic,      ///< compiler intrinsic call
	ret,            ///< return
	allocate,       ///< allocate an object (memory + ctor)
	dispose,        ///< deallocate an object (dtor + memory)

	stacksize,      ///< define the stack size for the current blueprint
	pragma,         ///< metadata for blueprints

	load_u64,       ///< read a 64 bits value from memory
	load_u32,       ///< read a 32 bits value from memory
	load_u8,        ///< read a 8 bits value from memory
	store_u64,      ///< write a 64 bits value in memory
	store_u32,      ///< write a 32 bits value in memory
	store_u8,       ///< write a 8b its value in memory

	memalloc,       ///< allocate a chunk of memory
	memfree,        ///< release a previously allocated chunk of memory
	memrealloc,     ///< reallocate a memory area
	memfill,        ///< fill a memory area with a pattern
	memcopy,        ///< see memcpy
	memmove,        ///< see memmove
	memcmp,         ///< see memcmp
	cstrlen,        ///< see strlen (32 or 64bits)
	label,          ///< define a new label
	opassert,       ///< assert if expr is false
	memcheckhold,   ///< mark a pointer as valid (MemChecker)


	// --- opcodes for compilation only
	identify,       ///< resolve an identifier completely or partially (if overload)
	identifyset,    ///< like 'identify', but perfers setters instead of getters
	ensureresolved, ///< resolve completely type of expr (if not done by 'identify')
	commontype,     ///< determine common type
	assign,         ///< assign a variable to another
	self,           ///< declare a register as 'self'
	tpush,          ///< push a generic type parameter
	follow,         ///< follow
	blueprint,      ///< define a new blueprint (class, func...)
	classdefsizeof, ///< compute the size of a type
	fieldget,       ///< read the value of a variable member
	fieldset,       ///< set the value of a variable member
	debugfile,      ///< current source file
	debugpos,       ///< current position in the current file
	namealias,      ///< variable name
	comment,        ///< just some comments
	scope,          ///< begining of a new scope
	typeisobject,   ///< check if a given type is an object
	qualifiers,     ///< ref, conf

	end,            ///< end of scope (and the last opcode)
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
