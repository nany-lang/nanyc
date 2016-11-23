#pragma once
#include "source.h"


namespace ny {


inline void Source::resetTarget(CTarget* target) {
	ThreadingPolicy::MutexLocker locker{*this};
	m_target = target;
}


} // namespace ny
