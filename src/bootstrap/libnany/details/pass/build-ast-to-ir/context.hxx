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


	inline void Context::emitTmplParameters(const std::vector<std::pair<uint32_t, AnyString>>& list)
	{
		for (auto& pair: list)
		{
			if (pair.second.empty())
				sequence.emitTPush(pair.first);
			else
				sequence.emitTPush(pair.first, pair.second);
		}
	}





} // namespace Producer
} // namespace IR
} // namespace Nany
