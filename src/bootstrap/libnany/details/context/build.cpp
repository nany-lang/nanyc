#include "build.h"
#include "details/reporting/report.h"
#include "details/ast/tree-index.h"
#include <yuni/datetime/timestamp.h>
#include "details/ir/sequence.h"
#include "details/nsl/import-stdcore.h"
#include "libnany-config.h"

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
		if (cf.on.destroy)
			cf.on.destroy(self(), project.self());

		this->~Build();

		auto& allocator = const_cast<nyallocator_t&>(cf.allocator);
		allocator.deallocate(&allocator, this, sizeof(Nany::Build));
	}


	void Build::prepare()
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
	}


	bool Build::compile()
	{
		// preparing report
		Nany::Logs::Report report{*messages.get()};
		buildtime = DateTime::NowMilliSeconds();

		try
		{
			do
			{
				if (YUNI_UNLIKELY(sources.empty()))
				{
					report.error() << "no target to build";
					break;
				}

				// initialization of some global data
				Nany::Sema::Metadata::initialize(); // TODO remove those methods
				Nany::ASTHelper::initialize();

				success = true;

				// build each source
				for (auto& src: sources)
					success &= src.get().build(*this);

				// end
				duration = DateTime::NowMilliSeconds() - buildtime;
				return success;
			}
			while (false);
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

		success = false;
		duration = DateTime::NowMilliSeconds() - buildtime;
		return false;
	}




} // namespace Nany
