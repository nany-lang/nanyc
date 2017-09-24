#pragma once
#include "libnanyc.h"
#include <yuni/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include "libnanyc-config.h"
#include <array>
#include "details/atom/ctype.h"


namespace ny {
namespace intrinsic {


//! Definition of a single user-defined intrinsic
struct Intrinsic final {
	Intrinsic(void* callback, uint32_t id): callback(callback), id(id) {}

	//! C-Callback
	void* callback = nullptr;
	//! Intrinsic ID
	const uint32_t id;
	//! The return type
	CType rettype = CType::t_void;
	//! The total number of parameters
	uint32_t paramcount = 0;
	//! All parameter types
	std::array<CType, config::maxPushedParameters> params;

}; // struct Intrinsic


} // namespace intrinsic
} // namespace ny
