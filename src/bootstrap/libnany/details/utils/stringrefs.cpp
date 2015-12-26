#include "stringrefs.h"

using namespace Yuni;




namespace Nany
{

	StringRefs::StringRefs()
	{
		pRefs.reserve(8);
		pRefs.emplace_back(); // keep the element 0 empty
	}


	void StringRefs::clear()
	{
		pRefs.clear();
		pRefs.emplace_back(); // keep the element 0 empty
		pIndex.clear();
	}


	uint32_t StringRefs::keepString(const AnyString& text)
	{
		assert(not text.empty());
		uint32_t ix = static_cast<uint32_t>(pRefs.size());
		pRefs.emplace_back(text);
		pIndex.insert(std::make_pair(pRefs.back().toString(), ix));
		return ix;
	}


	size_t StringRefs::inspectMemoryUsage() const
	{
		size_t s = sizeof(void*) * pRefs.max_size();
		for (auto& element: pRefs)
			s += element.size + 1;
		// arbitrary
		s += pIndex.size() * (sizeof(std::pair<AnyString, uint32_t>) * sizeof(void*) * 2);
		return s;
	}




} // namespace Nany
