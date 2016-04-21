/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANY_NANY_C_MEMALLOC_H__
#define __LIBNANY_NANY_C_MEMALLOC_H__
#include "../nany/types.h"



#ifdef __cplusplus
extern "C" {
#endif


typedef struct nyallocator_t nyallocator_t;

typedef struct nyallocator_t
{
	/*! Allocates some memory */
	void* (*allocate)(nyallocator_t*, size_t);
	/*! Re-allocate */
	void* (*reallocate)(nyallocator_t*, void* ptr, size_t oldsize, size_t newsize);
	/*! free */
	void (*deallocate)(nyallocator_t*, void* ptr, size_t);

	/*! Special values that may not be used directly but are here for performance reasons */
	volatile size_t reserved_mem0;
	/*! Memory usage limit (in bytes) */
	size_t limit_mem_size;

	/*! event: not enough memory */
	void (*on_not_enough_memory)(nyallocator_t*, nybool_t limit_reached);
}
nyallocator_t;





/*!
** \nbrief Switch to the standard C memory allocator
*/
NY_EXPORT void nany_memalloc_set_default(nyallocator_t*);

/*!
** \nbrief Switch to the std C memory allocator with bounds checking
*/
NY_EXPORT void nany_memalloc_set_with_limit(nyallocator_t*, size_t limit);

/*!
** \nbrief Copy allocator
*/
NY_EXPORT void nany_memalloc_copy(nyallocator_t* out, const nyallocator_t* const src);



#ifdef __cplusplus
}
#endif

#endif /* __LIBNANY_NANY_C_MEMALLOC_H__ */
