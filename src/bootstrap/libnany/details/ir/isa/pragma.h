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

		//! shortcircuit attribute
		shortcircuit,
		//! shortcircuit program offset of 'nop' instructions
		shortcircuitOpNopOffset,
		//! Builtin alias
		builtinalias,

		//! Suggestion (for error reporting)
		suggest,

		//! The maximum number of elements, for integrity check
		max,
	};






} // namespace ISA
} // namespace IR
} // namealias Nany
