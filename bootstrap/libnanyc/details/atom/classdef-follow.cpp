#include "classdef-follow.h"

using namespace Yuni;




namespace ny
{

	void ClassdefFollow::print(Yuni::String& out, bool clearBefore) const
	{
		if (clearBefore)
			out.clear();

		for (auto& clid: extends)
			out << "        follow -> " << clid << '\n';

		for (auto& pair: pushedIndexedParams)
			out << "        follow call " << pair.first << " param " << pair.second << '\n';

		for (auto& pair: pushedNamedParams)
			out << "        follow call " << pair.first << " param \"" << pair.second << "\"\n";
	}




} // namespace ny
