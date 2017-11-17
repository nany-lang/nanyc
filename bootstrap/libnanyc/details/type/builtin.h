#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <nany/nany.h>

namespace ny::Type::Builtin {

template<class T> struct TypeToEnumValue final {
};

template<> struct TypeToEnumValue<void> final {
	using NativeType = void;
	enum { value = nyt_void, size = 0 };
	static AnyString toTypeString() {
		return "void";
	}
};

# define NY_DECLARE_BUILTIN(TYPE, ENUM, SIZE) \
	template<> struct TypeToEnumValue<TYPE> final \
	{ \
		using NativeType = yuint64; \
		enum { value = ENUM, size = SIZE }; \
		static AnyString toTypeString() { return #TYPE; } \
	}

NY_DECLARE_BUILTIN(bool,     nyt_bool, 1);
NY_DECLARE_BUILTIN(void*,    nyt_ptr,  sizeof(void*));

NY_DECLARE_BUILTIN(uint64_t, nyt_u64,  8);
NY_DECLARE_BUILTIN(uint32_t, nyt_u32,  4);
NY_DECLARE_BUILTIN(uint16_t, nyt_u16,  2);
NY_DECLARE_BUILTIN(uint8_t,  nyt_u8,   1);

NY_DECLARE_BUILTIN(int64_t,  nyt_i64,  8);
NY_DECLARE_BUILTIN(int32_t,  nyt_i32,  4);
NY_DECLARE_BUILTIN(int16_t,  nyt_i16,  2);
NY_DECLARE_BUILTIN(int8_t,   nyt_i8,   1);

NY_DECLARE_BUILTIN(double,   nyt_f64,  sizeof(double));
NY_DECLARE_BUILTIN(float,    nyt_f32,  sizeof(float));

#undef NY_DECLARE_BUILTIN

} // ny::Type::Builtin
