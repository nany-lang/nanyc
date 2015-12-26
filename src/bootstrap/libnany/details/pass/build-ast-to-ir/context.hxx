#pragma once
#include "details/pass/build-ast-to-ir/context.h"




namespace Nany
{
namespace IR
{
namespace Producer
{


	inline Context::Context(Program& program, Logs::Report report)
		: program(program)
		, report(report)
	{}







} // namespace Producer
} // namespace IR
} // namespace Nany
