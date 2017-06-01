#include "details/compiler/compile.h"
#include "details/compiler/compiler.h"
#include "details/program/program.h"
#include "details/reporting/report.h"
#include <libnanyc.h>
#include <utility>
#include <memory>

namespace ny {
namespace compiler {

namespace {

Logs::Report buildGenerateReport(void* ptr, Logs::Level level) {
	return (*((ny::Logs::Report*) ptr)).fromErrLevel(level);
}

nyprogram_t* complainNoSource(ny::Logs::Report& report) {
	report.error() << "no input source code";
	return nullptr;
}

} // namespace

inline Compiler::Compiler(const nycompile_opts_t& opts)
	: opts(opts) {
}

inline nyprogram_t* Compiler::compile() {
	ny::Logs::Report report{messages};
	Logs::Handler errorHandler{&report, &buildGenerateReport};
	if (unlikely(opts.sources.count == 0))
		return complainNoSource(report);
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
