#pragma once
#include "func-overload-match.h"



namespace Nany
{


	inline bool FuncOverloadMatch::hasAtLeastOneParameter(Atom& atom) const
	{
		return not atom.parameters.empty() or not input.params.indexed.empty()
			or not input.params.named.empty();
	}



} // namespace Nany
