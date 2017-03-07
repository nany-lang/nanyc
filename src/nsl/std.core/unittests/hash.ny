// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


unittest std.core.hash {
	// u8
	assert(0u == std.hash(0u8));
	assert(42u == std.hash(42u8));
	// u16
	assert(0u == std.hash(0u16));
	assert(42u == std.hash(42u16));
	assert(12345u == std.hash(12345u16));
	// u32
	assert(0u == std.hash(0u));
	assert(42u == std.hash(42u));
	assert(123456u == std.hash(123456u));
	// string
	std.hash("");
	assert(0u != std.hash("hello world\n"));
}
