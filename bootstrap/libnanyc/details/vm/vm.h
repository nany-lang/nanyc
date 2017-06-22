#pragma once
#include <nanyc/vm.h>
#include "details/program/program.h"


namespace ny {
namespace vm {

struct Machine final {
	Machine(const nyvm_opts_t&, const ny::Program*);
	Machine(const Machine&) = delete;
	Machine(Machine&&) = delete;
	Machine& operator = (const Machine&) = delete;
	Machine& operator = (Machine&&) = delete;

	nyvm_opts_t opts;
	const ny::Program* program;
	nyvm_t vmcx;
};

} // namespace vm
} // namespace ny
