#include "nany/nany.h"
#include "details/fwd.h"
#include <yuni/job/queue/service.h>
#include <yuni/job/taskgroup.h>
#include <memory>

using namespace Yuni;



namespace // anonymous
{

	static inline nytask_status_t toNanyTaskStatus(Job::Taskgroup::Status status)
	{
		switch (status)
		{
			case Job::Taskgroup::stSucceeded: return nys_succeeded;
			case Job::Taskgroup::stFailed:    return nys_failed;
			case Job::Taskgroup::stCanceled:  return nys_canceled;
			case Job::Taskgroup::stRunning:   return nys_running;
		}
		return nys_failed;
	}

} // anonymous namespace



extern "C" nyqueueservice_t* nany_queueservice_create()
{
	try
	{
		auto ctx = std::make_unique<Job::QueueService>();
		ctx->addRef();
		return reinterpret_cast<nyqueueservice_t*>(ctx.release());
	}
	catch (...) {}
	return nullptr;
}


extern "C" void nany_queueservice_ref(nyqueueservice_t* ctx)
{
	if (ctx)
		reinterpret_cast<Job::QueueService*>(ctx)->addRef();
}


extern "C" void nany_queueservice_unref(nyqueueservice_t** queue)
{
	if (queue and *queue)
	{
		try
		{
			auto* qs = reinterpret_cast<Job::QueueService*>(*queue);
			if (qs->release())
			{
				qs->stop();
				delete qs;
			}
		}
		catch (...) {}
		*queue = nullptr;
	}
}


extern "C" void nany_queueservice_stop(nyqueueservice_t* queue)
{
	if (queue)
	{
		auto* qs = reinterpret_cast<Job::QueueService*>(queue);
		qs->stop();
	}
}


extern "C" nytask_status_t nany_wait(nytask_t* task)
{
	if (task)
	{
		auto status = (reinterpret_cast<Job::Taskgroup*>(task))->wait();
		return toNanyTaskStatus(status);
	}
	return nys_failed;
}


extern "C" nytask_status_t nany_wait_timeout(nytask_t* task, uint32_t ms)
{
	if (task)
	{
		auto status = (reinterpret_cast<Job::Taskgroup*>(task))->wait(ms);
		return toNanyTaskStatus(status);
	}
	return nys_failed;
}


extern "C" void nany_cancel(nytask_t* task)
{
	if (task)
		(reinterpret_cast<Job::Taskgroup*>(task))->cancel();
}


extern "C" nytask_status_t nany_wait_or_cancel(nytask_t* task, uint32_t ms)
{
	if (task)
	{
		auto& taskgroup = *(reinterpret_cast<Job::Taskgroup*>(task));
		auto status = taskgroup.wait(ms);
		if (Job::Taskgroup::stRunning == status)
		{
			taskgroup.cancel();
			taskgroup.wait();
			return nys_canceled;
		}
		return toNanyTaskStatus(status);
	}
	return nys_failed;
}
