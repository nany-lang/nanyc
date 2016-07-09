#include "build.h"
#include "details/reporting/report.h"
#include "details/ast/tree-index.h"
#include <yuni/datetime/timestamp.h>
#include "details/ir/sequence.h"
#include "details/nsl/import-stdcore.h"
#include "details/atom/classdef-table-view.h"
#include "details/errors/errors.h"
#include "libnany-config.h"
#include "libnany-traces.h"

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
		targets.clear();
		sources.clear();
		sources.shrink_to_fit();
		sources.reserve(16); // arbitrary
		messages = nullptr;
		duration = 0;
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
			targets.insert(std::make_pair(pair.first, newtarget));
		}

		if (Config::importNSL)
		{
			importNSLCoreString(intrinsics);
			importNSLOSProcess(intrinsics);
			importNSLIONative(intrinsics);
			importNSLEnv(intrinsics);
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

			//
			// -- intermediate name lookup
			//
			// TODO remove this method
			// report.info() << "intermediate name resolution";
			bool successNameLookup = cdeftable.performNameLookup();
			if (unlikely(not successNameLookup and success))
				report.warning() << "name lookup failed";

			//
			// -- Core Objects (bool, u32, u64, f32, ...)
			//
			success &= successNameLookup
				and cdeftable.atoms.fetchAndIndexCoreObjects();

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
