#pragma once
#include "stringrefs.h"


namespace ny
{

	inline StringRefs::StringImmutablePointer::~StringImmutablePointer()
	{
		delete[] text;
	}

	inline StringRefs::StringImmutablePointer::StringImmutablePointer(const AnyString& string)
		: text(new char[string.size() + 1])
		, size(string.size())
	{
		::memcpy(text, string.c_str(), sizeof(char) * size);
		text[size] = '\0';
	}

	inline StringRefs::StringImmutablePointer::StringImmutablePointer(StringRefs::StringImmutablePointer&& rhs)
		: text(rhs.text)
		, size(rhs.size)
	{
		rhs.text = nullptr;
	}

	inline AnyString StringRefs::StringImmutablePointer::toString() const
	{
		return AnyString{text, size};
	}







	inline bool StringRefs::empty() const
	{
		return pIndex.empty();
	}


	inline bool StringRefs::exists(const AnyString& text) const
	{
		return (pIndex.count(text) != 0);
	}


	inline uint32_t StringRefs::ref(const AnyString& text)
	{
		if (YUNI_LIKELY(not text.empty()))
		{
			auto it = pIndex.find(text);
			return (it != pIndex.end()) ? it->second : keepString(text);
		}
		return 0;
	}

	inline AnyString StringRefs::refstr(const AnyString& text)
	{
		return (*this)[ref(text)];
	}

	inline AnyString StringRefs::operator [] (uint32_t ix) const
	{
		assert(ix < pRefs.size());
		return pRefs[ix].toString();
	}





} // namespace ny
