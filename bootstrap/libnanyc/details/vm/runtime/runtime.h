#pragma once
#include "nany/nany.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <memory>


namespace { // anonymous

template<class T, typename... Args>
inline T* vm_allocate(nyoldvm_t* const vm, Args&& ... args) {
	assert(vm);
	T* object = (T*) vm->allocator->allocate(vm->allocator, sizeof(T));
	if (YUNI_UNLIKELY(!object))
		throw std::bad_alloc();
	new (object) T(std::forward<Args>(args)...);
	return object;
}

template<class T> inline T* vm_allocateraw(nyoldvm_t* const vm, size_t size) {
	T* ptr = (T*) vm->allocator->allocate(vm->allocator, size);
	if (YUNI_UNLIKELY(!ptr))
		throw std::bad_alloc();
	return ptr;
}


template<class T> inline void vm_deallocate(nyoldvm_t* const vm, T* object) {
	assert(object != nullptr);
	object->~T();
	vm->allocator->deallocate(vm->allocator, object, sizeof(T));
}


inline void vm_deallocate(nyoldvm_t* const vm, void* object, size_t size) {
	assert(object != nullptr);
	vm->allocator->deallocate(vm->allocator, object, size);
}


inline void vm_print(nyoldvm_t* const vm, const AnyString& msg) {
	vm->console->write_stdout(vm->console->internal, msg.c_str(), msg.size());
}


inline nybool_t to_nybool(bool v) {
	return v ? nytrue : nyfalse;
}


} // anonymous namespace
