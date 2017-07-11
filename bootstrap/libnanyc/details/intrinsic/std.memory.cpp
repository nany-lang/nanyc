#include "std.core.h"
#include "details/intrinsic/catalog.h"
#include <limits>

template<class T>
static T nyinx_strlen(nyvmthread_t*, void* string) {
	size_t len = string ? strlen(reinterpret_cast<const char*>(string)) : 0u;
	return (sizeof(size_t) == sizeof(T) or len < std::numeric_limits<T>::max())
		   ? static_cast<T>(len)
		   : static_cast<T>(-1);
}

namespace ny {
namespace intrinsic {
namespace import {

void memory(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("strlen32",  nyinx_strlen<uint32_t>);
	intrinsics.emplace("strlen64",  nyinx_strlen<uint64_t>);
}

} // namespace import
} // namespace intrinsic
} // namespace ny
