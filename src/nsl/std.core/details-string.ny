// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \brief   String implementation
/// \ingroup std.core

namespace std.details.string;




func grow(ref base, newsize: __u32)
{
	var oldcapa = new u32(base.m_capacity);
	var newcapa = oldcapa;
	do
	{
		newcapa = (if newcapa < 64u then 64u
				   else (if newcapa < 4096u then newcapa * 2u else newcapa += 4096u));
	}
	while newcapa < newsize;

	// create a new array
	base.m_capacity = newcapa.pod;
	base.m_cstr = std.memory.reallocate(base.m_cstr, 0u64 + oldcapa, 0u64 + newcapa);
}


func isBlank(cref base): ref bool
{
	var size = new u32(base.m_size);
	if size != 0u32 then
	{
		var i = 0u32;
		var p = base.m_cstr;
		var ascii = new std.Ascii;
		do
		{
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


func getFirst(cref base): ref std.Ascii
{
	return new std.Ascii(if base.m_size != 0__u32 then !!load.u8(base.m_cstr) else 0__u8);
}


func getLast(cref base): ref std.Ascii
{
	var size: __u32 = base.m_size;
	return new std.Ascii(if size != 0__u32 then !!load.u8(base.m_cstr + size - 1__u32) else 0__u8);
}


func contains(cref base, cref ascii: std.Ascii): ref bool
{
	var size = base.m_size;
	if size != 0__u32 then
	{
		var i = 0u;
		var needle = ascii.asU8.pod;
		var p = base.m_cstr;
		do
		{
			if needle == !!load.u8(p + i.pod) then
				return true;
		}
		while (i += 1u) < size;
	}
	return false;

}


func index(cref base, offset: u32, cref ascii: std.Ascii): ref u32
{
	var size = base.m_size;
	if offset < size then
	{
		var p = base.m_cstr + offset.pod;
		var needle = ascii.asU8.pod;
		do
		{
			if needle == !!load.u8(p) then
				return offset;
			offset += 1u;
			p = p + 1__u32;
		}
		while offset < size;
	}
	return new u32(size);
}


func index(ref base, offset: u32, cref predicate): ref u32
{
	var size = base.m_size;
	if offset < size then
	{
		var p = base.m_cstr + offset.pod;
		var ascii = new std.Ascii;
		do
		{
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


func index(cref base, offset: u32, cref needle: string): ref u32
{
	var size = base.m_size;
	var needlesize = needle.size;
	if needlesize != 0u and (offset + needlesize <= size) then
	{
		var cstr = base.m_cstr;
		var needlecstr = needle.m_cstr;
		var maxsize = size - needlesize.pod;
		do
		{
			if maxsize != offset then
			{
				offset = index(offset, needle.at(0u));
				if not (offset <= maxsize) then
					return new u32(size);
			}

			if std.memory.equals(cstr + offset.pod, needlecstr, 0__u64 + needlesize.pod) then
				return offset;

			offset = offset + 1u;
		}
		while offset < maxsize;
	}
	return new u32(size);
}


func lastIndex(cref base, offset: u32, cref ascii: std.Ascii): ref u32
{
	var size = base. m_size;
	if size != 0__u32 then
	{
		if offset.pod >= size then
			offset = size - 1u;

		var p = base.m_cstr + offset.pod;
		var needle = ascii.asU8.pod;
		do
		{
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


func count(cref base, cref ascii: std.Ascii): u32
{
	var c = 0u;
	var size = base.m_size;
	if size != 0__u32 then
	{
		var cstr = base.m_cstr;
		var i = 0u;
		var needle = ascii.asU8.pod;
		do
		{
			if needle == !!load.u8(cstr + i.pod) then
				c += 1u;
		}
		while (i += 1u) < size;
	}
	return c;
}


func chop(ref base, bytes: u32)
{
	if bytes != 0u then
	{
		var size = base.m_size;
		base.m_size = if size >= bytes.pod then size - bytes.pod else 0__u32;
	}
}


func startsWith(cref base, cref prefix: string): ref bool
{
	var psize = prefix.m_size;
	var size = base.m_size;
	return (size != 0__u32 and psize <= size) and std.memory.equals(m_cstr, prefix.m_cstr, 0__u64 + psize);
}


func endsWith(cref base, cref suffix: string): ref bool
{
	var fsize = suffix.m_size;
	var size = base.m_size;
	return (size != 0__u32 and fsize <= size) and std.memory.equals(base.m_cstr + size - fsize, prefix.m_cstr, 0__u64 + fsize);
}


func left(cref base, count: u32): ref string
{
	// TODO: utf8 instead of ascii
	if count.pod < base.m_size then
		return new string(base, count);
	return new string();
}


func trimRight(ref base, cref predicate)
{
	var size = new u32(base.m_size);
	if size != 0u32 then
	{
		var p = m_cstr;
		var ascii = new std.Ascii;
		do
		{
			size -= 1u;
			ascii.asU8 = !!load.u8(p + size.pod);

			if not predicate(ascii) then
			{
				base.m_size = size.pod + 1__u32;
				return;
			}
			if size == 0u32 then
			{
				base.m_size = 0__u32;
				return;
			}
		}
		while __true;
	}
}


func makeViewAscii(cref base, cref filter): ref
{
	ref m_parentString = base;
	ref m_parentFilter = filter;
	return new class
	{
		func cursor: ref
		{
			ref origstr = m_parentString;
			ref accept = m_parentFilter;
			return new class
			{
				func findFirst: bool
					-> (not origstr.empty) and (accept(origstr.at(0u)) or next());

				func next: bool
				{
					do
					{
						m_index += 1u;
						if not (m_index < origstr.size) then
							return false;
					}
					while not accept(origstr.at(m_index));
					return true;
				}

				func get: ref
					-> origstr.at(m_index);

				var m_index = 0u;
			};
		}
	};
}


func makeViewBytes(cref base, cref filter): ref
{
	ref m_parentString = base;
	ref m_parentFilter = filter;
	return new class
	{
		func cursor: ref
		{
			ref origstr = m_parentString;
			ref accept = m_parentFilter;
			return new class
			{
				func findFirst: bool
					-> (not origstr.empty) and (accept(origstr.at(0u).asU8) or next());

				func next: bool
				{
					do
					{
						m_index += 1u;
						if not (m_index < origstr.size) then
							return false;
					}
					while not accept(origstr.at(m_index).asU8);
					return true;
				}

				func get: ref
					-> origstr.at(m_index).asU8;

				var m_index = 0u;
			};
		}
	};
}


func makeViewSplit(cref base, cref filter, separatorLength: u32, cref predicate): ref
{
	ref m_parentString = base;
	ref m_parentFilter = filter;
	ref m_parentPredicate = predicate;
	ref m_parentSepLength = separatorLength;
	return new class
	{
		func cursor: ref
		{
			ref origstr = m_parentString;
			ref accept = m_parentFilter;
			ref predicate = m_parentPredicate;
			ref separatorLength = m_parentSepLength;
			return new class
			{
				func findFirst: bool
					-> next();

				func next: bool
				{
					ref str = origstr;
					if not (m_index < str.size) then
						return false;
					do
					{
						var nextOffset = str.index(m_index, predicate);

						if nextOffset > m_index then
						{
							var distance = nextOffset - m_index;
							m_word.assign(str.m_cstr + m_index.pod, distance.pod);
						}
						else
							m_word.clear();

						m_index = nextOffset + separatorLength;
					}
					while not accept(m_word);
					return true;
				}

				func get: ref
					-> m_word;

				var m_index = 0u;
				var m_word = "";
			};
		}
	};
}


func makeViewSplitByLines(cref base, cref filter): ref
{
	ref m_parentString = base;
	ref m_parentFilter = filter;
	return new class
	{
		func cursor: ref
		{
			ref origstr = m_parentString;
			ref accept = m_parentFilter;
			return new class
			{
				func findFirst: bool
					-> next();

				func next: bool
				{
					ref str = origstr;
					if not (m_index < str.size) then
						return false;
					do
					{
						var nextOffset = str.index(m_index, '\n');

						if nextOffset > m_index then
						{
							var distance = nextOffset - m_index;
							m_word.assign(str.m_cstr + m_index.pod, distance.pod);
							if m_word.last == '\r' then
								m_word.removeLastAscii();
						}
						else
							m_word.clear();

						m_index = nextOffset + 1u;
					}
					while not accept(m_word);
					return true;
				}

				func get: ref
					-> m_word;

				var m_index = 0u;
				var m_word = "";
			};
		}
	};
}


func makeViewIndex(cref base, cref filter, offset: u32, cref pattern): ref
{
	ref m_parentString = base;
	ref m_parentFilter = filter;
	ref m_parentPattern = pattern;
	ref m_parentOffset = offset;
	return new class
	{
		func cursor: ref
		{
			ref origstr = m_parentString;
			ref accept = m_parentFilter;
			ref pattern = m_parentPattern;
			ref startOffset = m_parentOffset;
			return new class
			{
				func findFirst: bool
					-> next();

				func next: bool
				{
					ref str = origstr;
					if not (m_index < str.size) then
						return false;
					do
					{
						m_offset = str.index(m_index, pattern);
						if not (m_offset < str.size) then
							return false;
						m_index = m_offset + 1u;
					}
					while not accept(m_offset);
					return true;
				}

				func get: ref
					-> m_offset;

				var m_index = startOffset;
				var m_offset = 0u;
			};
		}
	};
}






// -*- mode: nany;-*-
// vim: set filetype=nany:
