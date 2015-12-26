#include <yuni/yuni.h>
#include <yuni/job/queue/service.h>
#include "details/context/context.h"
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


	CTarget::CTarget(Context* ctx, const AnyString& name)
		: pContext(ctx)
		, pName{name}
	{}


	CTarget::CTarget(Context* ctx, const CTarget& rhs) // assuming that rhs is already protected
		: pContext(ctx)
		, pName(rhs.pName)
	{
		if (not rhs.pSources.empty())
		{
			pSources.reserve(rhs.pSources.size());
			for (auto& ptr: rhs.pSources)
			{
				auto& source = *ptr;

				Source::ThreadingPolicy::MutexLocker locker{source};
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
	}


	CTarget::~CTarget()
	{
		ThreadingPolicy::MutexLocker locker{*this};

		for (auto& ptr: pSources)
			ptr->resetTarget(nullptr); // detach from parent

		// force cleanup
		pSourcesByName.clear();
		pSourcesByFilename.clear();
		pSources.clear();
	}


	void CTarget::notifyNotEnoughMemory() const
	{
		ThreadingPolicy::MutexLocker locker{*this};
		if (pContext)
			pContext->usercontext.memory.on_not_enough_memory(&(pContext->usercontext));
	}


	void CTarget::notifyErrorFileAccess(const AnyString& filename) const
	{
		ThreadingPolicy::MutexLocker locker{*this};
		if (pContext)
		{
			auto func = pContext->usercontext.build.on_err_file_access;
			if (func)
				func(&(pContext->usercontext), filename.c_str(), filename.size());
		}
	}


	bool CTarget::rename(AnyString newname)
	{
		newname.trim();
		if (not IsNameValid(newname))
			return false;

		ThreadingPolicy::MutexLocker locker{*this};
		if (pName.empty()) // disallow renaming of the default target
			return false;

		// copy & lowercase
		if (newname == pName)
			return true; // id - nothing to do

		if (pContext != nullptr)
		{
			auto& map = pContext->pTargets;
			if (map.count(newname) != 0)
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

			ThreadingPolicy::MutexLocker locker{*this};

			auto it = pSourcesByName.find(source->pFilename);
			if (it != pSourcesByName.end())
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


	void CTarget::fetchSourceList(BuildInfoContext& ctxbuildinfo)
	{
		ThreadingPolicy::MutexLocker locker{*this};
		ctxbuildinfo.sources.reserve(ctxbuildinfo.sources.size() + pSources.size());
		for (auto& ptr: pSources)
			ctxbuildinfo.sources.push_back(ptr);
	}




} // namespace nany
