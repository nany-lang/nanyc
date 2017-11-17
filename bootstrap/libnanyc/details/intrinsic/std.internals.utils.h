#pragma once
#include <yuni/core/string.h>
#include <nanyc/nanyc.h>

namespace ny::intrinsic {

struct FromNanycString final { ///< Non Zero-terminated string
	FromNanycString(const char* data, uint32_t size)
			: m_size(size) {
		if (size != 0) {
			if (m_size < 64) {
				m_cstr = &m_blob[0];
				memcpy(m_blob, data, size);
				m_blob[m_size] = '\0';
			}
			else {
				auto* str = new (&m_blob[0]) yuni::String(data, size);
				m_cstr = str->c_str();
			}
		}
	}

	~FromNanycString() {
		using yuni::String;
		if (not (m_size < 64))
			reinterpret_cast<String*>(&m_blob[0])->~String();
	}

	const char* c_str() const {
		return m_cstr;
	}

	uint32_t size() const {
		return m_size;
	}

	bool empty() const {
		return m_size == 0;
	}

	AnyString anystring() const {
		return AnyString(m_cstr, m_size);
	}

	char* acquireBufferOwnership(nyvmthread_t* tx) {
		if (empty())
			return nullptr;
		char* newptr;
		if (m_size < 64) {
			newptr = (char*) tx->allocator.allocate(&tx->allocator, m_size + 1);
			if (unlikely(newptr == nullptr))
				throw std::bad_alloc();
			memcpy(newptr, m_cstr, m_size + 1);
		}
		else {
			newptr = reinterpret_cast<yuni::String*>(&m_blob[0])->forgetContent();
			newptr = (char*) tx->allocator.transfer_ownership_from_malloc(&tx->allocator, newptr, m_size);
			if (unlikely(newptr == nullptr))
				throw std::bad_alloc();
		}
		m_size = 0;
		return newptr;
	}

private:
	uint32_t m_size;
	const char* m_cstr;
	char m_blob[64];
};

inline void* makeInterimNanycString(nyvmthread_t* tx, const char* cstr, uint64_t size, uint64_t capacity) {
	tx->returnValue.size     = size;
	tx->returnValue.capacity = capacity;
	tx->returnValue.data     = cstr;
	return &tx->returnValue;
}

inline void* makeInterimNanycString(nyvmthread_t* tx, FromNanycString& nanycstring) {
	auto size = nanycstring.size();
	auto* cstr = nanycstring.acquireBufferOwnership(tx);
	return makeInterimNanycString(tx, cstr, size, size);
}

template<class S> inline void* makeInterimNanycString(nyvmthread_t* tx, S& string) {
	auto size = string.size();
	auto capacity = string.capacity();
	auto* cstr = string.forgetContent();
	if (size < capacity - ny::config::extraObjectSize) {
		capacity -= ny::config::extraObjectSize;
	}
	else {
		cstr = (char*) tx->allocator.transfer_ownership_from_malloc(&tx->allocator, cstr, capacity);
		if (unlikely(cstr == nullptr))
			throw std::bad_alloc();
	}
	return makeInterimNanycString(tx, cstr, size, capacity);
}

} // ny::intrinsic
