#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/project.h"
#include "details/context/build.h"
#include "libnanyc-version.h"

using namespace Yuni;


namespace { // anonymous


void nybuild_print_compiler_info_to_console(ny::Build& build) {
	// nanyc {c++/bootstrap} v0.1.0-alpha+ed25d59 {debug}
	{
		ny::Logs::Message msg{ny::Logs::Level::info};
		msg.section = "comp";
		msg.prefix = "nanyc {c++/bootstrap} ";
		msg.message << 'v' << LIBNANYC_VERSION_STR;
		if (debugmode)
			msg.message << " {debug}";
		msg.print(build.cf.console, false);
	}
}


} // anonymous namespace


extern "C" void nybuild_print_report_to_console(nybuild_t* ptr, nybool_t print_header) {
	if (ptr) {
		auto& build = ny::ref(ptr);
		try {
			if (YUNI_UNLIKELY(print_header != nyfalse))
				nybuild_print_compiler_info_to_console(build);
			if (!!build.messages)
				build.messages->print(build.cf.console, false);
		}
		catch (...) {}
	}
}


extern "C" void nybuild_cf_init(nybuild_cf_t* cf, const nyproject_t* project) {
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nybuild_cf_t));
	if (project) {
		auto& prj = *(reinterpret_cast<const ny::Project*>(project));
		nany_memalloc_copy(&cf->allocator, &prj.cf.allocator);
	}
	else
		nany_memalloc_set_default(&(cf->allocator));
	nyconsole_cf_set_stdcout(&cf->console);
	// default entrypoint
	cf->entrypoint.size  = 4;
	cf->entrypoint.c_str = "main";
}


extern "C" nybuild_t* nybuild_prepare(nyproject_t* ptr, const nybuild_cf_t* cf) {
	if (ptr) {
		constexpr bool async = false;
		auto& project = ny::ref(ptr);
		ny::Build* build;
		try {
			if (cf) {
				if (cf->on_query and (nyfalse == cf->on_query(ptr)))
					return nullptr;
				auto& allocator = const_cast<nyallocator_t&>(cf->allocator);
				void* inplace = allocator.allocate(&allocator, sizeof(ny::Build));
				if (!inplace)
					return nullptr;
				build = new (inplace) ny::Build(project, *cf, async);
			}
			else {
				nybuild_cf_t ncf;
				nybuild_cf_init(&ncf, ptr);
				auto& allocator = const_cast<nyallocator_t&>(ncf.allocator);
				void* inplace = allocator.allocate(&allocator, sizeof(ny::Build));
				if (!inplace)
					return nullptr;
				build = new (inplace) ny::Build(project, ncf, async);
			}
		}
		catch (...) {
			return nullptr;
		}
		try {
			// making sure that user-events do not destroy the project by mistake
			build->addRef();
			// initialize the project after incrementing the ref count
			build->init();
			return build->self();
		}
		catch (...) {
			build->destroy();
		}
	}
	return nullptr;
}


extern "C" nybool_t nybuild(nybuild_t* ptr) {
	return ((ptr) ? ny::ref(ptr).compile() : false) ? nytrue : nyfalse;
}


extern "C" void nybuild_ref(nybuild_t* build) {
	if (build)
		ny::ref(build).addRef();
}


extern "C" void nybuild_unref(nybuild_t* ptr) {
	if (ptr) {
		try {
			auto& build = ny::ref(ptr);
			if (build.release())
				build.destroy();
		}
		catch (...) {}
	}
}
