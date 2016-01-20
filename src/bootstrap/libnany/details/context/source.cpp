#include <yuni/yuni.h>
#include <yuni/thread/utility.h>
#include <yuni/job/taskgroup.h>
#include <yuni/io/file.h>
#include "source.h"
#include "build-info.h"
#include "target.h"
#include "libnany-config.h"
#include "details/reporting/report.h"
#include <memory>

using namespace Yuni;



namespace Nany
{

	Source::Source(CTarget* target, Source::Type type, AnyString filename, AnyString content)
		: pType{type}
		, pContent{content}
		, pTarget(target)
	{
		switch (type)
		{
			case Type::file:   IO::Canonicalize(pFilename, filename); break;
			case Type::memory: pFilename = filename; break;
		}
	}


	Source::Source(CTarget* target, const Source& rhs) // assuming that rhs is already protected
		: pType(rhs.pType)
		, pFilename(rhs.pFilename)
		, pContent(rhs.pContent)
		, pTarget(target)
	{}


	Source::~Source()
	{
		// keep the symbol local
	}


	inline bool Source::isOutdatedWL(yint64& lastModified) const
	{
		if (pType == Type::file)
		{
			auto lmt = IO::File::LastModificationTime(pFilename);
			if (lmt != pLastCompiled)
			{
				lastModified = lmt;
				return true;
			}
			return false;
		}

		lastModified = pLastCompiled;
		return (pLastCompiled <= 0);
	}


	bool Source::isOutdated(yint64& lastModified) const
	{
		ThreadingPolicy::MutexLocker locker{*this};
		return isOutdatedWL(lastModified);
	}


	void Source::clean()
	{
		// BuildInfo will be deleted outside the critical section
		decltype(pBuildInfo) deleter;

		{
			ThreadingPolicy::MutexLocker locker{*this};
			std::swap(pBuildInfo, deleter);
			pLastCompiled = 0;

			if (pType == Type::file)
			{
				pContent.clear();
				pContent.shrink();
			}
		}
	}


	bool Source::build(BuildInfoContext& ctx, Logs::Report& reporttarget)
	{
		bool astSuccess = false;
		yint64 modified = ctx.buildtime;

		ThreadingPolicy::MutexLocker locker{*this};
		try
		{
			if (not isOutdatedWL(modified))
			{
				astSuccess = true; // not modified
			}
			else
			{
				if (modified <= 0)
					modified = ctx.buildtime;

				// create a new report entry
				auto report = reporttarget.subgroup();

				// reporting
				{
					#ifndef YUNI_OS_WINDOWS
					//AnyString arrow{"\u21E2 "};
					AnyString arrow{"→ "};
					#else
					constexpr const char* arrow = nullptr;
					#endif

					auto entry = (report.info() << "compile " << pFilename);
					entry.message.prefix = arrow;
				}

				// assuming it will succeed, will be reverted to false as soon as something goes wrong
				astSuccess = true;

				if (pType == Type::file)
				{
					pContent.clear();
					pContent.shrink();
					astSuccess = (IO::errNone == IO::File::LoadFromFile(pContent, pFilename));
					if (unlikely(not astSuccess and pTarget))
						pTarget->notifyErrorFileAccess(pFilename);
				}

				// reset build-info
				report.data().origins.location.filename = pFilename;
				report.data().origins.location.target.clear();
				pBuildInfo.reset(nullptr); // making sure that the memory is released first
				pBuildInfo = std::make_unique<BuildInfoSource>();

				if (astSuccess) // file not opened
				{
					// creates an AST from source code
					astSuccess &= passASTFromSourceWL();
					// duplicates the AST and normalize it on-the-fly
					astSuccess &= passDuplicateAndNormalizeASTWL(report);
					// uses the normalized AST to generate high-level nany-IR
					astSuccess &= passTransformASTToIRWL(report);

					// attach the new sequence to the execution context
					if (astSuccess)
					{
						auto& sequence = pBuildInfo->parsing.sequence;
						astSuccess &= ctx.isolate.attach(sequence, report);
					}
				}

				// keep the result of the process somewhere
				pBuildInfo->parsing.success = astSuccess;
			}
		}
		catch (std::bad_alloc&)
		{
			if (pTarget)
				pTarget->notifyNotEnoughMemory();
			astSuccess = false;
		}
		catch (...)
		{
			auto report = reporttarget.subgroup();
			report.ICE() << "uncaught exception when building '" << pFilename << "'";
			astSuccess = false;
		}

		if (not astSuccess)
		{
			pLastCompiled = 0; // error

			// update the global status
			MutexLocker buildMutexLocker{ctx};
			ctx.success = false;
		}
		else
			pLastCompiled = modified;

		return astSuccess;
	}



	void Source::build(BuildInfoContext& ctx, Yuni::Job::Taskgroup& task, Logs::Report& reporttarget)
	{
		Source* self = this;
		self->addRef();

		async(task, [&,self](Job::IJob&) -> bool
		{
			bool success = self->build(ctx, reporttarget);

			// release the internal refcount but not delete ourselves
			self->release();

			return success;
		});
	}




} // namespace Nany
