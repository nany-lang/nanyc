#pragma once
#include "libnanyc.h"
#include <yuni/string.h>
#include <vector>
#include "details/atom/atom-map.h"
#include <unordered_map>
#include "nany/nany.h"


#define NANY_MEMCHECKER_PRINT 0

#if NANY_MEMCHECKER_PRINT != 0

#  include <iostream>
#  define NANY_MEMCHECK_HOLD(PTR) \
	do { std::cout << "== nany:vm:memcheck == hold " << (PTR) << '\n'; } while (0)
#  define NANY_MEMCHECK_FORGET(PTR) \
	do { std::cout << "== nany:vm:memcheck == forget " << (PTR) << '\n'; } while (0)

#else
#  define NANY_MEMCHECK_HOLD(PTR)
#  define NANY_MEMCHECK_FORGET(PTR)
#endif



namespace ny {
namespace vm {

class Program;

template<bool EnabledT>
struct MemChecker final {
	static constexpr void atomid(uint32_t) {}
	static constexpr uint32_t atomid() {
		return 0;
	}

	static constexpr void hold(const uint64_t*, size_t, uint32_t) {}
	static constexpr void forget(const uint64_t*) {}
	static constexpr bool checkObjectSize(const uint64_t*, size_t) {
		return true;
	}
	static constexpr size_t fetchObjectSize(const uint64_t* const) {
		return 0;
	}
	static constexpr bool has(const uint64_t*) {
		return true;
	}

	static constexpr void releaseAll(nyallocator_t&) {}
};


template<> struct MemChecker<true> final {
	void atomid(uint32_t atomid) {
		currentAtomid = atomid;
	}

	uint32_t atomid() const {
		return currentAtomid;
	}

	void releaseAll(nyallocator_t&);

	void hold(const uint64_t* const pointer, size_t size, uint32_t lvid) {
		NANY_MEMCHECK_HOLD(pointer);
		ownedPointers.insert(
			std::make_pair(pointer, AllocInfo{size, CLID{currentAtomid, lvid}}));
	}


	void forget(const uint64_t* pointer) {
		NANY_MEMCHECK_FORGET(pointer);
		ownedPointers.erase(pointer);
	}


	bool checkObjectSize(const uint64_t* const pointer, size_t size) const {
		auto it = ownedPointers.find(pointer);
		return (YUNI_LIKELY(it != ownedPointers.end())) and (size == it->second.objsize);
	}

	size_t fetchObjectSize(const uint64_t* const pointer) const {
		auto it = ownedPointers.find(pointer);
		return (YUNI_LIKELY(it != ownedPointers.end())) ? it->second.objsize : 0u;
	}


	bool has(const uint64_t* const pointer) const {
		return YUNI_LIKELY(pointer and ownedPointers.count(pointer) != 0);
	}


	void printLeaksIfAny(const nyprogram_cf_t& cf) const {
		if (YUNI_UNLIKELY(not ownedPointers.empty()))
			printLeaks(cf);
	}


private:
	void printLeaks(const nyprogram_cf_t&) const;

private:
	struct AllocInfo final {
		size_t objsize;
		CLID origin;
	};
	//! All allocated pointers with their size
	std::unordered_map<const uint64_t*, AllocInfo> ownedPointers;
	//! Current atomid
	uint32_t currentAtomid = 0;
};


} // namespace vm
} // namespace ny
