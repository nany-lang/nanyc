#include <nanyc/vm.h>
#include "details/vm/vm.h"
#include "libnanyc.h"
#include <memory>
#include <cstring>


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
