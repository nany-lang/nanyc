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
#include "details/compiler/report.h"
#include "libnanyc-config.h"
#include "libnanyc-traces.h"
#include "libnanyc-version.h"
#include "embed-nsl.hxx" // generated
#include <yuni/io/file.h>
#include <libnanyc.h>
#include <utility>
#include <memory>
#include <unordered_map>

namespace ny {
namespace compiler {

namespace {

constexpr uint32_t memoryHardlimit = 64 * 1024 * 1024;

void bugReportInfo(ny::Logs::Report& report) {
	auto e = report.info("nanyc {c++/bootstrap}") << " v" << LIBNANYC_VERSION_STR;
	if (yuni::debugmode)
		e << " {debug}";
	e.message.section = "comp";
}

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
		atom.funcinfo.raisedErrors.noleaks.enabled = true;
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
			return not report.hasErrors();
		}
	}
	catch (const char* e) {
		report.error() << "failed to instanciate '" << entrypoint << e;
	}
	compdb.entrypoint.atomid = (uint32_t) -1;
	compdb.entrypoint.instanceid = (uint32_t) -1;
	return false;
}

struct CompilerQueue final {
	ny::compiler::Compdb& compdb;
	ny::Logs::Report report;
	std::vector<yuni::String> collectionSearchPaths;
	std::unordered_set<yuni::String> collectionsLoaded;

	CompilerQueue(ny::compiler::Compdb& compdb)
		: compdb(compdb)
		, report(compdb.messages) {
		collectionSearchPaths.reserve(4);
		collectionSearchPaths.emplace_back(ny::config::collectionSystemPath);
	}

	static void usesCollection(void* userdata, const AnyString& name) {
		auto& queue = *(reinterpret_cast<CompilerQueue*>(userdata));
		if (queue.collectionsLoaded.count(name) != 0)
			return;
		queue.collectionsLoaded.insert(name);
		yuni::String filename;
		yuni::String content;
		for (auto& searchpath: queue.collectionSearchPaths) {
			filename = searchpath;
			filename << '/' << name << "/nanyc.collection";
			if (yuni::IO::errNone != yuni::IO::File::LoadFromFile(content, filename))
				continue;
			if (unlikely(queue.compdb.opts.verbose == nytrue))
				info() << "uses " << name << " -> " << searchpath << '/' << name;
			content.words("\n", [&](AnyString line) -> bool {
				if (not line.empty()) {
					if (unlikely(line[0] == 'u' and line.startsWith("uses "))) {
						line.consume(5);
						if (not line.empty())
							usesCollection(userdata, line);
					}
					else {
						queue.compdb.sources.emplace_back();
						auto& source = queue.compdb.sources.back();
						source.storageFilename = searchpath;
						source.storageFilename << '/' << name << '/' << line;
						source.filename = source.storageFilename;
					}
				}
				return true;
			});
			return;
		}
		auto err = (error() << "collection '" << name << "' not found");
		for (auto& searchpath: queue.collectionSearchPaths)
			err.hint() << "from path '" << searchpath << "'";
	}

	bool compileSource(ny::compiler::Source& source) {
		if (unlikely(compdb.opts.verbose == nytrue))
			info() << "compile " << source.filename;
		auto subreport = report.subgroup();
		subreport.data().origins.location.filename = source.filename;
		subreport.data().origins.location.target.clear();
		bool compiled = true;
		compiled &= makeASTFromSource(source);
		compiled &= passDuplicateAndNormalizeAST(source, subreport, &usesCollection, this);
		compiled &= passTransformASTToIR(source, subreport, compdb.opts);
		compiled  = compiled and attach(compdb, source);
		return compiled;
	}

	bool importSourceAndCompile(ny::compiler::Source& source, const nysource_opts_t& opts) {
		copySourceOpts(source, opts);
		return compileSource(source);
	}
};

std::unique_ptr<ny::Program> compile(ny::compiler::Compdb& compdb) {
	CompilerQueue queue(compdb);
	auto& report = queue.report;
	Logs::Handler errorHandler{&report, &buildGenerateReport};
	try {
		if (unlikely(compdb.opts.verbose == nytrue))
			bugReportInfo(report);
		uint32_t scount = compdb.opts.sources.count;
		if (unlikely(scount == 0))
			throw "no input source code";
		if (config::importNSL)
			ny::intrinsic::import::all(compdb.intrinsics);
		if (config::importNSL)
			scount += corefilesCount;
		auto& sources = compdb.sources;
		sources.resize(scount);
		bool compiled = true;
		uint32_t offset = 0;
		if (config::importNSL) {
			registerNSLCoreFiles(sources, offset, [&](ny::compiler::Source& source) {
				compiled &= queue.compileSource(source);
			});
		}
		if (unlikely(compdb.opts.with_nsl_unittests == nytrue))
			queue.usesCollection(&queue, "nsl.selftest");
		for (uint32_t i = 0; i != compdb.opts.sources.count; ++i) {
			auto& source = sources[offset + i];
			compiled &= queue.importSourceAndCompile(source, compdb.opts.sources.items[i]);
		}
		for (uint32_t i = offset + compdb.opts.sources.count; i < sources.size(); ++i) {
			auto& source = sources[i];
			compiled &= queue.compileSource(source);
		}
		if (unlikely(compdb.opts.verbose == nytrue))
			report.info() << "building... ";
		compiled = compiled
			and likely(compdb.opts.on_unittest == nullptr)
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
		if (config::traces::raisedErrorSummary)
			ny::compiler::report::raisedErrorsForAllAtoms(compdb, report);
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

void pleaseReport(nycompile_opts_t& opts, std::unique_ptr<ny::compiler::Compdb>& compdb) {
	auto* rp = reinterpret_cast<const nyreport_t*>(&compdb->messages);
	if (opts.on_report)
		opts.on_report(opts.userdata, rp);
	else
		nyreport_print_stdout(rp);
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
		if (not compdb->messages.entries.empty())
			pleaseReport(opts, compdb);
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
