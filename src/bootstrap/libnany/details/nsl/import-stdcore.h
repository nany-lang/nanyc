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


	template<class T, typename... Args> T* tctx_allocate(nyprogram_cf_t* const tctx, Args&&... args);

	template<class T> T* tctx_allocateraw(nyprogram_cf_t* const tctx, size_t size);

	template<class T> void tctx_deallocate(nyprogram_cf_t* const tctx, T* object);

	void tctx_deallocate(nyprogram_cf_t* const tctx, void* object, size_t size);

	void tctx_print(nyprogram_cf_t* const tctx, const AnyString& msg);



	template<class T, typename... Args> inline T* tctx_allocate(nyprogram_cf_t* const tctx, Args&&... args)
	{
		assert(tctx);
		T* object = (T*) tctx->allocator.allocate(&tctx->allocator, sizeof(T));
		if (YUNI_UNLIKELY(!object))
			throw std::bad_alloc();
		new (object) T(std::forward<Args>(args)...);
		return object;
	}

	template<class T> inline T* tctx_allocateraw(nyprogram_cf_t* const tctx, size_t size)
	{
		T* ptr = (T*) tctx->allocator.allocate(&tctx->allocator, size);
		if (YUNI_UNLIKELY(!ptr))
			throw std::bad_alloc();
		return ptr;
	}


	template<class T> inline void tctx_deallocate(nyprogram_cf_t* const tctx, T* object)
	{
		assert(object != nullptr);
		object->~T();
		tctx->allocator.deallocate(&tctx->allocator, object, sizeof(T));
	}

	inline void tctx_deallocate(nyprogram_cf_t* const tctx, void* object, size_t size)
	{
		assert(object != nullptr);
		tctx->allocator.deallocate(&tctx->allocator, object, size);
	}


	inline void tctx_print(nyprogram_cf_t* const tctx, const AnyString& msg)
	{
		tctx->console.write_stdout(tctx->console.internal, msg.c_str(), msg.size());
	}



} // namespace Nany
