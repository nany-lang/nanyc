#include "stringrefs.h"

using namespace Yuni;


namespace ny {


StringRefs::StringRefs() {
	m_storage.reserve(8);
	m_storage.emplace_back(); // keep the element 0 empty
}


void StringRefs::clear() {
	m_storage.clear();
	m_storage.emplace_back(); // keep the element 0 empty
	m_index.clear();
}


uint32_t StringRefs::keepString(const AnyString& text) {
	assert(not text.empty());
	uint32_t ix = static_cast<uint32_t>(m_storage.size());
	m_storage.emplace_back(text);
	m_index.insert(std::make_pair(m_storage.back().toString(), ix));
	return ix;
}


size_t StringRefs::sizeInBytes() const {
	size_t s = sizeof(void*) * m_storage.capacity();
	for (auto& element : m_storage)
		s += element.m_size + 1;
	// arbitrary
	s += m_index.size() * (sizeof(std::pair<AnyString, uint32_t>) * sizeof(void*) * 2);
	return s;
}


} // namespace ny
