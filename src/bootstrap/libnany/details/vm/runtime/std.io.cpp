#include "std.core.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/io/file.h>
#include "details/vm/vm.h"

using namespace Yuni;




static bool nanyc_io_set_cwd(nyvm_t* vm, void* string, uint32_t size)
{
	assert(vm != nullptr);
	auto& tc = *reinterpret_cast<Nany::VM::ThreadContext*>(vm->tctx);
	tc.cwd = AnyString{reinterpret_cast<const char*>(string), size};
	return true;
}


static const void* nanyc_io_get_cwd(nyvm_t* vm)
{
	assert(vm != nullptr);
	auto& tc = *reinterpret_cast<Nany::VM::ThreadContext*>(vm->tctx);
	return tc.cwd.c_str();
}



namespace Nany
{

	void importNSLIO(IntrinsicTable& intrinsics)
	{
		intrinsics.add("__nanyc_io_set_cwd",  nanyc_io_set_cwd);
		intrinsics.add("__nanyc_io_get_cwd",  nanyc_io_get_cwd);
	}


} // namespace Nany
