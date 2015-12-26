#pragma once
#include "source.h"



namespace Nany
{

	inline void Source::resetTarget(CTarget* target)
	{
		ThreadingPolicy::MutexLocker locker{*this};
		pTarget = target;
	}




} // namespace Nany
