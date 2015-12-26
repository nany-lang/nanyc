#include "local-variables.h"

using namespace Yuni;



namespace Nany
{


	LocalVariables::LocalVariables(ClassdefTable& classdefs, yuint32 groupid)
		: classdefs(classdefs)
		, pAtomID(groupid)
		, pMaxVarID(0)
	{}


	void LocalVariables::clear()
	{
		pMaxVarID = 0;
		pLocalVars.clear();
		pLocalVars.shrink_to_fit();
	}


	void LocalVariables::bulkCreate(yuint32 count)
	{
		if (likely(count != 0))
		{
			++count; // 1-based

			// creating local variables
			pLocalVars.resize(count);
			pMaxVarID = (LVID) count;
			if (pLocalVars.size() > 512)
				pLocalVars.shrink_to_fit(); // TODO: reduce memory consumption - useful ?

			for (uint32 i = 1; i != (uint32) pLocalVars.size(); ++i)
				pLocalVars[i].clid.reclass(pAtomID, i);
		}
		else
		{
			if (pMaxVarID != 0)
				clear();
		}
	}





} // namespace Nany
