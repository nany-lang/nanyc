#pragma once
#include "details/vm/context.h"
#include <iostream>


namespace ny {
namespace vm {


inline nytctx_t* Context::self() {
	return reinterpret_cast<nytctx_t*>(this);
}


inline const nytctx_t* Context::self() const {
	return reinterpret_cast<const nytctx_t*>(this);
}


} // namespace vm
} // namespace ny
