// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;


public func hash(n: u8): u32
	-> 0u + n;


public func hash(n: u16): u32
	-> 0u + n;


public func hash(n: u32): u32
	-> n;


public func hash(flag: bool): u32
	-> if flag then 1231u else 1237u;


public func hash(cref str: string): u32
	-> std.digest.xxhash32(str.data, str.size);
