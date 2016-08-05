#include "classdef-overloads.h"
#include "details/atom/atom.h"

using namespace Yuni;



namespace Nany
{

	void ClassdefOverloads::print(String& out, const ClassdefTableView& table, bool clearBefore) const
	{
		if (clearBefore)
			out.clear();

		for (auto& atom: pOverloads)
		{
			out << "        overload ";
			atom.get().retrieveCaption(out, table);
			out << '\n';
		}
	}





} // namespace Nany
