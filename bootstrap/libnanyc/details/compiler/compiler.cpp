#include "details/compiler/compiler.h"
#include "details/compiler/compdb.h"
#include "details/program/program.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"
#include "details/pass/a-src2ast/ast-from-source.h"
#include "details/pass/b-ast-normalize/normalize.h"
#include "details/pass/c-ast2ir/source-ast-to-ir.h"
#include "details/pass/d-object-map/attach.h"
#include "details/semantic/atom-factory.h"
#include "details/intrinsic/std.core.h"
#include "libnanyc-config.h"
#include "libnanyc-traces.h"
#include "embed-nsl.hxx" // generated
#include <yuni/io/file.h>
#include <libnanyc.h>
#include <utility>
#include <memory>
#include <iostream>

namespace ny {
namespace compiler {

namespace {

constexpr uint32_t memoryHardlimit = 64 * 1024 * 1024;

Logs::Report buildGenerateReport(void* ptr, Logs::Level level) {
	return (*((ny::Logs::Report*) ptr)).fromErrLevel(level);
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

Atom& findEntrypointAtom(Atom& root, const AnyString& entrypoint) {
	Atom* atom = nullptr;
	root.eachChild(entrypoint, [&](Atom & child) -> bool {
		if (unlikely(atom != nullptr))
			throw "': multiple entry points found";
		atom = &child;
		return true;
	});
	if (unlikely(!atom))
		throw "()': function not found";
	if (unlikely(not atom->isFunction() or atom->isClassMember()))
		throw "': the atom is not a function";
	return *atom;
}

bool instanciate(ny::compiler::Compdb& compdb, ny::Logs::Report& report, AnyString entrypoint) {
	using ParameterList = decltype(ny::semantic::FuncOverloadMatch::result.params);
	try {
		auto& atom = findEntrypointAtom(compdb.cdeftable.atoms.root, entrypoint);
		ParameterList params;
		ParameterList tmplparams;
		ClassdefTableView cdeftblView{compdb.cdeftable};
		ny::semantic::Settings settings(atom, cdeftblView, compdb, params, tmplparams);
		bool instanciated = ny::semantic::instanciateAtom(settings);
		report.appendEntry(settings.report);
		if (config::traces::atomTable)
			compdb.cdeftable.atoms.root.printTree(compdb.cdeftable);
		if (likely(instanciated)) {
			compdb.entrypoint.atomid = atom.atomid;
			compdb.entrypoint.instanceid = settings.instanceid;
			return true;
		}
	}
	catch (const char* e) {
		report.error() << "failed to instanciate '" << entrypoint << e;
	}
	compdb.entrypoint.atomid = (uint32_t) -1;
	compdb.entrypoint.instanceid = (uint32_t) -1;
	return false;
}

std::unique_ptr<ny::Program> compile(ny::compiler::Compdb& compdb) {
	ny::Logs::Report report{compdb.messages};
	Logs::Handler errorHandler{&report, &buildGenerateReport};
	try {
		auto& opts = compdb.opts;
		uint32_t scount = opts.sources.count;
		if (unlikely(scount == 0))
			throw "no input source code";
		if (config::importNSL)
			ny::intrinsic::import::all(compdb.intrinsics);
		if (config::importNSL)
			scount += corefilesCount;
		if (unlikely(opts.with_nsl_unittests == nytrue))
			scount += unittestCount;
		auto& sources = compdb.sources;
		sources.count = scount;
		sources.items = std::make_unique<Source[]>(scount);
		bool compiled = true;
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
			and compdb.cdeftable.atoms.fetchAndIndexCoreObjects() // indexing bool, u32, f64...
			and ny::semantic::resolveStrictParameterTypes(compdb, compdb.cdeftable.atoms.root); // typedef
		if (config::traces::preAtomTable)
			compdb.cdeftable.atoms.root.printTree(ClassdefTableView{compdb.cdeftable});
		if (unlikely(not compiled))
			return nullptr;
		auto& entrypoint = compdb.opts.entrypoint;
		if (unlikely(entrypoint.len == 0))
			return nullptr;
		bool epinst = instanciate(compdb, report, AnyString(entrypoint.c_str, static_cast<uint32_t>(entrypoint.len)));
		if (unlikely(not epinst))
			return nullptr;
		return std::make_unique<ny::Program>();
	}
	catch (const std::bad_alloc&) {
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
		auto compdb = std::make_unique<Compdb>(opts);
		auto program = compile(*compdb);
		if (opts.on_build_stop)
			opts.on_build_stop(opts.userdata, (program ? nytrue : nyfalse));
		if (opts.on_report and not compdb->messages.entries.empty())
			opts.on_report(opts.userdata, reinterpret_cast<const nyreport_t*>(&compdb->messages));
		if (unlikely(!program))
			return nullptr;
		program->compdb = std::move(compdb);
		return ny::Program::pointer(program.release());
	}
	catch (...) {
	}
	if (opts.on_build_stop)
		opts.on_build_stop(opts.userdata, nyfalse);
	return nullptr;
}

} // namespace compiler
} // namespace ny
