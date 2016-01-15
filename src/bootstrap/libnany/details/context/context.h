#pragma once
#include "../fwd.h"
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/string.h>
#include <yuni/job/queue/service.h>
#include <nany/nany.h>
#include "details/intrinsic/intrinsic-table.h"
#include "details/fwd.h"
#include <unordered_map>
#include <memory>
#include "target.h"



namespace Nany
{

	class BuildInfoContext;




	class Context final
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		explicit Context(nycontext_t& inherit);
		//! Copy constructor
		Context(nycontext_t&, const Context&);
		//! Destructor
		~Context();
		//@}


		CTarget& defaultTarget();

		//! \name Build
		//@{
		bool build(Logs::Message* message);
		//@}


		//! \name Execute
		//@{
		int run(bool& success);
		//@}


		void ensureQueueservice();


	public:
		//! Mutex
		Yuni::Mutex mutex;
		//! Attached context
		nycontext_t& usercontext;

		//! All intrinsics
		IntrinsicTable intrinsics;


	private:
		//! Copy constructor (import a template)
		Context(const Context&) = delete;
		std::unique_ptr<BuildInfoContext> doBuildWL(Logs::Report report, int64_t& duration);

	private:
		//! Default target
		CTarget::Ptr pDefaultTarget;
		//! All other targets
		std::unordered_map<AnyString, CTarget::Ptr> pTargets;

		//! Queue service
		Yuni::Job::QueueService::Ptr pQueueservice;
		//! Build info
		std::unique_ptr<BuildInfoContext> pBuildInfo;

		friend class CTarget;

	}; // class Context




} // namespace nany

#include "context.hxx"
