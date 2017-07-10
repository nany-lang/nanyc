#include "details/vm/machine.h"
#include "details/vm/thread.h"


namespace ny {
namespace vm {

Machine::Machine(const nyvm_opts_t& opts, const ny::Program& program)
	: opts(opts)
	, program(program) {
}

int Machine::run() {
	int exitstatus = -1;
	try {
		ny::vm::Thread thread(*this);
		uint32_t atomid = program.compdb->entrypoint.atomid;
		uint32_t instanceid = program.compdb->entrypoint.instanceid;
		auto r = thread.execute(atomid, instanceid);
		exitstatus = static_cast<int>(r);
	}
	catch (...) {
	}
	return exitstatus;
}

} // namespace vm
} // namespace ny
