#include "std.core.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>

using namespace Yuni;




static uint32_t nanyc_strlen(nyvm_t*, void* string)
{
	if (string)
	{
		size_t l = strlen(reinterpret_cast<const char*>(string));
		return  (l < static_cast<uint32_t>(-1)) ? static_cast<uint32_t>(l) : 0u;
	}
	return 0u;
}




namespace Nany
{

	void importNSLMemory(IntrinsicTable& intrinsics)
	{
		intrinsics.add("__nanyc_strlen",  nanyc_strlen);
	}


} // namespace Nany
