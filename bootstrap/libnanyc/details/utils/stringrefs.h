#pragma once
#include <yuni/yuni.h>
#include <yuni/core/dictionary.h>
#include <yuni/string.h>
#include <deque>


namespace ny {


//! Container for minimizing memory use of duplicate strings
struct StringRefs final {
	StringRefs();
	StringRefs(StringRefs&&) = default;
	StringRefs(const StringRefs&);

	//! Add a new entry within the catalog
	AnyString refstr(const AnyString& text);

	//! Get the unique id of a string
	uint32_t ref(const AnyString& text);

	//! Get if a given string is already indexed
	bool exists(const AnyString& text) const;

	//! Clear the container
	void clear();

	//! Retrieve a stored string from its index
	AnyString operator [] (uint32_t ix) const;

	StringRefs& operator = (const StringRefs&);
	StringRefs& operator = (StringRefs&&) = default;

private:
	uint32_t keepString(const AnyString& text);

	//! Storage for all unique strings
	std::deque<yuni::String> m_storage;
	//! Mapping between a stored string and its internal index
	std::unordered_map<AnyString, uint32_t> m_index;

}; // struct StringRefs


} // namespace ny

#include "stringrefs.hxx"
