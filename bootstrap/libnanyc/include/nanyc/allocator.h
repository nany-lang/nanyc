/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_ALLOCATOR_H__
#define __LIBNANYC_ALLOCATOR_H__

#include <nanyc/types.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nyallocator_t nyallocator_t;

typedef struct nyallocator_t {
	void* userdata;
	void* (*allocate)(nyallocator_t*, size_t);
	void (*deallocate)(nyallocator_t*, void*, size_t);
	void* (*reallocate)(nyallocator_t*, void*, size_t old_size, size_t new_size);
	void (*on_release)(nyallocator_t*);
	void (*on_not_enough_memory)(nyallocator_t*, const char* msg, uint32_t msg_len);
}
nyallocator_t;

struct nyallocator_opts_t {
	void* userdata;
	void* (*allocate)(nyallocator_t*, size_t);
	void (*deallocate)(nyallocator_t*, void*, size_t);
	void* (*reallocate)(nyallocator_t*, void*, size_t old_size, size_t new_size);
	void (*on_release)(nyallocator_t*);
	void (*on_not_enough_memory)(nyallocator_t*, const char* msg, uint32_t msg_len);
};

/*!
** \brief Make a new allocator object for using malloc
*/
NY_EXPORT nyallocator_t* nyallocator_make_from_malloc();

/*!
** \brief Make a new allocator object
*/
NY_EXPORT nyallocator_t* nyallocator_make(const nyallocator_opts_t*);

/*!
** \brief Initialize a preallocated allocator object
*/
NY_EXPORT void nyallocator_init(nyallocator_t*, const nyallocator_opts_t*);

/*!
** \brief Initialize a preallocated allocator object for using malloc
*/
NY_EXPORT void nyallocator_init_from_malloc(nyallocator_t*);

/*!
** \brief Release all the resources held by the allocator
*/
NY_EXPORT void nyallocator_dispose(nyallocator_t*);

/*!
** \brief Allocate a chunk of memory
*/
inline void* nyallocate(nyallocator_t*, size_t size);

/*!
** \brief Re-Allocate a chunk of memory
*/
inline void* nyreallocate(nyallocator_t*, void* ptr, size_t old_size, size_t new_size);

/*!
** \brief Release a chunk of memory previously allocated by nyallocate()
*/
inline void nydeallocate(nyallocator_t*, void* ptr, size_t size);




/* *** */




inline void* nyallocate(nyallocator_t* allocator, size_t size) {
	assert(allocator != NULL && "invalid nyallocator_t pointer");
	return allocator->allocate(allocator, size);
}

inline void* nyreallocate(nyallocator_t* allocator, void* ptr, size_t old_size, size_t new_size) {
	assert(allocator != NULL && "invalid nyallocator_t pointer");
	return allocator->reallocate(allocator, ptr, old_size, new_size);
}

inline void nydeallocate(nyallocator_t* allocator, void* ptr, size_t size) {
	assert(allocator != NULL && "invalid nyallocator_t pointer");
	allocator->deallocate(allocator, ptr, size);
}

#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_ALLOCATOR_H__ */
