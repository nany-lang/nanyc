#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include "nany/nany.h"
#include <memory>


namespace Nany
{

	class Project;
	class IntrinsicTable;



	/*!
	** \brief Nany Standard Library 'std.core'
	*/
	void importNSLCore(Project&);

	/*!
	** \brief Import intrinsics related to string manipulation
	*/
	void importNSLCoreString(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to process manipulation
	*/
	void importNSLIONative(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to process manipulation
	*/
	void importNSLOSProcess(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to environment variables manipulation
	*/
	void importNSLEnv(IntrinsicTable&);


	template<class T, typename... Args> T* vm_allocate(nyvm_t* const vm, Args&&... args);

	template<class T> T* vm_allocateraw(nyvm_t* const vm, size_t size);

	template<class T> void vm_deallocate(nyvm_t* const vm, T* object);

	void vm_deallocate(nyvm_t* const vm, void* object, size_t size);

	void vm_print(nyvm_t* const vm, const AnyString& msg);



	template<class T, typename... Args> inline T* vm_allocate(nyvm_t* const vm, Args&&... args)
	{
		assert(vm);
		T* object = (T*) vm->allocator->allocate(vm->allocator, sizeof(T));
		if (YUNI_UNLIKELY(!object))
			throw std::bad_alloc();
		new (object) T(std::forward<Args>(args)...);
		return object;
	}

	template<class T> inline T* vm_allocateraw(nyvm_t* const vm, size_t size)
	{
		T* ptr = (T*) vm->allocator->allocate(vm->allocator, size);
		if (YUNI_UNLIKELY(!ptr))
			throw std::bad_alloc();
		return ptr;
	}


	template<class T> inline void vm_deallocate(nyvm_t* const vm, T* object)
	{
		assert(object != nullptr);
		object->~T();
		vm->allocator->deallocate(vm->allocator, object, sizeof(T));
	}

	inline void vm_deallocate(nyvm_t* const vm, void* object, size_t size)
	{
		assert(object != nullptr);
		vm->allocator->deallocate(vm->allocator, object, size);
	}


	inline void vm_print(nyvm_t* const vm, const AnyString& msg)
	{
		vm->console->write_stdout(vm->console->internal, msg.c_str(), msg.size());
	}



} // namespace Nany
