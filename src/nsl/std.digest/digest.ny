// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.digest;



public func md5(cref str: string): ref string
	-> md5(str.m_cstr, 0__u64 + str.size.pod);


public func md5(ptr: __pointer, size: u64): ref string
	-> md5(ptr, 0__u64 + size.pod);


public func md5(ptr: __pointer, size: __u64): ref string
{
	var p = !!__nanyc_digest_md5(ptr, size);
	return std.memory.nanyc_internal_create_string(p);
}






// -*- mode: nany;-*-
// vim: set filetype=nany:
