#pragma once
#include "stringrefs.h"

namespace ny {

inline bool StringRefs::exists(const AnyString& text) const {
	return m_index.count(text) != 0;
}

inline uint32_t StringRefs::ref(const AnyString& text) {
	auto it = m_index.find(text);
	return it != m_index.end() ? it->second : keepString(text);
}

inline AnyString StringRefs::refstr(const AnyString& text) {
	return (*this)[ref(text)];
}

inline AnyString StringRefs::operator [] (uint32_t ix) const {
	assert(ix < m_storage.size());
	return m_storage[ix];
}

} // ny
