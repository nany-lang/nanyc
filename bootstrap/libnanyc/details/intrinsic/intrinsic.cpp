#include "intrinsic.h"

using namespace Yuni;



namespace ny
{

	YString Intrinsic::print() const
	{
		String out;
		out << "{" << name << '(';
		for (uint32_t i = 0; i != paramcount; ++i)
		{
			if (i != 0)
				out << ", ";
			out << nytype_to_cstring(params[i]);
		}
		out << "): " << nytype_to_cstring(rettype);
		return out;
	}



} // namespace ny
