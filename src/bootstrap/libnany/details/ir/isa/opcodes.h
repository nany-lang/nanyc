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

	enum class Pragma: yuint32
	{
		//! Unknown pragma / invalid
		unknown = 0,
		//! Code generation flag
		codegen,

		// --- pragma for compilation only

		//! Namespace definition (one part of it)
		namespacedef,
		//! Function definition
		blueprintfuncdef,
		//! Variable member definition
		blueprintvar,
		//! Class definition
		blueprintclassdef,
		//! Parameter definition (for a function or a class)
		blueprintparam,
		//! Parameter definition, with auto assignment
		blueprintparamself,
		//! Size of the blueprint, in opcodes
		blueprintsize,
		//! Visibility modifier
		visibility,
		//! body start,
		bodystart,
		//! The maximum number of elements, for integrity check
		max,
	};




	enum class Op: std::uint32_t
	{
		//! nop opcode (or unknown)
		nop = 0,
		//! Store a constant value in a register
		storeConstant,
		//! Store value from another register
		store,
		//! Store a context text
		storeText,
		//! alloca (on the stack)
		stackalloc,
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


		// --- opcodes for compilation only

		//! fetch a variable or a function
		identify,
		//! assign a variable to another
		assign,
		//! declare a register as 'self'
		self,
		//! follow
		follow,
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
		//! Comment within the program
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


	#define LIBNANY_IR_VISIT_PROGRAM(PREFIX,VISITOR, IT) \
	do \
	{ \
		uint32_t opc = (IT).opcodes[0]; \
		if (opc <= static_cast<uint32_t>(ISA::Op::end)) \
		{ \
			switch ((ISA::Op) opc) \
			{ \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::storeConstant) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::store) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::storeText) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::stackalloc) \
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
				\
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::follow) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::classdefsizeof) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fieldget) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::fieldset) \
				LIBNANY_IR_VISIT_OPCODE(PREFIX, VISITOR, IT, ISA::Op::identify) \
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
