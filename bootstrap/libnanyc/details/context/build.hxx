#pragma once
#include "build.h"


namespace ny {


inline nybuild_t* Build::self() {
	return reinterpret_cast<nybuild_t*>(this);
}


inline const nybuild_t* Build::self() const {
	return reinterpret_cast<const nybuild_t*>(this);
}


inline Build& ref(nybuild_t* const ptr) {
	assert(ptr != nullptr);
	return *(reinterpret_cast<ny::Build*>(ptr));
}

inline const Build& ref(const nybuild_t* const ptr) {
	assert(ptr != nullptr);
	return *(reinterpret_cast<const ny::Build*>(ptr));
}

inline void Build::printStderr(const AnyString& msg) {
	cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
}

inline void Build::cerrColor(nycolor_t color) {
	cf.console.set_color(cf.console.internal, nycerr, color);
}


} // namespace ny
