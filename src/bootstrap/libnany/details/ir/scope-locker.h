#pragma once
#include "program.h"



namespace Nany
{
namespace IR
{

	struct OpcodeScopeLocker final
	{
		OpcodeScopeLocker(Program& program)
			: program(program)
		{
			program.emitScope();
		}

		~OpcodeScopeLocker()
		{
			program.emitEnd();
		}

		OpcodeScopeLocker(const OpcodeScopeLocker&) = delete;
		OpcodeScopeLocker(OpcodeScopeLocker&&) = default;

		Program& program;
	};


	struct OpcodeCodegenDisabler final
	{
		OpcodeCodegenDisabler(Program& program)
			: program(program)
		{
			program.emitPragmaAllowCodeGeneration(false);
		}

		~OpcodeCodegenDisabler()
		{
			program.emitPragmaAllowCodeGeneration(true);
		}

		OpcodeCodegenDisabler(const OpcodeCodegenDisabler&) = delete;
		OpcodeCodegenDisabler(OpcodeCodegenDisabler&&) = default;

		Program& program;
	};





} // namespace IR
} // namespace Nany
