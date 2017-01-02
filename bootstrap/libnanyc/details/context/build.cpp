#include "build.h"
#include "details/reporting/report.h"
#include "details/ast/tree-index.h"
#include <yuni/datetime/timestamp.h>
#include "details/ir/sequence.h"
#include "details/vm/runtime/std.core.h"
#include "details/atom/classdef-table-view.h"
#include "details/errors/errors.h"
#include "libnanyc-config.h"
#include "libnanyc-traces.h"

using namespace Yuni;


namespace ny {
namespace {

Logs::Report buildGenerateReport(void* ptr, Logs::Level level) {
	return (*((ny::Logs::Report*) ptr)).fromErrLevel(level);
}


} // namespace


Build::AttachedSequenceRef::~AttachedSequenceRef() {
	if (owned)
		delete sequence;
}


Build::Build(Ref<Project> project, const nybuild_cf_t& cf, bool async)
	: cf(cf)
	, project(project)
	, isAsync(async) {
	if (not m_targets.empty()) { // big cleanup
		m_targets.clear();
		m_targets.shrink_to_fit();
		m_sources.clear();
		m_sources.shrink_to_fit();
		m_attachedSequences.clear();
		m_attachedSequences.shrink_to_fit();
		duration = 0;
		buildtime = 0;
		messages = nullptr;
	}
	m_targets.reserve(project->targets.all.size());
	m_sources.reserve(32); // arbitrary, at least more than 20 source files from nsl
	success = true;
	// keeping our own list of targets / sources to be completely
	// isolated from the project
	messages = std::make_unique<Logs::Message>(Logs::Level::none);
	for (auto& pair : project->targets.all) {
		auto newtarget = make_ref<CTarget>(project->self(), *pair.second);
		newtarget->eachSource([&](Source & source) {
			m_sources.push_back(std::ref(source));
		});
		m_targets.emplace_back(newtarget);
	}
	if (config::importNSL) {
		nsl::import::string(intrinsics);
		nsl::import::process(intrinsics);
		nsl::import::env(intrinsics);
		nsl::import::io(intrinsics);
		nsl::import::memory(intrinsics);
		nsl::import::console(intrinsics);
		nsl::import::digest(intrinsics);
	}
}


Build::~Build() {
	// clear internal containers before releasing the project itself
	m_attachedSequences.clear();
	m_sources.clear();
	m_targets.clear();
}


bool Build::compile() {
	ny::Logs::Report report{*messages.get()};
	Logs::Handler newHandler{&report, &buildGenerateReport};
	success = true;
	buildtime = DateTime::NowMilliSeconds();
	if (not m_sources.empty()) {
		for (auto& src : m_sources)
			success &= src.get().build(*this);
		if (cf.ignore_atoms == nyfalse) {
			// Indexing Core Objects (bool, u32, u64, f32, ...)
			success = success and cdeftable.atoms.fetchAndIndexCoreObjects();
			// Resolve strict parameter types
			// ex: func foo(p1, p2: TypeP2) // Here TypeP2 must be resolved
			// This will be used for deduce func overloading
			success = success and resolveStrictParameterTypes(cdeftable.atoms.root);
			if (config::traces::preAtomTable)
				cdeftable.atoms.root.printTree(ClassdefTableView{cdeftable});
			const nytype_t* argtypes = nullptr;
			AnyString entrypoint{cf.entrypoint.c_str, cf.entrypoint.size};
			if (not entrypoint.empty())
				success = success and instanciate(entrypoint, argtypes, main.atomid, main.instanceid);
		}
	}
	else {
		report.error() << "no target to build";
		success = false;
	}
	duration += DateTime::NowMilliSeconds() - buildtime;
	if (debugmode and unlikely(not success))
		report.error() << "debug: failed to compile";
	return success;
}


} // namespace ny
