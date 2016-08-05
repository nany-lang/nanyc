#pragma once
#include "sequence.h"



namespace Nany
{
namespace IR
{

	struct OpcodeScopeLocker final
	{
		OpcodeScopeLocker(Sequence& sequence)
			: sequence(sequence)
		{
			sequence.emitScope();
		}

		~OpcodeScopeLocker()
		{
			sequence.emitEnd();
		}

		OpcodeScopeLocker(const OpcodeScopeLocker&) = delete;
		OpcodeScopeLocker(OpcodeScopeLocker&&) = default;

		Sequence& sequence;
	};


	struct OpcodeCodegenDisabler final
	{
		OpcodeCodegenDisabler(Sequence& sequence)
			: sequence(sequence)
		{
			sequence.emitPragmaAllowCodeGeneration(false);
		}

		~OpcodeCodegenDisabler()
		{
			sequence.emitPragmaAllowCodeGeneration(true);
		}

		OpcodeCodegenDisabler(const OpcodeCodegenDisabler&) = delete;
		OpcodeCodegenDisabler(OpcodeCodegenDisabler&&) = default;

		Sequence& sequence;
	};





} // namespace IR
} // namespace Nany
