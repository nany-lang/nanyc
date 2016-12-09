#pragma once
#include "stacktrace.h"


namespace ny {
namespace vm {


inline void Stacktrace<true>::push(uint32_t atomid, uint32_t instanceid) {
	if (unlikely(not (++topframe < upperLimit)))
		grow();
	*topframe = {atomid, instanceid};
}


inline void Stacktrace<true>::pop() noexcept {
	assert(topframe > baseframe);
	--topframe;
}


} // namespace vm
} // namespace ny
