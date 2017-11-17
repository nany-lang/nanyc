#pragma once
#include "libnanyc.h"

namespace ny {
namespace {

constexpr uint32_t localvarVoid = 0;
constexpr uint32_t localvarAny  = (uint32_t) - 1;

inline bool isAny(uint32_t localvar) {
	return localvar == localvarAny;
}

inline bool isVoid(uint32_t localvar) {
	return localvar == localvarVoid;
}

inline bool isValidLocalvar(uint32_t localvar) {
	return localvar != 0 and localvar != (uint32_t) - 1;
}

} // namespace
} // namespace ny
