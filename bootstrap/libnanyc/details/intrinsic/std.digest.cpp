#include "details/intrinsic/std.h"
#include "details/intrinsic/catalog.h"
#include <yuni/core/hash/checksum/md5.h>

using namespace Yuni;

static void* nyinx_digest_md5(nyvmthread_t* vm, const char* string, uint64_t length) {
	Hash::Checksum::MD5 md5;
	md5.fromRawData(string, length);
	if (not md5.value().empty()) {
		uint32_t size = md5.value().size();
		uint32_t capacity = size + ny::config::extraObjectSize;
		char* cstr = (char*) malloc(capacity);
		if (cstr) {
			const char* src = md5.value().c_str();
			for (uint32_t i = 0; i != size; ++i)
				cstr[i] = src[i];
			vm->returnValue.size     = md5.value().size();
			vm->returnValue.capacity = md5.value().size();
			vm->returnValue.data     = cstr;
			return &vm->returnValue;
		}
	}
	return nullptr;
}

namespace ny {
namespace intrinsic {
namespace import {

void digest(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc_digest_md5", nyinx_digest_md5);
}

} // namespace import
} // namespace intrinsic
} // namespace ny
