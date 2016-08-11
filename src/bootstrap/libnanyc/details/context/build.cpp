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




namespace Nany
{

	Build::AttachedSequenceRef::~AttachedSequenceRef()
	{
		if (owned)
			delete sequence;
	}


	void Build::destroy()
	{
		if (cf.on_destroy)
			cf.on_destroy(self(), project.self());

		this->~Build();

		auto& allocator = const_cast<nyallocator_t&>(cf.allocator);
		allocator.deallocate(&allocator, this, sizeof(Nany::Build));
	}


	void Build::init()
	{
		if (not targets.empty()) // big cleanup
		{
			targets.clear();
			targets.shrink_to_fit();
			sources.clear();
			sources.shrink_to_fit();
			pAttachedSequences.clear();
			pAttachedSequences.shrink_to_fit();
			duration = 0;
			buildtime = 0;
			messages = nullptr;
		}

		targets.reserve(project.targets.all.size());
		sources.reserve(32); // arbitrary, at least more than 20 source files from nsl
		success = true;

		// keeping our own list of targets / sources to be completely
		// isolated from the project
		messages = std::make_unique<Logs::Message>(Logs::Level::none);
		for (auto& pair: project.targets.all)
		{
			CTarget::Ptr newtarget = new CTarget{project.self(), *pair.second};
			newtarget->eachSource([&](Source& source) {
				sources.push_back(std::ref(source));
			});
			targets.emplace_back(newtarget);
		}

		if (Config::importNSL)
		{
			importNSLCoreString(intrinsics);
			importNSLOSProcess(intrinsics);
			importNSLEnv(intrinsics);
			importNSLIO(intrinsics);
			importNSLMemory(intrinsics);
			importNSLConsole(intrinsics);
		}

		if (cf.on_create)
			cf.on_create(self(), project.self());
	}


	static Logs::Report buildGenerateReport(void* ptr, Logs::Level level)
	{
		return (*((Nany::Logs::Report*) ptr)).fromErrLevel(level);
	}


	bool Build::compile()
	{
		// preparing report
		Nany::Logs::Report report{*messages.get()};
		Logs::Handler newHandler{&report, &buildGenerateReport};


		buildtime = DateTime::NowMilliSeconds();

		if (unlikely(sources.empty()))
		{
			report.error() << "no target to build";
		}
		else
		{
			// initialization of some global data
			Nany::Sema::Metadata::initialize(); // TODO remove those methods
			Nany::ASTHelper::initialize();

			success = true;

			// build each source
			for (auto& src: sources)
				success &= src.get().build(*this);

			// Indexing Core Objects (bool, u32, u64, f32, ...)
			success = success and cdeftable.atoms.fetchAndIndexCoreObjects();

			// Resolve strict parameter types
			// ex: func foo(p1, p2: TypeP2) // Here TypeP2 must be resolved
			// This will be used for deduce func overloading
			success = success and resolveStrictParameterTypes(cdeftable.atoms.root);


			if (Config::Traces::preAtomTable)
				cdeftable.atoms.root.printTree(ClassdefTableView{cdeftable});

			//
			// -- instanciate
			//
			const nytype_t* argtypes = nullptr;
			success = success and instanciate("main", argtypes, main.atomid, main.instanceid);

			// end
			duration += DateTime::NowMilliSeconds() - buildtime;

			if (debugmode and unlikely(not success))
				report.error() << "debug: failed to compile";

			return success;
		}

		success = false;
		duration += DateTime::NowMilliSeconds() - buildtime;
		return false;
	}




} // namespace Nany
