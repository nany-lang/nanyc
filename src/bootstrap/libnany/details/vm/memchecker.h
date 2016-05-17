#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <vector>
#include "details/atom/atom-map.h"
#include <unordered_map>
#include "nany/nany.h"




namespace Nany
{
namespace VM
{

	class Program;

	template<bool EnabledT>
	struct MemChecker final
	{
		static constexpr void atomid(uint32_t) {}
		static constexpr uint32_t atomid() { return 0; }

		static constexpr void hold(const uint64_t*, size_t, uint32_t) {}
		static constexpr void forget(const uint64_t*) {}
		static constexpr bool checkObjectSize(const uint64_t*, size_t) { return true; }
		static constexpr bool has(const uint64_t*) { return true; }

		static constexpr void releaseAll(nyallocator_t&) {}
	};



	template<>
	struct MemChecker<true> final
	{
		void atomid(uint32_t atomid)
		{
			currentAtomid = atomid;
		}

		uint32_t atomid() const
		{
			return currentAtomid;
		}

		void releaseAll(nyallocator_t&);

		void hold(const uint64_t* const pointer, size_t size, uint32_t lvid)
		{
			ownedPointers.insert(
				std::make_pair(pointer, AllocInfo{size, CLID{currentAtomid, lvid}}));
		}


		void forget(const uint64_t* pointer)
		{
			ownedPointers.erase(pointer);
		}


		bool checkObjectSize(const uint64_t* const pointer, size_t size) const
		{
			auto it = ownedPointers.find(pointer);
			return (YUNI_LIKELY(it != ownedPointers.end())) and (size == it->second.objsize);
		}


		bool has(const uint64_t* const pointer) const
		{
			return YUNI_LIKELY(pointer and ownedPointers.count(pointer) != 0);
		}


		void printLeaksIfAny(const nyprogram_cf_t& cf) const
		{
			if (YUNI_UNLIKELY(not ownedPointers.empty()))
				printLeaks(cf);
		}


	private:
		void printLeaks(const nyprogram_cf_t&) const;

	private:
		struct AllocInfo final
		{
			size_t objsize;
			CLID origin;
		};

		//! All allocated pointers with their size
		std::unordered_map<const uint64_t*, AllocInfo> ownedPointers;
		//! Current atomid
		uint32_t currentAtomid = 0;
	};





} // namespace VM
} // namespace Nany
