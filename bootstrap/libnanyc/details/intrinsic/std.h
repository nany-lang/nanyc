#pragma once
#include <nanyc/vm.h>
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <memory>


namespace { // anonymous

template<class T, typename... Args>
inline T* vm_allocate(nyvmthread_t* const /*vm*/, Args&& ... args) {
	T* object = (T*) malloc(sizeof(T));
	if (YUNI_UNLIKELY(!object))
		throw std::bad_alloc();
	new (object) T(std::forward<Args>(args)...);
	return object;
}

template<class T> inline T* vm_allocateraw(nyvmthread_t* const /*vm*/, size_t size) {
	T* ptr = (T*) malloc(size);
	if (YUNI_UNLIKELY(!ptr))
		throw std::bad_alloc();
	return ptr;
}

template<class T> inline void vm_deallocate(nyvmthread_t* const vm, T* object) {
	assert(object != nullptr);
	object->~T();
	free(object); // sizeof(T);
}

inline void vm_deallocate(nyvmthread_t* const /*vm*/, void* object, size_t /*size*/) {
	assert(object != nullptr);
	free(object);
}

inline nybool_t to_nybool(bool v) {
	return v ? nytrue : nyfalse;
}

} // namespace
