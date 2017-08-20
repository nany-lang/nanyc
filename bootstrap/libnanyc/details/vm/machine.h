#pragma once
#include <nanyc/vm.h>
#include "details/program/program.h"

namespace ny {
namespace vm {

struct Machine final {
	Machine(const nyvm_opts_t&, const ny::Program&);
	Machine(const Machine&) = delete;
	Machine(Machine&&) = delete;
	Machine& operator = (const Machine&) = delete;
	Machine& operator = (Machine&&) = delete;

	int run();
	void cout(const AnyString&);
	void cerr(const AnyString&);
	void cerrexception(const AnyString&);

	nyvm_opts_t opts;
	const ny::Program& program;
};

} // namespace vm
} // namespace ny
