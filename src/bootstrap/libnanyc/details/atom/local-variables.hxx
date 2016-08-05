#pragma once
#include "local-variables.h"




namespace Nany
{


	inline bool LocalVariables::makeHardlink(LVID source, LVID target)
	{
		if (likely(source < pMaxVarID and target < pMaxVarID))
			return classdefs.makeHardlink(pLocalVars[source].clid, pLocalVars[target].clid);
		return false;
	}


	inline bool LocalVariables::exists(LVID lvid) const
	{
		return likely(lvid < pMaxVarID) and (lvid != 0);
	}


	inline Vardef& LocalVariables::vardef(LVID lvid)
	{
		assert(exists(lvid) and "invalid lvid for retrieving vardef");
		return likely(0 != lvid) ? pLocalVars[lvid] : pLocalVars[0];
	}


	inline const Vardef& LocalVariables::vardef(LVID lvid) const
	{
		assert(exists(lvid) and "invalid lvid for retrieving vardef");
		return likely(0 != lvid) ? pLocalVars[lvid] : pLocalVars[0];
	}




} // namespace Nany
