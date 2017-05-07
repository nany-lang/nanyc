#include "program.h"
#include "details/atom/atom.h"
#include "details/intrinsic/catalog.h"
#include "types.h"
#include "details/vm/context.h"

using namespace Yuni;


namespace ny {
namespace vm {
namespace {


void flushAll(nyconsole_t& console) {
	if (console.flush) {
		console.flush(console.internal, nycerr);
		console.flush(console.internal, nycout);
	}
}


} // anonymous namespace


Program::Program(const nyprogram_cf_t& cf, nybuild_t* build)
	: cf(cf)
	, build(build)
	, map(ref(build).cdeftable.atoms) {
	ref(build).addRef(); // nany_build_ref()
}


Program::~Program() {
	auto& b = ref(build); // nany_build_unref(&build);
	if (b.release())
		delete &b;
}


int Program::execute(uint32_t argc, const char** argv) {
	if (unlikely(cf.entrypoint.size == 0))
		return 0;
	if (cf.on_execute) {
		if (nyfalse == cf.on_execute(self()))
			return 1;
	}
	// TODO Take input arguments into consideration
	// (requires std.Array<:T:>)
	(void) argc;
	(void) argv;
	retvalue = EXIT_FAILURE;
	uint32_t atomid = ny::ref(build).main.atomid;
	uint32_t instanceid = ny::ref(build).main.instanceid;
	auto& ircode = map.ircode(atomid, instanceid);
	Context context{*this, AnyString{cf.entrypoint.c_str, cf.entrypoint.size}};
	bool success = context.initializeFirstTContext();
	if (unlikely(not success))
		return 666;
	uint64_t exitstatus;
	if (context.invoke(exitstatus, ircode, atomid, instanceid) != 0) {
		retvalue = static_cast<int>(exitstatus);
		if (cf.on_terminate)
			cf.on_terminate(self(), nytrue, retvalue);
		flushAll(cf.console);
		return retvalue;
	}
	if (cf.on_terminate)
		cf.on_terminate(self(), nyfalse, retvalue);
	flushAll(cf.console);
	return retvalue;
}


} // namespace vm
} // namespace ny
