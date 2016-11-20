#pragma once
#include "context.h"




namespace ny
{
namespace ir
{
namespace Producer
{


	inline void Context::invalidateLastDebugLine()
	{
		pPreviousDbgLine = (uint32_t) -1; // forcing debug infos
	}



} // namespace Producer
} // namespace ir
} // namespace ny
