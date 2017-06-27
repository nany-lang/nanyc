#pragma once
#include <yuni/core/string.h>


namespace ny {

//! Nany Language builtin types
enum class CType: uint32_t {
	/*! No type */
	t_void,
	/*! Custom user type */
	t_any,
	/*! Raw pointer (arch dependent) */
	t_ptr,
	/*! Boolean (nytrue/nyfalse) */
	t_bool,
	/*! Unsigned 8  bits integer */
	t_u8,
	/*! Unsigned 16 bits integer */
	t_u16,
	/*! Unsigned 32 bits integer */
	t_u32,
	/*! Unsigned 64 bits integer */
	t_u64,
	/*! Signed 8  bits integer */
	t_i8,
	/*! Signed 16 bits integer */
	t_i16,
	/*! Signed 32 bits integer */
	t_i32,
	/*! Signed 64 bits integer */
	t_i64,
	/*! Floating-point number 32 bits */
	t_f32,
	/*! Floating-point number 64 bits */
	t_f64
};

constexpr uint32_t ctypeCount = static_cast<uint32_t>(CType::t_f64) + 1;

CType toCType(const AnyString&);

AnyString toString(CType);

uint32_t ctypeSizeof(CType);

} // namespace ny
