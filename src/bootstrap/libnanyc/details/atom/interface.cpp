#include "interface.h"

using namespace Yuni;



namespace ny
{

	void ClassdefInterface::print(Yuni::String& out, bool clearBefore) const
	{
		if (clearBefore)
			out.clear();

		bool hasSelf = !(!pSelf);
		if (hasSelf)
		{
			out << "        self: ";
			pSelf->print(out, false);
			out << '\n';
		}

		for (auto& funcdef: pInterface)
		{
			out << "        ";
			if (hasSelf)
				out << "    ";
			funcdef->print(out, false);
			out << '\n';
		}
	}





} // namespace ny
