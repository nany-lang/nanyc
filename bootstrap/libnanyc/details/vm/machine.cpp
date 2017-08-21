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

void Machine::cout(const AnyString& string) {
	opts.cout.write(&opts.cout, string.c_str(), string.size());
}

void Machine::cerr(const AnyString& string) {
	opts.cerr.write(&opts.cerr, string.c_str(), string.size());
}

void Machine::cerrexception(const AnyString& string) {
	opts.cerr.write(&opts.cerr, "\n\n", 2);
	{
		constexpr const char prefix[] = "nanyc vm exception:";
		opts.cerr.set_bkcolor(&opts.cerr, nyc_default);
		opts.cerr.set_color(&opts.cerr, nyc_red);
		opts.cerr.write(&opts.cerr, prefix, strlen(prefix));
		opts.cerr.set_color(&opts.cerr, nyc_default);
	}
	opts.cerr.write(&opts.cerr, " ", 1);
	opts.cerr.write(&opts.cerr, string.c_str(), string.size());
	opts.cerr.write(&opts.cerr, "\n", 1);
	opts.cerr.flush(&opts.cerr);
}

} // namespace vm
} // namespace ny
