#include "vm.h"


namespace ny {
namespace vm {

Machine::Machine(const nyvm_opts_t& opts, const ny::Program& program)
	: opts(opts)
	, program(program) {
	vmcx.opts = &this->opts;
	vmcx.internal = reinterpret_cast<void*>(this);
}

} // namespace vm
} // namespace ny
