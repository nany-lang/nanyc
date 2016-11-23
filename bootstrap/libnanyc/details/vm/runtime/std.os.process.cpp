#include "std.core.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/core/process/program.h>

using namespace Yuni;


static bool nanyc_os_process_execute(nyvm_t*, const char* cmd, uint32_t len, uint32_t timeout) {
	return Process::Execute(AnyString{cmd, len}, timeout);
}


namespace ny {


void importNSLOSProcess(IntrinsicTable& intrinsics) {
	intrinsics.add("__nanyc_os_execute",   nanyc_os_process_execute);
}


} // namespace ny
