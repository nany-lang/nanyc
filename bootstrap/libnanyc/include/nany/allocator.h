/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_ALLOCATOR_H__
#define __LIBNANYC_ALLOCATOR_H__
#include <stdint.h>
#include <stdlib.h>
#include <nany/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*! \name Memory allocator */
/*@{*/
typedef struct nyallocator_t {
	/*! Allocates some memory */
	void* (*allocate)(struct nyallocator_t*, size_t);
	/*! Re-allocate */
	void* (*reallocate)(struct nyallocator_t*, void* ptr, size_t oldsize, size_t newsize);
	/*! free */
	void (*deallocate)(struct nyallocator_t*, void* ptr, size_t);
	/*! Special values that may not be used directly but are here for performance reasons */
	volatile size_t reserved_mem0;
	/*! Memory usage limit (in bytes) */
	size_t limit_mem_size;
	/*! event: not enough memory */
	void (*on_not_enough_memory)(struct nyallocator_t*, nybool_t limit_reached);
	/*! Flush STDERR */
	void (*release)(const struct nyallocator_t*);
}
nyallocator_t;


/*! Set callbacks to the standard C memory allocator */
NY_EXPORT void nany_memalloc_set_default(nyallocator_t*);

/*! Set callbacks to the std C memory allocator with bounds checking */
NY_EXPORT void nany_memalloc_set_with_limit(nyallocator_t*, size_t limit);

/*! Copy allocator */
NY_EXPORT void nany_memalloc_copy(nyallocator_t* out, const nyallocator_t* const src);
/*@}*/


#ifdef __cplusplus
}
#endif

#endif  /*__LIBNANYC_ALLOCATOR_H__ */
