#pragma once
#include "emit.h"



namespace ny
{
namespace ir
{

	struct OpcodeScopeLocker final
	{
		OpcodeScopeLocker(Sequence& sequence)
			: sequence(sequence)
		{
			ir::emit::scopeBegin(sequence);
		}

		~OpcodeScopeLocker()
		{
			ir::emit::scopeEnd(sequence);
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
			ir::emit::pragma::codegen(sequence, false);
		}

		~OpcodeCodegenDisabler()
		{
			ir::emit::pragma::codegen(sequence, true);
		}

		OpcodeCodegenDisabler(const OpcodeCodegenDisabler&) = delete;
		OpcodeCodegenDisabler(OpcodeCodegenDisabler&&) = default;

		Sequence& sequence;
	};


} // namespace ir
} // namespace ny
