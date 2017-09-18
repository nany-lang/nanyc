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
public typedef uint8_t: u8;

//! Unsigned integer with width of exactly 16 bits
public typedef uint16_t: u16;

//! Unsigned integer with width of exactly 32 bits
public typedef uint32_t: u32;

//! Unsigned integer with width of exactly 64 bits
public typedef uint64_t: u64;

//! Signed integer with width of exactly 8 bits
public typedef int8_t: i8;

//! Signed integer with width of exactly 16 bits
public typedef int16_t: i16;

//! Signed integer with width of exactly 32 bits
public typedef int32_t: i32;

//! Signed integer with width of exactly 64 bits
public typedef int64_t: i64;

//! Floating Point Data Type single precision (32 bits, -3.4E38 .. 3.4E38)
public typedef float: f32;

//! Floating Point Data Type single precision (64 bits, -1.7E308 .. 1.7E308)
public typedef double: f64;

//! Alias for the standard C ssize_t
public typedef ssize_t: typeof(!!__nanyc_type_ssize_t());

//! Alias for the standard C size_t
public typedef size_t: typeof(!!__nanyc_type_size_t());

//! Alias for the standard C ptrdiff_t
// public typedef ptrdiff_t: __i64;

//! Alias for the standard C intptr_t
// public typedef intptr_t: __i64;


//! Pointer
public typedef ptr: __pointer;
