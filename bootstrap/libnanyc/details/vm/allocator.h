#pragma once
#include <yuni/yuni.h>
#include "libnanyc.h"
#include <cassert>


namespace ny {
namespace vm {
namespace memory {

struct PointerMismatch final {
	PointerMismatch(void* pointer, size_t oldsize, size_t newsize, uint32_t atomid, uint32_t lvid)
		: pointer(pointer)
		, oldsize(oldsize)
		, newsize(newsize)
		, atomid(atomid)
		, lvid(lvid) {
	}
	void* pointer;
	size_t oldsize;
	size_t newsize;
	uint32_t atomid;
	uint32_t lvid;
};

struct UnknownPointer final {
	UnknownPointer(void* pointer, uint32_t atomid, uint32_t lvid)
		: pointer(pointer)
		, atomid(atomid)
		, lvid(lvid) {
	}
	void* pointer;
	uint32_t atomid;
	uint32_t lvid;
};

struct NoTracker final {
	static constexpr void atomid(uint32_t) {}
	static constexpr uint32_t atomid() { return 0; }
	static constexpr void hold(const void*, size_t, uint32_t) {}
	static constexpr void forget(const void*) {}
	static constexpr bool checkObjectSize(const void*, size_t) { return true; }
	static constexpr size_t fetchObjectSize(void*) { return 0; }
	static constexpr bool has(const void*) { return true; }
	static constexpr void releaseAll(nyallocator_t&) {}
};

struct TrackPointer final {
	void atomid(uint32_t atomid) {
		currentAtomid = atomid;
	}

	uint32_t atomid() const {
		return currentAtomid;
	}

	void releaseAll();

	void hold(void* pointer, size_t size, uint32_t lvid) {
		ownedPointers.insert(std::make_pair(pointer, AllocInfo{size, CLID{currentAtomid, lvid}}));
	}

	void forget(const void* pointer) {
		ownedPointers.erase(pointer);
	}

	bool checkObjectSize(void* pointer, size_t size) const {
		auto it = ownedPointers.find(pointer);
		return (likely(it != ownedPointers.end())) and (size == it->second.objsize);
	}

	size_t fetchObjectSize(void* pointer) const {
		auto it = ownedPointers.find(pointer);
		return (likely(it != ownedPointers.end())) ? it->second.objsize : 0u;
	}

	bool has(void* pointer) const {
		return likely(pointer and ownedPointers.count(pointer) != 0);
	}

private:
	struct AllocInfo final { size_t objsize; CLID origin; };
	std::unordered_map<const void*, AllocInfo> ownedPointers;
	uint32_t currentAtomid = 0;
};

template<class Tracker = NoTracker>
struct Allocator final {
	constexpr static const bool fillWithPattern = yuni::debugmode;
	constexpr static const int fillAlloc = 0xCD;
	constexpr static const int fillFree  = 0xEF;
	Tracker tracker;

	void* allocate(size_t size, uint32_t lvid) {
		void* pointer = malloc(size);
		if (likely(pointer != nullptr)) {
			tracker.hold(pointer, size, lvid);
			if (fillWithPattern)
				memset(pointer, fillAlloc, size);
			return pointer;
		}
		throw std::bad_alloc();
	}

	void deallocate(void* pointer, size_t size) {
		if (unlikely(not tracker.checkObjectSize(pointer, size)))
			throw PointerMismatch(pointer, size, size, tracker.atomid(), 0);
		if (fillWithPattern)
			memset(pointer, fillFree, size);
		free(pointer);
		tracker.forget(pointer);
	}

	void* reallocate(void* pointer, size_t oldsize, size_t newsize, uint32_t lvid) {
		if (pointer) {
			if (unlikely(not tracker.checkObjectSize(pointer, oldsize)))
				throw PointerMismatch(pointer, oldsize, newsize, tracker.atomid(), lvid);
			tracker.forget(pointer);
			auto* newptr = realloc(pointer, newsize);
			if (likely(newptr != nullptr)) {
				tracker.hold(newptr, newsize, lvid);
				return newptr;
			}
			deallocate(pointer, oldsize);
			throw std::bad_alloc();
		}
		return allocate(newsize, lvid);
	}

	void releaseAll() {
		tracker.releaseAll();
	}

	void hold(void* pointer, size_t size, uint32_t lvid) {
		tracker.hold(pointer, size, lvid);
	}

	void forget(void* pointer) {
		tracker.forget(pointer);
	}

	bool checkObjectSize(void* pointer, size_t size) const {
		return tracker.checkObjectSize(pointer, size);
	}

	size_t fetchObjectSize(void* pointer) const {
		return tracker.fetchObjectSize(pointer);
	}

	void validate(void* pointer, uint32_t lvid) const {
		if (unlikely(not tracker.has(pointer)))
			throw UnknownPointer(pointer, tracker.atomid(), lvid);
	}
};

} // namespace memory
} // namespace vm
} // namespace
