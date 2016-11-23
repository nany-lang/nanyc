#pragma once
#include "classdef-follow.h"


namespace ny {


inline bool ClassdefFollow::empty() const {
	return pushedIndexedParams.empty() and pushedNamedParams.empty();
}


} // namespace ny
