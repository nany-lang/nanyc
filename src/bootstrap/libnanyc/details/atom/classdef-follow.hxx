#pragma once
#include "classdef-follow.h"



namespace Nany
{

	inline bool ClassdefFollow::empty() const
	{
		return pushedIndexedParams.empty() and pushedNamedParams.empty();
	}




} // namespace Nany
