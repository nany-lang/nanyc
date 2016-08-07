#include <yuni/yuni.h>
#include <yuni/job/queue/service.h>
#include "details/context/project.h"
#include "details/context/target.h"
#include "details/context/build-info.h"
#include "details/fwd.h"
#include <memory>
using namespace Yuni;




namespace Nany
{


	inline void CTarget::registerTargetToProject()
	{
		// !!internal: using `pName` and not `name`, since the internal pointer
		// is not the same
		auto& prj = Nany::ref(project);
		AnyString name{pName};

		// add the target in the project
		prj.targets.all.insert(std::make_pair(name, this));
		// event
		if (prj.cf.on_target_added)
			prj.cf.on_target_added(project, self(), name.c_str(), name.size());
	}


	inline void CTarget::unregisterTargetFromProject()
	{
		// remove this target from the list of all targ
		if (project)
		{
			auto& prj = Nany::ref(project);
			AnyString name{pName};

			// event
			if (prj.cf.on_target_removed)
				prj.cf.on_target_removed(project, self(), name.c_str(), name.size());
			// remove the target from the list of all targets
			prj.targets.all.erase(name);
			// reset project pointer
			project = nullptr;
		}
	}


	CTarget::CTarget(nyproject_t* project, const AnyString& name)
		: project(project)
		, pName{name}
	{
		assert(project != nullptr);
		registerTargetToProject();
	}


	CTarget::CTarget(nyproject_t* project, const CTarget& rhs)
		: project(project)
		, pName(rhs.pName)
	{
		if (not rhs.pSources.empty())
		{
			pSources.reserve(rhs.pSources.size());
			for (auto& ptr: rhs.pSources)
			{
				auto& source = *ptr;

				Source::Ptr newsource{new Source(this, source)};
				pSources.push_back(newsource);
				auto& ref = *newsource;

				switch (ref.pType)
				{
					case Source::Type::memory:
					{
						pSourcesByName.insert(std::make_pair(ref.pFilename, std::ref(ref)));
						break;
					}
					case Source::Type::file:
					{
						pSourcesByFilename.insert(std::make_pair(ref.pFilename, std::ref(ref)));
						break;
					}
				}
			}
		}

		registerTargetToProject();
	}


	CTarget::~CTarget()
	{
		unregisterTargetFromProject();

		for (auto& ptr: pSources)
			ptr->resetTarget(nullptr); // detach all sources from parent

		// force cleanup
		pSourcesByName.clear();
		pSourcesByFilename.clear();
		pSources.clear();
	}


	void CTarget::addSource(const AnyString& name, const AnyString& content)
	{
		if (not content.empty())
		{
			Source::Ptr source{new Source(this, Source::Type::memory, name, content)};

			auto it = pSourcesByName.find(source->pFilename);
			if (it == pSourcesByName.end())
				pSourcesByName.insert(std::make_pair(AnyString{source->pFilename}, std::ref(*source)));
			else
				it->second = std::ref(*source);

			pSources.push_back(source);
		}
	}


	void CTarget::addSourceFromFile(const AnyString& filename)
	{
		if (not filename.empty())
		{
			auto source = std::make_unique<Source>(this, Source::Type::file, filename, AnyString());

			auto it = pSourcesByFilename.find(source->pFilename);
			if (it == pSourcesByFilename.end())
				pSourcesByFilename.insert(std::make_pair(AnyString{source->pFilename}, std::ref(*source)));
			else
				it->second = std::ref(*source);

			pSources.emplace_back(source.release());
		}
	}


	bool CTarget::IsNameValid(const AnyString& name) noexcept
	{
		if (unlikely(name.size() < 2 or not (name.size() < decltype(pName)::chunkSize)))
			return false;

		if (unlikely(not AnyString::IsAlpha(name[0])))
			return false;

		for (size_t i = 1; i < name.size(); ++i)
		{
			auto c = name[(uint)i];
			if (not AnyString::IsAlpha(c) and not AnyString::IsDigit(c) and c != '-')
				return false;
		}
		return true;
	}





} // namespace nany
