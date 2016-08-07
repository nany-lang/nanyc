#pragma once
#include "build.h"



namespace Nany
{


	inline Build::Build(Project& project, const nybuild_cf_t& cf, bool async)
		: cf(cf)
		, project(project)
		, isAsync(async)
	{
		project.addRef();
	}


	inline Build::~Build()
	{
		// clear internal containers before releasing the project itself
		pAttachedSequences.clear();
		sources.clear();
		targets.clear();

		if (project.release())
			project.destroy();
	}


	inline nybuild_t* Build::self()
	{
		return reinterpret_cast<nybuild_t*>(this);
	}


	inline const nybuild_t* Build::self() const
	{
		return reinterpret_cast<const nybuild_t*>(this);
	}


	inline Build& ref(nybuild_t* const ptr)
	{
		assert(ptr != nullptr);
		return *(reinterpret_cast<Nany::Build*>(ptr));
	}

	inline const Build& ref(const nybuild_t* const ptr)
	{
		assert(ptr != nullptr);
		return *(reinterpret_cast<const Nany::Build*>(ptr));
	}


	template<class T, typename... Args> inline T* Build::allocate(Args&&... args)
	{
		T* object = (T*) cf.allocator.allocate(&cf.allocator, sizeof(T));
		if (YUNI_UNLIKELY(!object))
			throw std::bad_alloc();
		new (object) T(std::forward<Args>(args)...);
		return object;
	}

	template<class T> inline T* Build::allocateraw(size_t size)
	{
		T* ptr = (T*) cf.allocator.allocate(&cf.allocator, size);
		if (YUNI_UNLIKELY(!ptr))
			throw std::bad_alloc();
		return ptr;
	}


	template<class T> inline void Build::deallocate(T* object)
	{
		assert(object != nullptr);
		object->~T();
		cf.allocator.deallocate(&cf.allocator, object, sizeof(T));
	}

	inline void Build::deallocate(void* object, size_t size)
	{
		assert(object != nullptr);
		cf.allocator.deallocate(&cf.allocator, object, size);
	}


	inline void Build::printStderr(const AnyString& msg)
	{
		cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
	}

	inline void Build::cerrColor(nycolor_t color)
	{
		cf.console.set_color(cf.console.internal, nycerr, color);
	}



} // namespace Nany
