#include <yuni/yuni.h>
#include "nany/nany.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*! Fill new allocated and freed areas with a certain pattern */
#define NANY_DEBUG_ALLOCATOR_FILL 0




static void* nany_allocate(nyallocator_t* alloc, size_t size)
{
	assert(0 != size);
	assert(alloc != NULL);
	void* p = malloc(size);
	if (YUNI_UNLIKELY(!p) and alloc->on_not_enough_memory)
		alloc->on_not_enough_memory(alloc, nyfalse);
	#if NANY_DEBUG_ALLOCATOR_FILL != 0
	if (p)
		memset(p, 0xCD, size);
	#endif
	return p;
}


static void* nany_reallocate(nyallocator_t* alloc, void* ptr, size_t oldsize, size_t newsize)
{
	void* p;

	assert(0 != newsize);
	assert(alloc != NULL);
	(void) oldsize;

	p = realloc(ptr, newsize);
	if (YUNI_UNLIKELY(!p and alloc->on_not_enough_memory))
		alloc->on_not_enough_memory(alloc, nyfalse);
	return p;
}


static void nany_free(nyallocator_t* allocator, void* ptr, size_t size)
{
	(void) allocator;
	(void) size;
	#if NANY_DEBUG_ALLOCATOR_FILL != 0
	if (ptr)
		memset(p, 0xEF, size);
	#endif
	free(ptr);
}


void nany_memalloc_set_default(nyallocator_t* allocator)
{
	if (allocator)
	{
		allocator->allocate   = &nany_allocate;
		allocator->reallocate = &nany_reallocate;
		allocator->deallocate = &nany_free;
		allocator->limit_mem_size = (size_t) -1; // just to have a value set
		allocator->reserved_mem0  = 0;
		allocator->on_not_enough_memory = NULL;
	}
}







static void* nyallocator_withlimit_allocate(nyallocator_t* allocator, size_t size)
{
	void* p;

	assert(0 != size);
	assert(allocator != NULL);
	// TODO not thread safe
	if ((allocator->reserved_mem0 += size) < allocator->limit_mem_size)
	{
		p = malloc(size);
		if (p)
		{
			#if NANY_DEBUG_ALLOCATOR_FILL != 0
			memset(ptr, 0xCD, size);
			#endif
			return p;
		}

		if (allocator->on_not_enough_memory)
			allocator->on_not_enough_memory(allocator, nyfalse);
	}
	else
	{
		if (allocator->on_not_enough_memory)
			allocator->on_not_enough_memory(allocator, nytrue);
	}
	allocator->reserved_mem0 -= size;
	return NULL;
}


static void* nyallocator_withlimit_reallocate(nyallocator_t* allocator, void* ptr, size_t oldsize, size_t newsize)
{
	void* p;

	assert(allocator != NULL);
	// in this implementation, the total allocated is based on the fact that
	// it costs nothing to reallocate a chunk of memory smaller than the previous one
	// A safer implementation would probably consider that the cost is always based on
	// the old size + the new one

	if (newsize > oldsize)
	{
		// TODO not thread safe
		if ((allocator->reserved_mem0 += (newsize - oldsize)) < allocator->limit_mem_size)
		{
			p = realloc(ptr, newsize);
			if (p)
				return p;

			allocator->reserved_mem0 -= newsize - oldsize;
		}
		else
		{
			if (allocator->on_not_enough_memory)
				allocator->on_not_enough_memory(allocator, nytrue);
			return NULL;
		}
	}
	else
	{
		p = realloc(ptr, newsize);
		if (p)
		{
			allocator->reserved_mem0 -= oldsize - newsize;
			return p;
		}
	}

	if (allocator->on_not_enough_memory)
		allocator->on_not_enough_memory(allocator, nyfalse);
	return NULL;
}


static void nyallocator_withlimit_release(nyallocator_t* allocator, void* ptr, size_t size)
{
	assert(allocator != NULL);
	if (ptr)
	{
		#if NANY_DEBUG_ALLOCATOR_FILL != 0
		memset(ptr, 0xEF, size);
		#endif

		free(ptr);
		// TODO not thread safe
		allocator->reserved_mem0 -= size;
	}
}


void nany_memalloc_set_with_limit(nyallocator_t* allocator, size_t limit)
{
	if (allocator)
	{
		allocator->allocate   = &nyallocator_withlimit_allocate;
		allocator->reallocate = &nyallocator_withlimit_reallocate;
		allocator->deallocate = &nyallocator_withlimit_release;
		allocator->limit_mem_size = limit;
		allocator->reserved_mem0  = 0;
		allocator->on_not_enough_memory = NULL;
	}
}


void nany_memalloc_copy(nyallocator_t* out, const nyallocator_t* const src)
{
	if (out)
		memcpy(out, src, sizeof(nyallocator_t));
}
