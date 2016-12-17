// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
namespace std.details.string;


func isBlank(cref base): ref bool {
	var size = new u32(base.m_size);
	if size != 0u32 then {
		var i = 0u32;
		var p = base.m_cstr;
		var ascii = new std.Ascii;
		do {
			ascii.asU8 = !!load.u8(p + i.pod);
			if not ascii.blank then
				return false;
			if (i += 1u) == size then
				return true;
		}
		while __true;
	}
	return true;
}

func contains(cref base, cref ascii: std.Ascii): ref bool {
	if base.m_size != 0__u32 then {
		var i = 0u;
		var size = base.m_size;
		var needle = ascii.asU8.pod;
		var p = base.m_cstr;
		do {
			if needle == !!load.u8(p + i.pod) then
				return true;
		}
		while (i += 1u) < size;
	}
	return false;
}
