#include "details/compiler/compile.h"
#include "details/compiler/compiler.h"
#include "details/program/program.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"
#include "details/pass/a-src2ast/ast-from-source.h"
#include <yuni/io/file.h>
#include <libnanyc.h>
#include <utility>
#include <memory>

namespace ny {
namespace compiler {

namespace {

constexpr uint32_t memoryHardlimit = 64 * 1024 * 1024;

Logs::Report buildGenerateReport(void* ptr, Logs::Level level) {
	return (*((ny::Logs::Report*) ptr)).fromErrLevel(level);
}

nyprogram_t* complainNoSource(ny::Logs::Report& report) {
	report.error() << "no input source code";
	return nullptr;
}

void copySourceOpts(ny::compiler::Source& source, const nysource_opts_t& opts) {
	if (opts.filename.len != 0) {
		if (unlikely(opts.filename.len > memoryHardlimit))
			throw "input filename bigger than internal limit";
		yuni::IO::Canonicalize(source.filename, AnyString{opts.filename.c_str, static_cast<uint32_t>(opts.filename.len)});
	}
	if (opts.content.len != 0) {
		if (unlikely(opts.content.len > memoryHardlimit))
			throw "input source content bigger than internal limit";
		source.content.assign(opts.content.c_str, static_cast<uint32_t>(opts.content.len));
	}
}

void compileSource(ny::compiler::Source& source) {
}

void importSourceAndCompile(ny::compiler::Source& source, const nysource_opts_t& opts) {
	copySourceOpts(source, opts);
	compileSource(source);
}

} // namespace

inline Compiler::Compiler(const nycompile_opts_t& opts)
	: opts(opts) {
}

inline nyprogram_t* Compiler::compile() {
	ny::Logs::Report report{messages};
	Logs::Handler errorHandler{&report, &buildGenerateReport};
	uint32_t scount = opts.sources.count;
	if (unlikely(scount == 0))
		return complainNoSource(report);
	sources.count = scount;
	sources.items = std::make_unique<Source[]>(scount);
	for (uint32_t i = 0; i != opts.sources.count; ++i)
		importSourceAndCompile(sources[i], opts.sources.items[i]);
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
