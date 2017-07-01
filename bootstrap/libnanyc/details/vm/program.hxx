#pragma once
#include "program.h"


namespace ny {


inline vm::Program& ref(nyoldprogram_t* const ptr) {
	assert(ptr != nullptr);
	return *(reinterpret_cast<ny::vm::Program*>(ptr));
}


inline const vm::Program& ref(const nyoldprogram_t* const ptr) {
	assert(ptr != nullptr);
	return *(reinterpret_cast<const ny::vm::Program*>(ptr));
}


namespace vm {


inline nyoldprogram_t* Program::self() {
	return reinterpret_cast<nyoldprogram_t*>(this);
}


inline const nyoldprogram_t* Program::self() const {
	return reinterpret_cast<const nyoldprogram_t*>(this);
}


inline void Program::printStderr(const AnyString& msg) {
	cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
}


} // namespace vm
} // namespace ny
