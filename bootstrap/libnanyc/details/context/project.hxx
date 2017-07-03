#pragma once
#include "project.h"


namespace ny {


inline nyproject_t* Project::self() {
	return reinterpret_cast<nyproject_t*>(this);
}


inline const nyproject_t* Project::self() const {
	return reinterpret_cast<const nyproject_t*>(this);
}


inline Project& ref(nyproject_t* const ptr) {
	assert(ptr != nullptr);
	return *(reinterpret_cast<ny::Project*>(ptr));
}


inline const Project& ref(const nyproject_t* const ptr) {
	assert(ptr != nullptr);
	return *(reinterpret_cast<const ny::Project*>(ptr));
}

} // namespace ny
