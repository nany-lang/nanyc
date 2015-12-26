#pragma once
#include "func-overload-match.h"



namespace Nany
{


	inline bool FuncOverloadMatch::hasAtLeastOneParameter(Atom& atom) const
	{
		return not atom.parameters.empty() or not input.indexedParams.empty()
			or not input.namedParams.empty();
	}



} // namespace Nany
