#pragma once
#include "thread-context.h"
#include <iostream>


namespace ny {
namespace vm {


inline nytctx_t* ThreadContext::self() {
	return reinterpret_cast<nytctx_t*>(this);
}


inline const nytctx_t* ThreadContext::self() const {
	return reinterpret_cast<const nytctx_t*>(this);
}


inline void ThreadContext::cerr(const AnyString& msg) {
	cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
}


inline void ThreadContext::cerrColor(nycolor_t color) {
	cf.console.set_color(cf.console.internal, nycerr, color);
}


} // namespace vm
} // namespace ny
