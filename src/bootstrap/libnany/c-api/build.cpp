#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/project.h"
#include "details/context/build.h"
#include "libnany-version.h"

using namespace Yuni;





extern "C" void nany_build_cf_init(nybuild_cf_t* cf, const nyproject_t* project)
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


extern "C" nybuild_t* nany_build_prepare(nyproject_t* ptr, const nybuild_cf_t* cf)
{
	if (ptr)
	{
		constexpr bool async = false;
		auto& project = Nany::ref(ptr);
		Nany::Build* build;
		try
		{
			if (cf)
			{
				if (cf->on_query and (nyfalse == cf->on_query(ptr)))
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
				nany_build_cf_init(&ncf, ptr);

				auto& allocator = const_cast<nyallocator_t&>(ncf.allocator);
				void* inplace = allocator.allocate(&allocator, sizeof(Nany::Build));
				if (!inplace)
					return nullptr;
				build = new (inplace) Nany::Build(project, ncf, async);
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

			return build->self();
		}
		catch (...)
		{
			build->destroy();
		}
	}
	return nullptr;
}


extern "C" nybool_t nany_build(nybuild_t* ptr)
{
	return ((ptr) ? Nany::ref(ptr).compile() : false) ? nytrue : nyfalse;
}




namespace // anonymous
{

	static void nany_build_print_compiler_info_to_console(Nany::Build& build)
	{
		// nanyc {c++/bootstrap} v0.1.0-alpha+ed25d59 {debug}
		{
			Nany::Logs::Message msg{Nany::Logs::Level::info};
			msg.section = "comp";
			msg.prefix = "nanyc {c++/bootstrap} ";
			msg.message << 'v' << LIBNANY_VERSION_STR;
			if (debugmode)
				msg.message << " {debug}";
			msg.print(build.cf.console, false);
		}
	}

} // anonymous namespace


extern "C" void nany_build_print_report_to_console(nybuild_t* ptr, nybool_t print_header)
{
	if (ptr)
	{
		auto& build = Nany::ref(ptr);

		try
		{
			if (YUNI_UNLIKELY(print_header != nyfalse))
				nany_build_print_compiler_info_to_console(build);

			if (!!build.messages)
				build.messages->print(build.cf.console, false);
		}
		catch (...) {}
	}
}


extern "C" void nany_build_ref(nybuild_t* build)
{
	if (build)
		Nany::ref(build).addRef();
}


extern "C" void nany_build_unref(nybuild_t* ptr)
{
	if (ptr)
	{
		try
		{
			auto& build = Nany::ref(ptr);
			if (build.release())
				build.destroy();
		}
		catch (...) {}
	}
}
