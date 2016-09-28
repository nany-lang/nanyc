// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
//! \file    ctypes.ny
//! \ingroup std.c

namespace std.c;



//! Unsigned integer with width of exactly 8 bits
public typedef u8: __u8;

//! Unsigned integer with width of exactly 16 bits
public typedef u16: __u16;

//! Unsigned integer with width of exactly 32 bits
public typedef u32: __u32;

//! Unsigned integer with width of exactly 64 bits
public typedef u64: __u64;


//! Signed integer with width of exactly 8 bits
public typedef i8: __i8;

//! Signed integer with width of exactly 16 bits
public typedef i16: __i16;

//! Signed integer with width of exactly 32 bits
public typedef i32: __i32;

//! Signed integer with width of exactly 64 bits
public typedef i64: __i64;



//! Alias for the standard C ssize_t
public typedef ssize_t: __i64;

//! Alias for the standard C size_t
public typedef size_t: __u64;

//! Alias for the standard C ptrdiff_t
public typedef ptrdiff_t: __i64;

//! Alias for the standard C intptr_t
public typedef intptr_t: __i64;



//! Boolean
public typedef bool: __bool;

//! Pointer
public typedef ptr: __pointer;
