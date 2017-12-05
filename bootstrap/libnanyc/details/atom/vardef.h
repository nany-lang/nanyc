#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/utils/clid.h"
#include "type.h"

namespace ny {

struct Vardef final : public Yuni::NonCopyable<Vardef> {
	Vardef() = default;
	Vardef(Vardef&& other): clid{other.clid} {}
	Vardef& operator = (const Vardef&) = default;
	Vardef& operator = (Vardef&&) = default;

	CLID clid;
};

} // ny
