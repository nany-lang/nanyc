#include "intrinsic-table.h"
#include "nany/nany.h"

using namespace Yuni;




namespace Nany
{

	static uint8_t intrinsic_bool_false()
	{
		return 0;
	}

	static uint8_t intrinsic_bool_true()
	{
		return 1;
	}




	void IntrinsicTable::registerBool()
	{
		add("bool_false", intrinsic_bool_false);
		add("bool_true", intrinsic_bool_true);
	}



} // namespace Nany
