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


template<class T, typename... Args> inline T* Project::allocate(Args&& ... args) {
	T* object = (T*) cf.allocator.allocate(&cf.allocator, sizeof(T));
	if (YUNI_UNLIKELY(!object))
		throw std::bad_alloc();
	new (object) T(std::forward<Args>(args)...);
	return object;
}


template<class T> inline void Project::deallocate(T* object) {
	assert(object != nullptr);
	object->~T();
	cf.allocator.deallocate(&cf.allocator, object, sizeof(T));
}


} // namespace ny
