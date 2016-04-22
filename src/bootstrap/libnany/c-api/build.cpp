#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/project.h"
#include "details/context/build.h"

using namespace Yuni;





extern "C" void nany_build_cf_reset(nybuild_cf_t* cf, const nyproject_t* project)
{
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nybuild_cf_t));
	if (project)
	{
		auto& prj = *(reinterpret_cast<const Nany::Project*>(project));
		nany_memalloc_copy(&cf->allocator, &prj.cf.allocator);
	}
	else
		nany_memalloc_set_default(&(cf->allocator));

	nany_console_cf_set_stdcout(&cf->console);
}



namespace // anonymous
{

	static inline nybuild_t* create_nany_build(nyproject_t* ptr, const nybuild_cf_t* cf, bool async)
	{
		if (ptr)
		{
			auto& project = Nany::ref(ptr);
			Nany::Build* build;
			try
			{
				if (cf)
				{
					if (cf->on.query and (nyfalse == cf->on.query(ptr)))
						return nullptr;

					auto& allocator = const_cast<nyallocator_t&>(cf->allocator);
					void* inplace = allocator.allocate(&allocator, sizeof(Nany::Build));
					if (!inplace)
						return nullptr;
					build = new (inplace) Nany::Build(project, *cf, async);
				}
				else
				{
					nybuild_cf_t ncf;
					nany_build_cf_reset(&ncf, ptr);

					auto& allocator = const_cast<nyallocator_t&>(cf->allocator);
					void* inplace = allocator.allocate(&allocator, sizeof(Nany::Build));
					if (!inplace)
						return nullptr;
					build = new (inplace) Nany::Build(project, *cf, async);
				}
			}
			catch (...)
			{
				return nullptr;
			}

			try
			{
				// making sure that user-events do not destroy the project by mistake
				build->addRef();
				// initialize the project after incrementing the ref count
				build->init();
				// make a copy of all targets to be independant from the project
				build->prepare();

				return build->self();
			}
			catch (...)
			{
				build->destroy();
			}
		}
		return nullptr;
	}


} // anonymous namespace




extern "C" nybuild_t* nany_build_prepare(nyproject_t* ptr, const nybuild_cf_t* cf)
{
	return create_nany_build(ptr, cf, false);
}


extern "C" nybool_t nany_build(nybuild_t* ptr)
{
	return ((ptr) ? Nany::ref(ptr).compile() : false) ? nytrue : nyfalse;
}


extern "C" nybool_t nany_build_atom(nybuild_t* ptr, const char* atom, size_t atom_len, const nytype_t* args)
{
	if (ptr and atom and *atom != '\0' and atom_len > 0 and atom_len < 1024)
	{
		AnyString atomname{atom, static_cast<uint32_t>(atom_len)};
		Nany::ref(ptr).instanciate(atomname, args);
	}
	return nyfalse;
}


extern "C" void nany_build_print_report_to_console(nybuild_t* ptr, nybool_t unify)
{
	if (ptr)
	{
		auto& build = Nany::ref(ptr);
		if (!!build.messages)
			build.messages->print(build.cf.console, (unify != nyfalse));
	}
}


extern "C" void nany_build_ref(nybuild_t* build)
{
	if (build)
		Nany::ref(build).addRef();
}


extern "C" void nany_build_unref(nybuild_t** ptr)
{
	if (ptr and *ptr)
	{
		auto& build = Nany::ref(*ptr);
		if (build.release())
			build.destroy();
		*ptr = nullptr;
	}
}
