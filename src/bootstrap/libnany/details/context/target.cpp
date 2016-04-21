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

	bool CTarget::IsNameValid(const AnyString& name) noexcept
	{
		if (name.size() < 2 or name.size() >= decltype(pName)::chunkSize)
			return false;

		if (not AnyString::IsAlpha(name[0]))
			return false;

		for (size_t i = 1; i != name.size(); ++i)
		{
			auto c = name[(uint)i];
			if (not AnyString::IsAlpha(c) and not AnyString::IsDigit(c) and c != '-')
				return false;
		}
		return true;
	}


	CTarget::CTarget(nyproject_t* project, const AnyString& name)
		: project(project)
		, pName{name}
	{
		Nany::ref(project).targets.all.insert(std::make_pair(AnyString{pName}, this));
	}


	CTarget::CTarget(nyproject_t* project, const CTarget& rhs) // assuming that rhs is already protected
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

				switch (newsource->pType)
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

		Nany::ref(project).targets.all.insert(std::make_pair(AnyString{pName}, this));
	}


	CTarget::~CTarget()
	{
		// remove this target from the list of all targ
		if (project)
			Nany::ref(project).targets.all.erase(AnyString{pName});

		for (auto& ptr: pSources)
			ptr->resetTarget(nullptr); // detach from parent

		// force cleanup
		pSourcesByName.clear();
		pSourcesByFilename.clear();
		pSources.clear();
	}


	bool CTarget::rename(AnyString newname)
	{
		newname.trim();
		if (not IsNameValid(newname))
			return false;

		if (pName.empty()) // disallow renaming of the default target
			return false;

		// copy & lowercase
		if (newname == pName)
			return true; // id - nothing to do

		if (project != nullptr)
		{
			auto& map = Nany::ref(project).targets.all;
			if (map.count(newname) != 0) // failed to rename
				return false;

			addRef(); // keep me alive - just in case - called from the context
			map.erase(AnyString{pName});
			pName = newname;
			map.insert(std::make_pair(AnyString{pName}, this));
			release(); // do not handle the result to avoid invalid destruction when no smarptr is involved
		}
		else
			pName = newname; // nothing else to do

		return true;
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





} // namespace nany
