#include "stringrefs.h"

using namespace Yuni;


namespace ny {


StringRefs::StringRefs() {
	m_storage.emplace_back(); // keep the element 0 empty
	m_index.emplace(AnyString(), 0);
}


StringRefs::StringRefs(const StringRefs& other) {
	uint32_t ix = 0;
	for (auto& stored: other.m_storage) {
		m_storage.emplace_back(stored);
		m_index.insert(std::make_pair(AnyString{m_storage.back()}, ix));
		++ix;
	}
}


StringRefs& StringRefs::operator = (const StringRefs& other) {
	if (&other != this) {
		m_index.clear();
		m_storage.clear();
		uint32_t ix = 0;
		for (auto& stored: other.m_storage) {
			m_storage.emplace_back(stored);
			m_index.insert(std::make_pair(AnyString{m_storage.back()}, ix));
			++ix;
		}
	}
	return *this;
}


void StringRefs::clear() {
	m_index.clear();
	m_storage.clear();
	m_storage.emplace_back(); // keep the element 0 empty (as invalid)
	m_index.emplace(AnyString(), 0);
}


uint32_t StringRefs::keepString(const AnyString& text) {
	uint32_t ix = static_cast<uint32_t>(m_storage.size());
	m_storage.emplace_back(text);
	m_index.insert(std::make_pair(AnyString{m_storage.back()}, ix));
	return ix;
}


} // namespace ny
