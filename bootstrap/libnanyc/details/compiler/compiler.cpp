#include "details/compiler/compiler.h"
#include "details/compiler/compdb.h"
#include "details/program/program.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"
#include "details/pass/a-src2ast/ast-from-source.h"
#include "details/pass/b-ast-normalize/normalize.h"
#include "details/pass/c-ast2ir/source-ast-to-ir.h"
#include "details/pass/d-object-map/attach.h"
#include "details/vm/runtime/std.core.h"
#include "libnanyc-config.h"
#include "embed-nsl.hxx" // generated
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
		yuni::IO::Canonicalize(source.storageFilename, AnyString{opts.filename.c_str, static_cast<uint32_t>(opts.filename.len)});
		source.filename = source.storageFilename;
	}
	if (opts.content.len != 0) {
		if (unlikely(opts.content.len > memoryHardlimit))
			throw "input source content bigger than internal limit";
		source.storageContent.assign(opts.content.c_str, static_cast<uint32_t>(opts.content.len));
		source.content = source.storageContent;
	}
}

void importcompdbIntrinsics(intrinsic::Catalog& intrinsics) {
	nsl::import::string(intrinsics);
	nsl::import::process(intrinsics);
	nsl::import::env(intrinsics);
	nsl::import::io(intrinsics);
	nsl::import::memory(intrinsics);
	nsl::import::console(intrinsics);
	nsl::import::digest(intrinsics);
}

bool compileSource(ny::Logs::Report& mainreport, ny::compiler::Compdb& compdb, ny::compiler::Source& source, const nycompile_opts_t& gopts) {
	auto report = mainreport.subgroup();
	report.data().origins.location.filename = source.filename;
	report.data().origins.location.target.clear();
	bool compiled = true;
	compiled &= makeASTFromSource(source);
	compiled &= passDuplicateAndNormalizeAST(source, report);
	compiled &= passTransformASTToIR(source, report, gopts);
	compiled  = compiled and attach(compdb, source);
	return compiled;
}

bool importSourceAndCompile(ny::Logs::Report& mainreport, ny::compiler::Compdb& compdb, ny::compiler::Source& source, const nycompile_opts_t& gopts, const nysource_opts_t& opts) {
	copySourceOpts(source, opts);
	return compileSource(mainreport, compdb, source, gopts);
}

nyprogram_t* compile(ny::compiler::Compdb& compdb) {
	ny::Logs::Report report{compdb.messages};
	Logs::Handler errorHandler{&report, &buildGenerateReport};
	try {
		auto& opts = compdb.opts;
		uint32_t scount = opts.sources.count;
		if (unlikely(scount == 0))
			return complainNoSource(report);
		if (config::importNSL)
			importcompdbIntrinsics(compdb.intrinsics);
		if (config::importNSL)
			scount += corefilesCount;
		if (unlikely(opts.with_nsl_unittests == nytrue))
			scount += unittestCount;
		auto& sources = compdb.sources;
		sources.count = scount;
		sources.items = std::make_unique<Source[]>(scount);
		bool compiled = opts.with_nsl_unittests == nyfalse;
		uint32_t offset = 0;
		if (config::importNSL) {
			registerNSLCoreFiles(sources, offset, [&](ny::compiler::Source& source) {
				compiled &= compileSource(report, compdb, source, opts);
			});
		}
		if (opts.with_nsl_unittests == nytrue) {
			registerUnittestFiles(sources, offset, [&](ny::compiler::Source& source) {
				compiled &= compileSource(report, compdb, source, opts);
			});
		}
		for (uint32_t i = 0; i != opts.sources.count; ++i) {
			auto& source = sources[offset + i];
			compiled &= importSourceAndCompile(report, compdb, source, opts, opts.sources.items[i]);
		}
		compiled = compiled
			and compdb.cdeftable.atoms.fetchAndIndexCoreObjects(); // indexing bool, u32, f64...
		if (unlikely(not compiled))
			return nullptr;
		auto program = std::make_unique<ny::Program>();
		return ny::Program::pointer(program.release());
	}
	catch (const std::bad_alloc& e) {
		report.ice() << "not enough memory when compiling";
	}
	catch (const std::exception& e) {
		report.ice() << "exception: " << e.what();
	}
	catch (const char* e) {
		report.error() << e;
	}
	catch (...) {
		report.ice() << "uncaught exception when compiling";
	}
	return nullptr;
}

} // namespace

nyprogram_t* compile(nycompile_opts_t& opts) {
	try {
		if (opts.on_build_start)
			opts.userdata = opts.on_build_start(opts.userdata);
		ny::compiler::Compdb compdb{opts};
		auto* program = compile(compdb);
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
