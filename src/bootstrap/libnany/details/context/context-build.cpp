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

		static inline void printDebugInfoClassdefAtomTable(Logs::Report& report, ClassdefTable& table)
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
			if (Config::Traces::printClassdefTable)
			{
				auto trace = (report.trace() << "CLASSDEF PER ATOM\n");
				ClassdefTableView{table}.print(trace.data().message, false);
				report.info(); // for beauty
				report.info(); // for beauty
			}
		}


	} // anonymous namespace




	inline std::unique_ptr<BuildInfoContext> Context::doBuildWL(Logs::Report report, int64_t& duration)
	{
		pBuildInfo.reset(nullptr); // release some memory
		auto buildinfoptr = std::make_unique<BuildInfoContext>(usercontext);
		auto& buildinfo = *(buildinfoptr.get());
		// the result will succeed by default, and will be reverted to false as soon as an error occurs
		buildinfo.success = true;
		// start time

		// build
		report.info("preparing sources").data().section = "build";

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

		if (intrinsics.empty())
			intrinsics.registerStdCore();

		// preparing the queue service if not already present
		auto* qs = usercontext.mt.queueservice;
		auto* queueservice = reinterpret_cast<Job::QueueService*>(qs);
		bool qsWasStarted = false;
		if (queueservice)
		{
			queueservice->addRef();
			qsWasStarted = queueservice->started();
			if (not qsWasStarted)
				queueservice->start();
		}


		// run all tasks
		if (qs)
		{
			Job::Taskgroup task{*queueservice, false};
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
		if (Config::Traces::printSourceOpcodeProgram)
			buildinfo.isolate.print(report);

		// instanciate the whole code (if possible)
		report.info(); // empty line for beauty
		report.info("compiling assemblies");

		//if (not buildinfo.success)
		//	report.warning() << "build failed";

		// report.info() << "intermediate name resolution";
		bool successNameLookup = cdeftable.performNameLookup();
		if (unlikely(not successNameLookup and buildinfo.success))
			report.warning() << "name lookup failed";

		buildinfo.success &= successNameLookup
			and cdeftable.atoms.fetchAndIndexCoreObjects(report)
			and buildinfo.isolate.instanciate(report, "main");

		yint64 endTime = DateTime::NowMilliSeconds();
		duration = endTime - buildinfo.buildtime;

		if (Config::Traces::printAtomTable)
		{
			auto trace = report.trace();
			trace.message.prefix = "ATOMS";
			cdeftable.atoms.root.print(report, ClassdefTableView{cdeftable});
			report.info(); // for beauty
			report.info(); // for beauty
		}

		if (qs)
		{
			if (not qsWasStarted) // stop the queueservice if it was not already started
				queueservice->stop();
			nany_queueservice_unref(&qs);
		}


		if (unlikely(not buildinfo.success))
		{
			report.info(); // empty line for beauty (some other errors are already displayed)
			report.error() << "build failed. some error occured (" << duration << "ms)";
			return nullptr;
		}

		report.success() << "build succeeded (" << duration << "ms)";
		return buildinfoptr;
	}



	bool Context::build(Logs::Message* message)
	{
		assert(message != nullptr);

		// build report
		Nany::Logs::Report report{*message};

		bool success = false;
		try
		{
			if (usercontext.build.on_build_query)
			{
				if (not usercontext.build.on_build_query(&usercontext))
					return false;
			}

			if (unlikely(pTargets.empty() and !pDefaultTarget)) // nothing to build ?
				return false;

			if (usercontext.build.on_build_begin)
				usercontext.build.on_build_begin(&usercontext);

			// build
			int64_t duration = 0;
			std::unique_ptr<BuildInfoContext> buildinfo = doBuildWL(report, duration);
			success = (buildinfo.get() != nullptr);

			if (usercontext.build.on_build_end)
			{
				auto* creport = reinterpret_cast<const nyreport_t*>(message);
				nybool_t endSuccess = nytrue;
				usercontext.build.on_build_end(&usercontext, endSuccess, creport, duration);
				success &= (endSuccess == nytrue);
			}

			std::swap(buildinfo, pBuildInfo);
		}
		catch (const std::bad_alloc&)
		{
			report.error() << "not enough memory";
			success = false;
		}
		catch (const String& msg)
		{
			report.ICE() << "unexpected error: " << msg;
			success = false;
		}
		catch (const char* msg)
		{
			report.ICE() << "unexpected error: " << msg;
			success = false;
		}
		catch (const std::exception& ex)
		{
			report.ICE() << "unexpected error: " << ex.what();
			success = false;
		}
		catch (...)
		{
			report.error() << "unexpected error";
			success = false;
		}
		return success;
	}




} // namespace Nany
