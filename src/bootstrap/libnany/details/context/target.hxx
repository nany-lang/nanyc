#pragma once
#include "target.h"


namespace Nany
{

	inline void CTarget::resetContext(Context* ctx)
	{
		ThreadingPolicy::MutexLocker locker{*this};
		pContext = ctx;
	}





} // namespace nany
