#include "details/intrinsic/std.h"
#include "details/intrinsic/catalog.h"

using namespace Yuni;

static void nanyc_cout(nyvmthread_t* tctx, void* string, uint32_t length) {
	assert(length == 0 or string != nullptr);
	tctx->cout.write(&tctx->cout, (const char*) string, length);
}

template<class T> struct IntCast    { using value = T;        };
template<> struct IntCast<int8_t>   { using value = int32_t;  };
template<> struct IntCast<uint8_t>  { using value = uint32_t; };
template<> struct IntCast<int16_t>  { using value = int32_t;  };
template<> struct IntCast<uint16_t> { using value = uint32_t; };

template<class T> static uint32_t nanyc_string_append(nyvmthread_t*, void* string, T value) {
	ShortString64 str;
	str << static_cast<typename IntCast<T>::value>(value);
	memcpy(string, str.c_str(), str.size());
	return str.size();
}

static uint32_t nanyc_string_append_ptr(nyvmthread_t*, void* string, void* ptr) {
	ShortString32 str;
	str << ptr;
	memcpy(string, str.c_str(), str.size());
	return str.size();
}

namespace ny {
namespace intrinsic {
namespace import {

void string(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc.string.append.u8",   nanyc_string_append<uint8_t>);
	intrinsics.emplace("__nanyc.string.append.u16",  nanyc_string_append<uint16_t>);
	intrinsics.emplace("__nanyc.string.append.u32",  nanyc_string_append<uint32_t>);
	intrinsics.emplace("__nanyc.string.append.u64",  nanyc_string_append<uint64_t>);
	intrinsics.emplace("__nanyc.string.append.i8",   nanyc_string_append<int8_t>);
	intrinsics.emplace("__nanyc.string.append.i16",  nanyc_string_append<int16_t>);
	intrinsics.emplace("__nanyc.string.append.i32",  nanyc_string_append<int32_t>);
	intrinsics.emplace("__nanyc.string.append.i64",  nanyc_string_append<int64_t>);
	intrinsics.emplace("__nanyc.string.append.f32",  nanyc_string_append<float>);
	intrinsics.emplace("__nanyc.string.append.f64",  nanyc_string_append<double>);
	intrinsics.emplace("__nanyc.string.append.ptr",  nanyc_string_append_ptr);
	intrinsics.emplace("__nanyc.cout", nanyc_cout);
}

} // namespace import
} // namespace intrinsic
} // namespace ny
