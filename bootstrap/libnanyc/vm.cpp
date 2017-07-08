#include <nanyc/vm.h>
#include "details/vm/machine.h"
#include "libnanyc.h"
#include <memory>
#include <cstring>


extern "C" void nyvm_opts_init_defaults(nyvm_opts_t* opts) {
	if (opts) {
		opts->userdata = nullptr;
		nyallocator_init_from_malloc(&opts->allocator);
		nyconsole_init_from_stdout(&opts->cout);
		nyconsole_init_from_stderr(&opts->cerr);
	}
}

nybool_t nyvm_run_entrypoint(const nyvm_opts_t* opts, const nyprogram_t* program) {
	if (unlikely(!opts or !program))
		return nyfalse;
	try {
		auto* prgm = reinterpret_cast<const ny::Program*>(program);
		auto machine = std::make_unique<ny::vm::Machine>(*opts, *prgm);
		return nytrue;
	}
	catch (...) {
	}
	return nyfalse;
}
