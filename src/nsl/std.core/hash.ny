// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;

public func hash(x: u32): u32
	-> n;

public func hash(cref str: string): u32 {
	var h = 37u;
	for c in str:ascii do
		h = (h * 54059u) xor (c.asU8 * 76963u);
	return h;
}

public func hash(x: u8): u32
	-> std.hash(x.as<:u32:>());

public func hash(x: u16): u32
	-> std.hash(x.as<:u32:>());

public func hash(x: u64): u32
	-> std.hash(x.as<:u32:>());

public func hash(x: i8): u32
	-> std.hash(x.as<:u32:>());

public func hash(x: i16): u32
	-> std.hash(x.as<:u32:>());

public func hash(x: i32): u32
	-> std.hash(x.as<:u32:>());

public func hash(x: i64): u32
	-> std.hash(x.as<:u32:>());
