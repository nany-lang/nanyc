#pragma once
#include "target.h"


namespace Nany
{

	inline AnyString CTarget::name() const
	{
		return pName;
	}


	template<class T>
	inline void CTarget::eachSource(const T& callback)
	{
		for (auto& ptr: pSources)
			callback(*ptr);
	}



} // namespace nany
