// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// \brief   Low level memory management
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// \brief   Low level memory management
// \file    allocator.ny
// \ingroup std.memory

namespace std.memory;



/*!
** \brief Allocate a new chunk of memory
*/
#[nosuggest] public func allocate(size: __u64): __pointer
	-> !!memory.allocate(size);

/*!
** \brief Allocate a new chunk of memory
*/
public func allocate(size: u64): __pointer
	-> !!memory.allocate(size.pod);

/*!
** \brief Allocate a new chunk of memory
*/
#[nosuggest] public func allocate<:T:>(count: __u64): __pointer
	-> !!memory.allocate(size * sizeof(T));

/*!
** \brief Allocate a new chunk of memory
*/
public func allocate<:T:>(count: u64): __pointer
	-> !!memory.allocate((size * sizeof(T)).pod);


/*!
** \brief Re-allocate a new chunk of memory
*/
#[nosuggest] public func reallocate(ptr: __pointer, oldsize: __u64, newsize: __u64): __pointer
	-> !!memory.realloc(ptr, oldsize, newsize);

/*!
** \brief Re-allocate a new chunk of memory
*/
public func reallocate(ptr: __pointer, oldsize: u64, newsize: u64): __pointer
	-> !!memory.realloc(ptr, oldsize.pod, newsize.pod);



/*!
** \brief Dispose a chunk of memory
** \param ptr A pointer previously allocated by allocate()
** \param size The size in bytes of the allocated chunk
*/
#[nosuggest] public func dispose(ptr: __pointer, size: __u64): void
	-> !!memory.dispose(ptr, size);

/*!
** \brief Dispose a chunk of memory
** \param ptr A pointer previously allocated by allocate()
** \param size The size in bytes of the allocated chunk
*/
public func dispose(ptr: __pointer, size: u64): void
	-> !!memory.dispose(ptr, size.pod);


/*!
** \brief Copy memory regions
*/
public func copy(dst: __pointer, src: __pointer, size: u64): void
	-> !!memory.copy(dst, src, size.pod);

/*!
** \brief Copy memory regions
*/
public func copy(dst: __pointer, src: __pointer, size: __u64): void
	-> !!memory.copy(dst, src, size);

/*!
** \brief Copy memory regions which may overlap
*/
public func copyOverlap(dst: __pointer, src: __pointer, size: u64): void
	-> !!memory.move(dst, src, size.pod);

/*!
** \brief Copy memory regions which may overlap
*/
public func copyOverlap(dst: __pointer, src: __pointer, size: __u64): void
	-> !!memory.move(dst, src, size);

/*!
** \brief Fill memory with a given pattern
*/
public func fill(ptr: __pointer, size: u64, pattern: u8): void
	-> !!memory.fill(ptr, size.pod, pattern.pod);

/*!
** \brief Fill memory with a given pattern
*/
#[nosuggest] public func fill(ptr: __pointer, size: __u64, pattern: __u8): void
	-> !!memory.fill(ptr, size, pattern);

/*!
** \brief Fill memory with a given pattern
*/
public func zero(ptr: __pointer, size: u64): void
	-> !!memory.fill(ptr, size.pod, 0__u8);

/*!
** \brief Fill memory with a given pattern
*/
#[nosuggest] public func zero(ptr: __pointer, size: __u64): void
	-> !!memory.fill(ptr, size, 0__u8);


/*!
** \brief Get if 2 memory areas are identical
*/
#[nosuggest] public func equals(p1: __pointer, p2: __pointer, size: __u64): bool
	-> new bool(!!memory.cmp(p1, p2, size) == 0__u32);

/*!
** \brief Get if 2 memory areas are identical
*/
public func equals(p1: __pointer, p2: __pointer, size: u64): bool
	-> new bool(!!memory.cmp(p1, p2, size.pod) == 0__u32);


#[nosuggest] public func less(p1: __pointer, p2: __pointer, size: __u64): bool
	-> new bool(!!memory.cmp(p1, p2, size) == 2__u32);

public func less(p1: __pointer, p2: __pointer, size: u64): bool
	-> new bool(!!memory.cmp(p1, p2, size.pod) == 2__u32);

#[nosuggest] public func greater(p1: __pointer, p2: __pointer, size: __u64): bool
	-> new bool(!!memory.cmp(p1, p2, size) == 1__u32);

public func greater(p1: __pointer, p2: __pointer, size: u64): bool
	-> new bool(!!memory.cmp(p1, p2, size.pod) == 1__u32);


/*!
** \brief Calculate the length of a string (in bytes)
*/
public func strlen(str: __pointer): u32
	-> new u32(if str != null then !!__nanyc_strlen(str) else 0__u32);




#[nosuggest] public func nanyc_internal_create_string(ptr: __pointer): ref string
{
	ref content = new string;
	// pointer to a special per-thread struct
	if ptr != null then
	{
		var size = !!load.u64(ptr);
		if size != 0__u64 then
		{
			var capacity = !!load.u64(ptr + 8__u32);
			var newptr   = !!load.ptr(ptr + 16__u32);
			// avoid false asserts when the memory checker is active
			!!__nanyc_memchecker_hold(newptr, capacity);

			// this func should return a std.Clob (which has 64bits size)
			if (capacity < 4294967295__u32)
			{
				content.adopt(newptr,
					!!__reinterpret(size, #[__nanyc_synthetic] __u32),
					!!__reinterpret(capacity, #[__nanyc_synthetic] __u32));
			}
			else
				std.memory.dispose(newptr, capacity);
		}
	}
	//else
	//   error
	return content;
}




// -*- mode: nany;-*-
// vim: set filetype=nany:
