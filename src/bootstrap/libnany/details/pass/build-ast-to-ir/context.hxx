#pragma once
#include "details/pass/build-ast-to-ir/context.h"




namespace Nany
{
namespace IR
{
namespace Producer
{


	inline Context::Context(nybuild_cf_t& cf, Sequence& sequence, Logs::Report report)
		: cf(cf)
		, sequence(sequence)
		, report(report)
	{}





} // namespace Producer
} // namespace IR
} // namespace Nany
