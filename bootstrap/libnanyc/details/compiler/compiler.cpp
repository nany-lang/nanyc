#include "details/compiler/compile.h"
#include "details/compiler/compiler.h"
#include "details/program/program.h"
#include <libnanyc.h>
#include <utility>
#include <memory>

namespace ny {
namespace compiler {

inline Compiler::Compiler(const nycompile_opts_t& opts)
	: opts(opts) {
}

inline nyprogram_t* Compiler::compile() {
	auto program = std::make_unique<ny::Program>();
	return ny::Program::pointer(program.release());
}

nyprogram_t* compile(nycompile_opts_t& opts) {
	try {
		if (opts.on_build_start)
			opts.userdata = opts.on_build_start(opts.userdata);
		auto* program = Compiler{opts}.compile();
		if (opts.on_build_stop)
			opts.on_build_stop(opts.userdata, (program ? nytrue : nyfalse));
		return program;
	}
	catch (...) {
	}
	if (opts.on_build_stop)
		opts.on_build_stop(opts.userdata, nyfalse);
	return nullptr;
}

} // namespace compiler
} // namespace ny
