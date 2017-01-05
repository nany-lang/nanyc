#include "stringrefs.h"

using namespace Yuni;


namespace ny {


namespace {


template<class K, class V>
auto estimateMapMemoryOverhead(const std::unordered_map<K,V>& map) {
	auto entrySize = sizeof(K) + sizeof(V) + sizeof(void*);
	auto bucketSize = sizeof(void*);
	auto adminSize = 3 * sizeof(void*) + sizeof(size_t);
	return adminSize + map.size() * entrySize + map.max_bucket_count() * bucketSize;
}


} // namespace


StringRefs::StringRefs() {
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
	size_t bytes = estimateMapMemoryOverhead(m_index);
	for (auto& element : m_storage)
		bytes += element.m_size;
	return bytes;
}


} // namespace ny
