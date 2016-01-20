#pragma once
#include "details/pass/build-ast-to-ir/context.h"




namespace Nany
{
namespace IR
{
namespace Producer
{


	inline Context::Context(Sequence& sequence, Logs::Report report)
		: sequence(sequence)
		, report(report)
	{}







} // namespace Producer
} // namespace IR
} // namespace Nany
