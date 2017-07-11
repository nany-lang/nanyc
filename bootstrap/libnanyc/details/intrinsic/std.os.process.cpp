#include "std.core.h"
#include "details/intrinsic/catalog.h"
#include <yuni/core/process/program.h>

using namespace Yuni;

static bool nyinx_os_process_execute(nyvmthread_t*, const char* cmd, uint32_t len, uint32_t timeout) {
	return Process::Execute(AnyString{cmd, len}, timeout);
}

namespace ny {
namespace intrinsic {
namespace import {

void process(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc_os_execute",   nyinx_os_process_execute);
}

} // namespace import
} // namespace intrinsic
} // namespace ny
