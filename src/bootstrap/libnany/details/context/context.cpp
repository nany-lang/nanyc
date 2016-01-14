#include <yuni/yuni.h>
#include "details/context/context.h"
#include "details/fwd.h"
#include "details/context/build-info.h"
#include "details/vm/vm.h"
#include <memory>
#include "details/nsl/import-stdcore.h"

using namespace Yuni;





namespace Nany
{

	Context::Context(nycontext_t& context)
		: usercontext(context)
	{
		importNSLCore(*this);
		importNSLCoreIntrinsics(intrinsics);
	}


	Context::Context(nycontext_t& context, const Context& rhs)
		: usercontext(context)
		, intrinsics(rhs.intrinsics)
		, pQueueservice(rhs.pQueueservice)
	{
		// import targets
	}


	Context::~Context()
	{
		pQueueservice = nullptr;
		// detach the default target
		if (!(!pDefaultTarget))
			pDefaultTarget->resetContext(nullptr);

		// detach all targets
		for (auto& pair: pTargets)
			pair.second->resetContext(nullptr);
	}


	void Context::ensureQueueservice()
	{
		if (!pQueueservice)
			pQueueservice = new Job::QueueService();
	}


	/*
	void Context::createReport()
	{
		if (not hasReport())
		{
			if (!pReportMessage) // just in case
			{
				auto* report = new Logs::Message{Logs::Level::none};
				pReportMessage = report;
			}
		}
	}
	*/


	CTarget& Context::defaultTarget()
	{
		if (!pDefaultTarget)
			pDefaultTarget = new CTarget(this, AnyString{});
		return *pDefaultTarget;
	}


	int Context::run(bool& success)
	{
		success = false;
		if (!!pBuildInfo)
			return pBuildInfo->isolate.run(success, usercontext, "main");
		return 0;
	}



} // namespace Nany
