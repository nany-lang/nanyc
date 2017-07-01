#pragma once
#include "details/vm/machine.h"
#include "details/ir/sequence.h"
#include "libnanyc-config.h"


namespace ny {
namespace vm {

struct Thread final {
	Thread(Machine&);
	Thread(const Thread&) = delete;
	Thread(Thread&&) = delete;
	Thread& operator = (const Thread&) = delete;
	Thread& operator = (Thread&&) = delete;

	void execute(const ny::ir::Sequence&);

	nyvmthread_t capi;
	ny::vm::Machine& machine;
};

} // namespace vm
} // namespace ny
