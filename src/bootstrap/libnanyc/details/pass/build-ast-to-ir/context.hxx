#pragma once
#include "details/pass/build-ast-to-ir/context.h"




namespace ny
{
namespace IR
{
namespace Producer
{


	inline void Context::invalidateLastDebugLine()
	{
		pPreviousDbgLine = (uint32_t) -1; // forcing debug infos
	}



} // namespace Producer
} // namespace IR
} // namespace ny
