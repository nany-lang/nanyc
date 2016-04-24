#include "import-stdcore.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/core/process/program.h>

using namespace Yuni;




namespace Nany
{
namespace Builtin
{


	static bool yn_os_process_execute(nyvm_t*, void* string, uint32_t timeout)
	{
		AnyString cmd = *(reinterpret_cast<String*>(string));
		return Process::Execute(cmd, timeout);
	}



} // namespace Builtin
} // namespace Nany



namespace Nany
{

	void importNSLOSProcess(IntrinsicTable& intrinsics)
	{
		intrinsics.add("yuni.os.execute",   Builtin::yn_os_process_execute);
	}


} // namespace Nany
