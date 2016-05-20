#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/vm/program.h"

using namespace Yuni;





extern "C" void nany_program_cf_init(nyprogram_cf_t* cf, const nybuild_cf_t* buildcf)
{
	assert(cf != NULL);
	memset(cf, 0x0, sizeof(nyprogram_cf_t));

	if (nullptr == buildcf)
	{
		nany_memalloc_set_default(&(cf->allocator));
		nany_console_cf_set_stdcout(&cf->console);
	}
	else
	{
		nany_memalloc_copy(&(cf->allocator), &(buildcf->allocator));
		nany_console_cf_copy(&(cf->console), &(buildcf->console));
	}
}


extern "C" nyprogram_t* nany_program_prepare(nybuild_t* build, const nyprogram_cf_t* cf)
{
	if (build)
	{
		Nany::VM::Program* program;
		if (cf)
		{
			auto& allocator = const_cast<nyallocator_t&>(cf->allocator);
			void* inplace = allocator.allocate(&allocator, sizeof(Nany::VM::Program));
			if (unlikely(!inplace))
				return nullptr;

			program = new (inplace) Nany::VM::Program(*cf, build);
		}
		else
		{
			nyprogram_cf_t ncf;
			nany_program_cf_init(&ncf, nullptr);

			auto& allocator = const_cast<nyallocator_t&>(ncf.allocator);
			void* inplace = allocator.allocate(&allocator, sizeof(Nany::VM::Program));
			if (unlikely(!inplace))
				return nullptr;

			program = new (inplace) Nany::VM::Program(ncf, build);
		}

		program->addRef();
		return program->self();
	}
	return nullptr;
}


extern "C" int nany_main(nyprogram_t* ptr, int argc, const char** argv)
{
	if (ptr and argc > 0 and argv != nullptr)
	{
		auto& program = Nany::ref(ptr);
		return program.execute(argc, argv);
	}
	return 1;
}


extern "C" void nany_program_ref(nyprogram_t* program)
{
	if (program)
		Nany::ref(program).addRef();
}


extern "C" void nany_program_unref(nyprogram_t* ptr)
{
	if (ptr)
	{
		auto& program = Nany::ref(ptr);
		if (program.release())
			program.destroy();
	}
}
