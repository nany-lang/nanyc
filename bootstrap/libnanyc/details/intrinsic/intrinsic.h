#pragma once
#include "libnanyc.h"
#include <yuni/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include "nany/nany.h"
#include "libnanyc-config.h"
#include <array>


namespace ny {
namespace intrinsic {


//! Definition of a single user-defined intrinsic
struct Intrinsic final {
	Intrinsic(void* callback, uint32_t id): callback(callback), id(id) {}

	//! C-Callback
	void* callback = nullptr;
	//! Intrinsic ID
	uint32_t id = (uint32_t) - 1;
	//! The return type
	nytype_t rettype = nyt_void;
	//! The total number of parameters
	uint32_t paramcount = 0;
	//! All parameter types
	std::array<nytype_t, config::maxPushedParameters> params;

}; // struct Intrinsic


} // namespace intrinsic
} // namespace ny
