#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/project.h"

using namespace Yuni;



extern "C" void nany_project_cf_init(nyproject_cf_t* cf)
{
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nyproject_cf_t));
	nany_memalloc_set_default(&(cf->allocator));
}


extern "C" nyproject_t* nany_project_create(const nyproject_cf_t* cf)
{
	Nany::Project* project;
	try
	{
		if (cf)
		{
			auto& allocator = const_cast<nyallocator_t&>(cf->allocator);
			void* inplace = allocator.allocate(&allocator, sizeof(Nany::Project));
			if (!inplace)
				return nullptr;
			project = new (inplace) Nany::Project(*cf);
		}
		else
		{
			nyproject_cf_t ncf;
			nany_project_cf_init(&ncf);

			auto& allocator = const_cast<nyallocator_t&>(ncf.allocator);
			void* inplace = allocator.allocate(&allocator, sizeof(Nany::Project));
			if (!inplace)
				return nullptr;
			project = new (inplace) Nany::Project(ncf);
		}
	}
	catch (...)
	{
		return nullptr;
	}

	try
	{
		// making sure that user-events do not destroy the project by mistake
		project->addRef();
		// initialize the project after incrementing the ref count
		project->init();

		return project->self();
	}
	catch (...)
	{
		project->destroy();
	}
	return nullptr;
}


extern "C" void nany_project_ref(nyproject_t* project)
{
	if (project)
		Nany::ref(project).addRef();
}


extern "C" void nany_project_unref(nyproject_t* ptr)
{
	if (ptr)
	{
		try
		{
			auto& project = Nany::ref(ptr);
			if (project.release())
				project.destroy();
		}
		catch (...) {}
	}
}


extern "C" nybool_t nany_project_add_source_from_file_n(nyproject_t* ptr, const char* filename, size_t len)
{
	if (ptr and filename and len != 0 and len < 32*1024)
	{
		try
		{
			AnyString path{filename, static_cast<uint32_t>(len)};
			Nany::ref(ptr).targets.anonym->addSourceFromFile(path);
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}


extern "C" nybool_t nany_project_add_source_from_file(nyproject_t* ptr, const char* filename)
{
	if (filename != nullptr)
	{
		size_t length = strlen(filename);
		return nany_project_add_source_from_file_n(ptr, filename, length);
	}
	return nyfalse;
}


extern "C" nybool_t nany_project_add_source_n(nyproject_t* ptr, const char* text, size_t len)
{
	if (ptr and text and len != 0 and len < 512 * 1024*1024) // arbitrary
	{
		try
		{
			AnyString source{text, static_cast<uint32_t>(len)};
			Nany::ref(ptr).targets.anonym->addSource("<unknown>", source);
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}


extern "C" nybool_t nany_project_add_source(nyproject_t* ptr, const char* text)
{
	if (text != nullptr)
	{
		size_t length = strlen(text);
		return nany_project_add_source_n(ptr, text, length);
	}
	return nyfalse;
}


extern "C" void nany_lock(const nyproject_t* ptr)
{
	if (ptr)
		Nany::ref(ptr).mutex.lock();
}


extern "C" void nany_unlock(const nyproject_t* ptr)
{
	if (ptr)
		Nany::ref(ptr).mutex.unlock();
}


extern "C" nybool_t nany_trylock(const nyproject_t* ptr)
{
	return ((ptr) ? Nany::ref(ptr).mutex.trylock() : false) ? nytrue : nyfalse;
}
