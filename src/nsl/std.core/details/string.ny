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

func index(cref base, offset: u32, cref needle: string): u32 {
	var size = base.m_size;
	var needlesize = needle.size;
	if needlesize != 0u and (offset + needlesize <= size) then {
		var maxsize = size - needlesize.pod;
		do {
			if maxsize != offset then {
				offset = index(offset, needle.at(0u));
				if not (offset <= maxsize) then
					return new u32(size);
			}
			if std.memory.equals(base.m_cstr + offset.pod, needle.m_cstr, 0__u64 + needlesize.pod) then
				return offset;
			offset = offset + 1u;
		}
		while offset < maxsize;
	}
	return new u32(size);
}

func index(cref base, offset: u32, cref ascii: std.Ascii): u32 {
	var size = base.m_size;
	if offset < size then {
		var p = base.m_cstr + offset.pod;
		var needle = ascii.asU8.pod;
		do {
			if needle == !!load.u8(p) then
				return offset;
			offset += 1u;
			p = p + 1__u32;
		}
		while offset < size;
	}
	return new u32(size);
}

func index(cref base, offset: u32, cref predicate): u32 {
	var size = base.m_size;
	if offset < size then {
		var p = base.m_cstr + offset.pod;
		var ascii = new std.Ascii;
		do {
			ascii.asU8 = !!load.u8(p);
			if predicate(ascii) then
				return offset;
			offset += 1u;
			p = p + 1__u32;
		}
		while offset < size;
	}
	return new u32(size);
}

func lastIndex(cref base, offset: u32, cref ascii: std.Ascii): u32 {
	var size = base.m_size;
	if size != 0__u32 then {
		if offset.pod >= size then
			offset = size - 1u;

		var p = base.m_cstr + offset.pod;
		var needle = ascii.asU8.pod;
		do {
			if needle == !!load.u8(p) then
				return offset;
			if offset == 0u then
				return new u32(size); // break
			offset -= 1u;
			p = p - 1__u32;
		}
		while true;
	}
	return new u32(size);
}

func count(cref base, cref ascii: std.Ascii): u32 {
	var c = 0u;
	if base.m_size != 0__u32 then {
		var i = 0u;
		var size = base.m_size;
		var needle = ascii.asU8.pod;
		do {
			if needle == !!load.u8(base.m_cstr + i.pod) then
				c += 1u;
		}
		while (i += 1u) < size;
	}
	return c;
}

func startsWith(cref base, cref prefix: string): ref bool {
	return (base.m_size != 0__u32 and prefix.m_size <= base.m_size)
		and std.memory.equals(base.m_cstr, prefix.m_cstr, 0__u64 + prefix.m_size);
}

func endsWith(cref base, cref suffix: string): bool {
	var fsize = suffix.m_size;
	return (base.m_size != 0__u32 and fsize <= base.m_size)
		and std.memory.equals(base.m_cstr + m_size - fsize, prefix.m_cstr, 0__u64 + fsize);
}

func left(cref base, bytes: u32): ref
	-> if bytes.pod < m_size then new string(self, bytes) else new string();
