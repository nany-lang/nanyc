#include "std.core.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/core/process/program.h>

using namespace Yuni;




static bool nanyc_os_process_execute(nyvm_t*, void* string, uint32_t timeout)
{
	AnyString cmd = *(reinterpret_cast<String*>(string));
	return Process::Execute(cmd, timeout);
}





namespace Nany
{

	void importNSLOSProcess(IntrinsicTable& intrinsics)
	{
		intrinsics.add("nanyc.os.execute",   nanyc_os_process_execute);
	}

} // namespace Nany
