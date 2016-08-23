#include "runtime.h"
#include "details/intrinsic/intrinsic-table.h"
#include "details/vm/thread-context.h"
#include <yuni/core/hash/checksum/md5.h>
#include <iostream>

using namespace Yuni;




static void* nanyc_digest_md5(nyvm_t* vm, const char* string, uint64_t length)
{
	auto& tc = *reinterpret_cast<Nany::VM::ThreadContext*>(vm->tctx);
	if (string and length)
	{
		Hash::Checksum::MD5 md5;
		md5.fromRawData(string, length);
		if (not md5.value().empty())
		{
			tc.returnValue.size     = md5.value().size();
			tc.returnValue.capacity = md5.value().capacity();
			tc.returnValue.data     = md5.forgetContent();
			return &tc.returnValue;
		}
	}
	return nullptr;
}


namespace Nany
{

	void importNSLDigest(IntrinsicTable& intrinsics)
	{
		intrinsics.add("__nanyc_digest_md5",   nanyc_digest_md5);
	}

} // namespace Nany
