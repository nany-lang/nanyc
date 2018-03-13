#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/utils/clid.h"
#include "type.h"

namespace ny {

struct Vardef final {
	Vardef() = default;
	Vardef(const CLID& clid): clid{clid} {}
	Vardef(CLID&& clid): clid(std::move(clid)) {}
	Vardef(Vardef&& other): clid{std::move(other.clid)} {}
	Vardef& operator = (const Vardef&) = default;
	Vardef& operator = (const CLID& clid) { this->clid = clid; return *this; }
	Vardef& operator = (Vardef&&) = default;

	CLID clid;
};

} // ny
