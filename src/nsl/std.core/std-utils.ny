// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \brief   Extra utilities
/// \ingroup std.core

namespace std;

public func asBuiltin(x);
public func asBuiltin(x: u8):  __u8  -> x.pod;
public func asBuiltin(x: u16): __u16 -> x.pod;
public func asBuiltin(x: u32): __u32 -> x.pod;
public func asBuiltin(x: u64): __u64 -> x.pod;
public func asBuiltin(x: i8):  __i8  -> x.pod;
public func asBuiltin(x: i16): __i16 -> x.pod;
public func asBuiltin(x: i32): __i32 -> x.pod;
public func asBuiltin(x: i64): __i64 -> x.pod;
public func asBuiltin(x: f32): __f32 -> x.pod;
public func asBuiltin(x: f64): __f64 -> x.pod;
public func asBuiltin(x: bool): __bool -> x.pod;
