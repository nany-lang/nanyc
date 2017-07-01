#include <yuni/yuni.h>
#include "nany/nany.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*! Fill new allocated and freed areas with a certain pattern */
#define NANY_DEBUG_ALLOCATOR_FILL 0


static void nyallocator_abort(nyoldalloc_t* allocator) {
	if (allocator->on_not_enough_memory)
		allocator->on_not_enough_memory(allocator, nyfalse);
	if (allocator->on_internal_abort)
		allocator->on_internal_abort(allocator);
}


static void* nystdmalloc_alloc(nyoldalloc_t* allocator, size_t size) {
	void* p = malloc(size);
	if (p)
		return p;
	nyallocator_abort(allocator);
	return NULL;
}


static void* nystdmalloc_reallocate(nyoldalloc_t* allocator, void* ptr, size_t oldsize, size_t newsize) {
	(void) oldsize;
	void* p = realloc(ptr, newsize);
	if (p)
		return p;
	free(ptr);
	nyallocator_abort(allocator);
	return NULL;
}


static void nystdmalloc_deallocate(nyoldalloc_t* allocator, void* ptr, size_t size) {
	(void) allocator;
	(void) size;
	free(ptr);
}


static nybool_t nystdmalloc_create_mt(nyoldalloc_t* allocator, const nyallocator_cf_t* cf) {
	memset(allocator, 0x0, sizeof(nyoldalloc_t));
	allocator->on_not_enough_memory = cf->on_not_enough_memory;
	allocator->allocate = &nystdmalloc_alloc;
	allocator->reallocate = &nystdmalloc_reallocate;
	allocator->deallocate = &nystdmalloc_deallocate;
	return nytrue;
}


void nyallocator_cf_init(nyallocator_cf_t* cf) {
	if (cf) {
		memset(cf, 0x0, sizeof(nyallocator_cf_t));
		cf->create_mt = &nystdmalloc_create_mt;
		cf->create_st = &nystdmalloc_create_mt;
	}
}


static void* nany_allocate(nyoldalloc_t* alloc, size_t size)
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


static void* nany_reallocate(nyoldalloc_t* alloc, void* ptr, size_t oldsize, size_t newsize)
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


static void nany_free(nyoldalloc_t* allocator, void* ptr, size_t size)
{
	(void) allocator;
	(void) size;
	#if NANY_DEBUG_ALLOCATOR_FILL != 0
	if (ptr)
		memset(p, 0xEF, size);
	#endif
	free(ptr);
}


void nany_memalloc_set_default(nyoldalloc_t* allocator)
{
	if (allocator)
	{
		allocator->allocate   = &nany_allocate;
		allocator->reallocate = &nany_reallocate;
		allocator->deallocate = &nany_free;
		allocator->limit_mem_size = (size_t) -1; // just to have a value set
		allocator->reserved_mem0  = 0;
		allocator->on_not_enough_memory = NULL;
		allocator->release = NULL;
	}
}







static void* nyallocator_withlimit_allocate(nyoldalloc_t* allocator, size_t size)
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


static void* nyallocator_withlimit_reallocate(nyoldalloc_t* allocator, void* ptr, size_t oldsize, size_t newsize)
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


static void nyallocator_withlimit_release(nyoldalloc_t* allocator, void* ptr, size_t size)
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


void nany_memalloc_set_with_limit(nyoldalloc_t* allocator, size_t limit)
{
	if (allocator)
	{
		allocator->allocate   = &nyallocator_withlimit_allocate;
		allocator->reallocate = &nyallocator_withlimit_reallocate;
		allocator->deallocate = &nyallocator_withlimit_release;
		allocator->limit_mem_size = limit;
		allocator->reserved_mem0  = 0;
		allocator->on_not_enough_memory = NULL;
		allocator->release = NULL;
	}
}


void nany_memalloc_copy(nyoldalloc_t* out, const nyoldalloc_t* const src)
{
	if (out)
		memcpy(out, src, sizeof(nyoldalloc_t));
}
