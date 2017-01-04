#pragma once
#include "stringrefs.h"


namespace ny {


inline StringRefs::StringImmutablePointer::~StringImmutablePointer() {
	delete[] m_cstr;
}


inline StringRefs::StringImmutablePointer::StringImmutablePointer(const AnyString& string)
	: m_cstr(new char[string.size() + 1])
	, m_size(string.size()) {
	::memcpy(m_cstr, string.c_str(), sizeof(char) * m_size);
	m_cstr[m_size] = '\0';
}


inline StringRefs::StringImmutablePointer::StringImmutablePointer(StringRefs::StringImmutablePointer&& rhs)
	: m_cstr(rhs.m_cstr)
	, m_size(rhs.m_size) {
	rhs.m_cstr = nullptr;
}


inline AnyString StringRefs::StringImmutablePointer::toString() const {
	return AnyString{m_cstr, m_size};
}


inline bool StringRefs::exists(const AnyString& text) const {
	return (m_index.count(text) != 0);
}


inline uint32_t StringRefs::ref(const AnyString& text) {
	if (YUNI_LIKELY(not text.empty())) {
		auto it = m_index.find(text);
		return (it != m_index.end()) ? it->second : keepString(text);
	}
	return 0;
}

inline AnyString StringRefs::refstr(const AnyString& text) {
	return (*this)[ref(text)];
}


inline AnyString StringRefs::operator [] (uint32_t ix) const {
	assert(ix < m_storage.size());
	return m_storage[ix].toString();
}


} // namespace ny
