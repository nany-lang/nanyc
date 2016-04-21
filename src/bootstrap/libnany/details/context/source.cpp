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
#include "details/context/build.h"

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
		return isOutdatedWL(lastModified);
	}


	bool Source::build(Build& build)
	{
		bool success = false;
		yint64 modified = build.buildtime;

		try
		{
			if (not isOutdatedWL(modified))
			{
				success = true; // not modified
			}
			else
			{
				if (modified <= 0)
					modified = build.buildtime;

				// create a new report entry
				auto report = Logs::Report{*build.messages}.subgroup();

				// reporting
				if (pFilename.first() != '{')
				{
					#ifndef YUNI_OS_WINDOWS
					AnyString arrow{"\u2192 "};
					#else
					constexpr const char* arrow = nullptr;
					#endif

					auto entry = (report.info() << "building " << pFilename);
					entry.message.prefix = arrow;
				}

				// assuming it will succeed, will be reverted to false as soon as something goes wrong
				success = true;

				if (pType == Type::file)
				{
					pContent.clear();
					pContent.shrink();
					success = (IO::errNone == IO::File::LoadFromFile(pContent, pFilename));
					if (unlikely(not success and pTarget))
					{
						auto f = build.cf.on_error_file_eacces;
						if (f)
							f(build.project.self(), build.self(), pFilename.c_str(), pFilename.size());
					}
				}

				// reset build-info
				report.data().origins.location.filename = pFilename;
				report.data().origins.location.target.clear();
				pBuildInfo.reset(nullptr); // making sure that the memory is released first
				pBuildInfo = std::make_unique<BuildInfoSource>();

				if (success) // file not opened
				{
					// creates an AST from source code
					success &= passASTFromSourceWL();
					// duplicates the AST and normalize it on-the-fly
					success &= passDuplicateAndNormalizeASTWL(report);
					// uses the normalized AST to generate high-level nany-IR
					success &= passTransformASTToIRWL(report);

					// attach the new sequence to the execution context
					if (success)
					{
						auto& sequence = pBuildInfo->parsing.sequence;
						success &= build.attach(sequence, report);
					}
				}

				// keep the result of the process somewhere
				pBuildInfo->parsing.success = success;
			}
		}
		catch (std::bad_alloc&)
		{
			build.printStderr("ICE: not enough memory");
			success = false;
		}
		catch (...)
		{
			auto report = Logs::Report{*build.messages};
			report.ICE() << "uncaught exception when building '" << pFilename << "'";
			success = false;
		}

		if (not success)
		{
			pLastCompiled = 0; // error
			// update the global status
			build.success = false;
		}
		else
			pLastCompiled = modified;

		return success;
	}





} // namespace Nany
