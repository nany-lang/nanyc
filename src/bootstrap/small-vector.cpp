#pragma once
//#include <yuni/yuni.h>
#include <iostream>


namespace Yuni
{

	template<class T, uint PreAllocatedT = 0, uint chunkSize = 64, class Allocator = std::allocator<T>>
	class SVector final
	{
	public:
		bool empty() const
		{
			return 0 == pData.metadata.size;
		}

		uint size() const
		{
			return pData.metadata.size;
		}

		void clear()
		{
			if (pData.metadata.size != 0)
			{
				if (hasInnerStorage())
				{
					if (not std::is_pod<T>::value)
					{
						for (uint i = 0; i != pData.metadata.size; ++i)
							pData.prealloc.innerStorage[i].~T();
					}
				}
				else
				{
					if (not std::is_pod<T>::value)
					{
						for (uint i = 0; i != pData.metadata.size; ++i)
							pData.dyn.array[i].~T();
					}

					delete[] pData.dyn.array;
				}

				pData.metadata.size = 0;
			}
		}

		void reserve(uint count)
		{
			if (not hasInnerStorage())
			{
				if (pData.dyn.capacity < count)
				{
					uint newcapa = pData.dyn.capacity;
					do { newcapa += chunkSize; } while (pData.dyn.capacity < count);

					T* newarray = allocator.allocate(pData.dync.capacity);
					if (not std::is_pod<T>::value)
					{
						for (uint i = 0; i != pData.dyn.size; ++i)
							newarray[i].T(std::move(pData.dyn.array[i]));
					}
					else
						memcpy(newarray, pData.dyn.array, sizeof(T) * pData.dyn.size);

					allocator.deallocate(pData.dyn.array, pData.dyn.capacity);
					pData.dyn.array = newarray;
					pData.dyn.capacity = newcapa;
				}
			}
		}

		template<class U>
		void push_back(const U& element)
		{
			if (hasInnerStorage())
			{
				if (pData.metadata.size + 1 <= preAllocated)
				{
					pData.prealloc.storage[pData.metadata.size].T(element);
				}
				else
				{
					// mutating into a dynamic container
					uint newcapa = 0;
					uint newsize = pData.metadata.size + 1;
					do { newcapa += chunkSize; } while (pData.dyn.capacity < newsize);

					T* newarray = allocator.allocate(pData.dync.capacity);
					if (not std::is_pod<T>::value)
					{
						for (uint i = 0; i != pData.dyn.size; ++i)
							newarray[i].T(std::move(pData.prealloc.storage[i]));
					}
					else
						memcpy(newarray, &pData.prealloc.storage, sizeof(T) * pData.dyn.size);

					pData.dyn.array = newarray;
					pData.dyn.capacity = newcapa;
				}
			}
			else
			{
				reserve(pData.metadata.size + 1);
				pData.dyn.array[pData.metadata.size].T(element);
			}

			++pData.metadata.size;
		}



		T& operator [] (uint i)
		{
			assert(i < pData.metadata.size);
			return hasInnerStorage()
				? pData.prealloc.innerStorage[i] : pData.dyn.array[i];
		}

		const T& operator [] (uint i) const
		{
			assert(i < pData.metadata.size);
			return hasInnerStorage()
				? pData.prealloc.innerStorage[i] : pData.dyn.array[i];
		}


	private:
		constexpr const uint preAllocated = sizeof(void*) / sizeof(T);
		//! Size in bytes of the pre-allocated storage room
		constexpr const size_t preAllocStorageSize = elementSize * preAllocated;

		bool hasInnerStorage() const { return pData.metadata.size <= preAllocated; }


	private:

		uint pSize = 0;
		union
		{
			struct {
				uint size;
			}
			metadata;
			struct {
				uint size;
				uint capacity;
				void* array;
			}
			dyn;
			struct {
				uint size;
				T innerStorage[preAllocStorageSize];
			}
			prealloc;
		}
		pData;
		Allocator pAllocator;
	};



} // namespace Yuni



int main()
{
	using namespace Yuni;

	SVector<int> array;
}
