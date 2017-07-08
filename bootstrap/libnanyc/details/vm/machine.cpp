#include "details/vm/machine.h"


namespace ny {
namespace vm {

Machine::Machine(const nyvm_opts_t& opts, const ny::Program& program)
	: opts(opts)
	, program(program) {
}

} // namespace vm
} // namespace ny
