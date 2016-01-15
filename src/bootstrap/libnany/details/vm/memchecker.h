#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <vector>
#include "details/atom/atom-map.h"
#include <unordered_map>




namespace Nany
{
namespace VM
{


	template<bool EnabledT>
	struct MemChecker final
	{
		static constexpr void atomid(uint32_t) {}
		static constexpr uint32_t atomid() { return 0; }

		static constexpr void hold(const uint64_t*, size_t, uint32_t) {}
		static constexpr void forget(const uint64_t*) {}
		static constexpr bool checkObjectSize(const uint64_t*, size_t) { return true; }
		static constexpr bool has(const uint64_t*) { return true; }

		static constexpr void clear() {}
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

		void clear()
		{
			ownedPointers.clear();
		}

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


		void printLeaksIfAny(nycontext_t& context) const
		{
			if (YUNI_UNLIKELY(not ownedPointers.empty()))
				printLeaks(context);
		}


	private:
		void printLeaks(nycontext_t&) const;

	private:
		struct AllocInfo
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
