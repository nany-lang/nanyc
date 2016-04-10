#include <yuni/yuni.h>
#include <yuni/datetime/timestamp.h>
#include <yuni/job/taskgroup.h>
#include "details/context/context.h"
#include "details/fwd.h"
#include "details/context/build-info.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"
#include "libnany-traces.h"
#include <memory>

using namespace Yuni;





namespace Nany
{

	namespace // anonymous
	{

		static void printDebugInfoClassdefAtomTable(Logs::Report& report, ClassdefTable& table)
		{
			report.info(); // for beauty
			report.info();

			if (Config::Traces::printPreAtomTable)
			{
				auto trace = report.trace();
				trace.message.prefix = "ATOMS";
				table.atoms.root.print(report, ClassdefTableView{table});
				report.info(); // for beauty
				report.info(); // for beauty
			}
			if (Config::Traces::printAllTypeDefinitions)
			{
				auto trace = (report.trace() << "CLASSDEF PER ATOM\n");
				ClassdefTableView{table}.print(trace.data().message, false);
				report.info(); // for beauty
				report.info(); // for beauty
			}
		}


		static void printAtomTable(Logs::Report& report, ClassdefTable& table)
		{
			auto trace = report.trace();
			trace.message.prefix = "ATOMS";
			table.atoms.root.print(report, ClassdefTableView{table});
			report.info(); // for beauty
			report.info(); // for beauty
		}

	} // anonymous namespace




	inline std::unique_ptr<BuildInfoContext> Context::doBuildWL(Logs::Report report, int64_t& duration)
	{
		//
		// -- initialization & source fetching
		//
		pBuildInfo.reset(nullptr); // release some memory
		auto buildinfoptr = std::make_unique<BuildInfoContext>(usercontext);
		auto& buildinfo = *(buildinfoptr.get());
		// the result will succeed by default, and will be reverted to false as soon as an error occurs
		buildinfo.success = true;

		// ask to the default target to provide its list of sources
		if (!(!pDefaultTarget))
			pDefaultTarget->fetchSourceList(buildinfo);
		// ask to all targets to provide its list of sources
		for (auto& pair: pTargets)
			pair.second->fetchSourceList(buildinfo);

		// ask to all sources to build themselves (create jobs)
		if (unlikely(buildinfo.sources.empty()))
		{
			report.error() << "no input file";
			return nullptr;
		}

		// initialization of some global data
		Nany::Sema::Metadata::initialize(); // TODO remove those methods
		Nany::ASTHelper::initialize();


		//
		// -- preparing the queue service for multitasking (if available)
		//
		auto* qs = usercontext.mt.queueservice;
		bool isMultithreaded = (qs != nullptr);
		auto* queueservice = reinterpret_cast<Job::QueueService*>(qs);
		bool qsWasStarted = false;
		if (isMultithreaded)
		{
			queueservice->addRef();
			qsWasStarted = queueservice->started();
			if (not qsWasStarted)
				queueservice->start();
		}


		//
		// -- parse and extract information for all input source files
		// -- transform input source files into AST
		// -- transform AST into IR (opcodes for both typing and )
		//
		if (isMultithreaded)
		{
			Job::Taskgroup task{*queueservice, false}; // prepare all tasks
			for (auto& ptr: buildinfo.sources)
				ptr->build(buildinfo, task, report);

			buildinfo.buildtime = DateTime::NowMilliSeconds();

			// launch all jobs and wait for them
			task.start();
			task.wait();
		}
		else
		{
			buildinfo.buildtime = DateTime::NowMilliSeconds();
			for (auto& ptr: buildinfo.sources)
				ptr->build(buildinfo, report);
		}

		auto& cdeftable = buildinfo.isolate.classdefTable;

		// debug
		if (Config::Traces::printPreAtomTable or Config::Traces::printClassdefTable)
			printDebugInfoClassdefAtomTable(report, cdeftable);
		if (Config::Traces::printSourceOpcodeSequence)
			buildinfo.isolate.print(report);

		// instanciate the whole code (if possible)
		report.info(); // empty line for beauty

		//if (not buildinfo.success)
		//	report.warning() << "build failed";


		//
		// -- intermediate name lookup
		//
		// TODO remove this method
		// report.info() << "intermediate name resolution";
		bool successNameLookup = cdeftable.performNameLookup();
		if (unlikely(not successNameLookup and buildinfo.success))
			report.warning() << "name lookup failed";


		//
		// -- Core Objects (bool, u32, u64, f32, ...)
		//
		buildinfo.success &= successNameLookup
			and cdeftable.atoms.fetchAndIndexCoreObjects(report);

		//
		// -- instanciate code from 'main' entry point
		// -- take the input IR, instanciate all functions and classes
		// -- perform complete type checking
		// -- generate opcodes for execution
		//
		buildinfo.success = buildinfo.success
			and buildinfo.isolate.instanciate(report, "main");


		// chrono stop
		yint64 endTime = DateTime::NowMilliSeconds();
		duration = endTime - buildinfo.buildtime;

		// debug info
		if (Config::Traces::printAtomTable)
			printAtomTable(report, cdeftable);


		//
		// -- stopping the queueservice (if any)
		//
		if (isMultithreaded)
		{
			// stop the queueservice only if not started at the beginning
			if (not qsWasStarted)
				queueservice->stop();
			nany_queueservice_unref(&qs);
		}


		// status report
		if (unlikely(not buildinfo.success))
		{
			report.info(); // empty line for beauty (some other errors are already displayed)
			report.error() << "build failed (" << duration << "ms)";
			return nullptr;
		}
		// SUCCESS !
		report.success() << "build succeeded (" << duration << "ms)";
		return buildinfoptr;
	}



	bool Context::build(Logs::Message& message)
	{
		if (unlikely(pTargets.empty() and !pDefaultTarget)) // nothing to build ?
			return false;

		// build report
		Nany::Logs::Report report{message};

		try
		{
			//
			// -- pre user event, which may cancel the build
			//
			if (usercontext.build.on_build_query)
			{
				if (not usercontext.build.on_build_query(&usercontext))
					return false;
			}
			// build accepted !
			if (usercontext.build.on_build_begin)
				usercontext.build.on_build_begin(&usercontext);

			//
			// -- build everything
			//
			int64_t duration = 0;
			std::unique_ptr<BuildInfoContext> buildinfo = doBuildWL(report, duration);
			bool success = (buildinfo.get() != nullptr);

			//
			// -- post user event
			//
			if (usercontext.build.on_build_end)
			{
				auto* creport = reinterpret_cast<const nyreport_t*>(&message);
				nybool_t endSuccess = nytrue;
				usercontext.build.on_build_end(&usercontext, endSuccess, creport, duration);
				success &= (endSuccess == nytrue);
			}

			std::swap(buildinfo, pBuildInfo);
			return success;
		}
		catch (const std::bad_alloc&)
		{
			report.error() << "not enough memory";
		}
		catch (const String& msg)
		{
			report.ICE() << "unexpected error: " << msg;
		}
		catch (const char* msg)
		{
			report.ICE() << "unexpected error: " << msg;
		}
		catch (const std::exception& ex)
		{
			report.ICE() << "unexpected error: " << ex.what();
		}
		catch (...)
		{
			report.error() << "unexpected error";
		}
		return false;
	}




} // namespace Nany
