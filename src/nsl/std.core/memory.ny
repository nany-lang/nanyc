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


//! Allocate a new chunk of memory
#[nosuggest] public func allocate(size: __u64): __pointer {
	return !!memory.allocate(size);
}

//! Allocate a new chunk of memory
public func allocate(size: u64): __pointer {
	return !!memory.allocate(size.pod);
}

//! Allocate a new chunk of memory
#[nosuggest] public func allocate<:T:>(count: __u64): __pointer {
	return !!memory.allocate(size * sizeof(T));
}

//! Allocate a new chunk of memory
public func allocate<:T:>(count: u64): __pointer {
	return !!memory.allocate((size * sizeof(T)).pod);
}

//! Re-allocate a new chunk of memory
#[nosuggest] public func reallocate(ptr: __pointer, oldsize: __u64, newsize: __u64): __pointer {
	return !!memory.realloc(ptr, oldsize, newsize);
}

//! Re-allocate a new chunk of memory
public func reallocate(ptr: __pointer, oldsize: u64, newsize: u64): __pointer {
	return !!memory.realloc(ptr, oldsize.pod, newsize.pod);
}

/*!
** \brief Dispose a chunk of memory
** \param ptr A pointer previously allocated by allocate()
** \param size The size in bytes of the allocated chunk
*/
#[nosuggest] public func dispose(ptr: __pointer, size: __u64) {
	return !!memory.dispose(ptr, size);
}

/*!
** \brief Dispose a chunk of memory
** \param ptr A pointer previously allocated by allocate()
** \param size The size in bytes of the allocated chunk
*/
public func dispose(ptr: __pointer, size: u64) {
	return !!memory.dispose(ptr, size.pod);
}

//! Copy memory regions
public func copy(dst: __pointer, src: __pointer, size: u64) {
	return !!memory.copy(dst, src, size.pod);
}

//! Copy memory regions
public func copy(dst: __pointer, src: __pointer, size: __u64) {
	return !!memory.copy(dst, src, size);
}

//! Copy memory regions which may overlap
public func copyOverlap(dst: __pointer, src: __pointer, size: u64) {
	return !!memory.move(dst, src, size.pod);
}

//! Copy memory regions which may overlap
public func copyOverlap(dst: __pointer, src: __pointer, size: __u64) {
	return !!memory.move(dst, src, size);
}

//! Fill memory with a given pattern
public func fill(ptr: __pointer, size: u64, pattern: u8) {
	return !!memory.fill(ptr, size.pod, pattern.pod);
}

//! Fill memory with a given pattern
#[nosuggest] public func fill(ptr: __pointer, size: __u64, pattern: __u8) {
	return !!memory.fill(ptr, size, pattern);
}

//! Fill memory with a given pattern
public func zero(ptr: __pointer, size: u64) {
	return !!memory.fill(ptr, size.pod, 0__u8);
}

//! Fill memory with a given pattern
#[nosuggest] public func zero(ptr: __pointer, size: __u64) {
	return !!memory.fill(ptr, size, 0__u8);
}

//! Get if 2 memory areas are identical
#[nosuggest] public func equals(p1: __pointer, p2: __pointer, size: __u64): bool {
	return new bool(!!memory.cmp(p1, p2, size) == 0__u32);
}

//! Get if 2 memory areas are identical
public func equals(p1: __pointer, p2: __pointer, size: u64): bool {
	return new bool(!!memory.cmp(p1, p2, size.pod) == 0__u32);
}

#[nosuggest] public func less(p1: __pointer, p2: __pointer, size: __u64): bool {
	return new bool(!!memory.cmp(p1, p2, size) == 2__u32);
}

public func less(p1: __pointer, p2: __pointer, size: u64): bool {
	return new bool(!!memory.cmp(p1, p2, size.pod) == 2__u32);
}

#[nosuggest] public func greater(p1: __pointer, p2: __pointer, size: __u64): bool {
	return new bool(!!memory.cmp(p1, p2, size) == 1__u32);
}

public func greater(p1: __pointer, p2: __pointer, size: u64): bool {
	return new bool(!!memory.cmp(p1, p2, size.pod) == 1__u32);
}
