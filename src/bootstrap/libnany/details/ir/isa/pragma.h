#pragma once
#include <yuni/yuni.h>




namespace Nany
{
namespace IR
{
namespace ISA //!< Instruction Set Architecture
{

	enum class Pragma: uint32_t
	{
		//! Unknown pragma / invalid
		unknown = 0,
		//! Code generation flag
		codegen,


		// --- pragma for compilation only

		//! Size of the blueprint, in opcodes
		blueprintsize,

		//! Visibility modifier
		visibility,
		//! body start,
		bodystart,

		//! shortcircuit attribute
		shortcircuit,
		//! shortcircuit sequence offset of 'nop' instructions
		shortcircuitOpNopOffset,
		//! Builtin alias
		builtinalias,

		//! Suggestion (for error reporting)
		suggest,

		//! The maximum number of elements, for integrity check
		max,
	};


	/*!
	** \internal the total number of items must currently be < 2^4 (see data struct)
	** \see struct Operand<ISA::Op::blueprint>
	*/
	enum class Blueprint
	{
		//! Function definition
		funcdef,
		//! Variable member definition
		vardef,
		//! Class definition
		classdef,
		//! Typedef
		typealias,
		//! Parameter definition (for a function or a class)
		param,
		//! Parameter definition, with auto assignment
		paramself,
		//! Namespace definition (one part of it)
		namespacedef
	};





} // namespace ISA
} // namespace IR
} // namealias Nany
