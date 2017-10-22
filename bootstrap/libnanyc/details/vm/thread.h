#pragma once
#include "details/vm/machine.h"
#include "details/vm/io.h"
#include "libnanyc-config.h"

namespace ny {
namespace vm {

struct Thread final {
	Thread(Machine&);
	Thread(const Thread&) = delete;
	Thread(Thread&&) = delete;
	Thread& operator = (const Thread&) = delete;
	Thread& operator = (Thread&&) = delete;

	uint64_t execute(uint32_t atomid, uint32_t instanceid);

	nyvmthread_t capi;
	ny::vm::IO io;
	ny::vm::Machine& machine;
};

} // namespace vm
} // namespace ny
