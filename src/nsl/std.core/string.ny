// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \brief   String implementation
/// \ingroup std.core



/*!
** \brief UTF-8 encoded sequences of characters
*/
public class string
{
	//! \name Constructors
	//@{
	operator new;

	operator new (cref value)
	{
		append(value);
	}

	operator new (cref str: string, size: u32)
	{
		var clen = str.size;
		append(str.m_cstr, (if size < clen then size else clen));
	}

	operator new (cref str: string, offset: u32)
	{
		var clen = str.size;
		if offset < clen then
			append(str.m_cstr + offset.pod, clen - offset);
	}

	operator new (cref str: string, size: u32, offset: u32)
	{
		var clen = str.size;
		if offset < clen then
		{
			size += offset;
			if size > clen then
				size = clen;
			size -= offset;
			append(str.m_cstr + offset.pod, size);
		}
	}

	operator new (str: __pointer)
	{
		appendCString(str);
	}

	operator new (str: __pointer, size: __u32)
	{
		append(str, size);
	}

	operator new (str: __pointer, cref size: u32)
	{
		append(str, size.pod);
	}

	operator clone(cref rhs: string)
	{
		 append(rhs);
	}

	operator dispose
	{
		std.memory.dispose(m_cstr, 0__u64 + m_capacity);
	}
	//@}


	//! Empty the string
	func clear
	{
		m_size = 0__u32;
	}


	//! Get if the string is empty
	var empty
		-> new bool(m_size == 0__u32);

	//! Get if the string is empty or contains only blank characters
	var blank
		-> isBlank();

	//! Get the size of the string (in bytes)
	var size
		-> new u32(m_size);

	//! Get the capacity of the string (in bytes)
	var capacity
		-> new u32(m_capacity);


	//! Get the first character, \0 if empty
	var first
		-> new std.Ascii(if m_size != 0__u32 then !!load.u8(m_cstr) else 0__u8);

	//! Get the last character, \0 if empty
	var last
		-> new std.Ascii(if m_size != 0__u32 then !!load.u8(m_cstr + m_size - 1__u32) else 0__u8);


	/*!
	** \brief Increase the capacity of the container if necessary
	*/
	func reserve(size: u32)
	{
		if m_capacity < size.pod then
			doGrow(size.pod);
	}

	/*!
	** \brief Increase the capacity of the container if necessary
	*/
	#[nosuggest] func reserve(size: __u32)
	{
		if m_capacity < size then
			doGrow(size);
	}

	func squeeze
		-> doSqueeze();


	//! Assign a new value to the string
	func assign(cref text)
	{
		m_size = 0__u32;
		append(text);
	}

	#[nosuggest] func assign(str: __pointer, size: __u32)
	{
		m_size = 0__u32;
		append(str, size);
	}

	//! Extend the string by appending a C-string
	func assign(str: __pointer, cref size: u32)
	{
		m_size = 0__u32;
		append(str, size.pod);
	}

	//! Extend the string by appending another string
	func append(cref text: string)
		-> append(text.m_cstr, text.m_size);

	func appendCString(str: __pointer)
		-> append(str, std.memory.strlen(str));


	//! Extend the string by appending a C-string
	#[nosuggest] func append(str: __pointer, size: __u32)
	{
		if size != 0__u32 then
		{
			assert(str != null);
			var oldsize = m_size;
			var newsize = oldsize + size;
			if m_capacity < newsize then
				doGrow(newsize);
			std.memory.copy(m_cstr + oldsize, str, 0__u64 + size);
			m_size = newsize;
		}
	}

	//! Extend the string by appending a C-string
	func append(str: __pointer, cref size: u32)
		-> append(str, size.pod);

	//! Extend the string by appending an ascii
	func append(cref ascii: std.Ascii)
	{
		var oldsize = m_size;
		var newsize = oldsize + 1__u32;
		if m_capacity < newsize then
			doGrow(newsize);
		!!store.u8(m_cstr + oldsize, ascii.asU8.pod);
		m_size = newsize;
	}

	func append(cref n: i8)
		-> append(n.pod);

	func append(cref n: i16)
		-> append(n.pod);

	func append(cref n: i32)
		-> append(n.pod);

	func append(cref n: i64)
		-> append(n.pod);

	func append(cref n: u8)
		-> append(n.pod);

	func append(cref n: u16)
		-> append(n.pod);

	func append(cref n: u32)
		-> append(n.pod);

	func append(cref n: u64)
		-> append(n.pod);

	func append(cref n: f32)
		-> append(n.pod);

	func append(cref n: f64)
		-> append(n.pod);

	func append(cref n: bool)
		-> append(n.pod);


	//	#[nosuggest] func append(n: void)
	//		-> append("<void>");

	#[nosuggest] func append(n: __pointer)
	{
		if n != null then
		{
			if m_capacity < m_size + 64__u32 then
				doGrow(m_size + 64__u32);
			m_size = m_size + !!__nanyc.string.append.ptr(m_cstr + m_size, n);
		}
		else
			append("0x0");
	}

	#[nosuggest] func append(n: __i8)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i8(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __i16)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i16(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __i32)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i32(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __i64)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i64(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u8)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u8(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u16)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u16(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u32)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u32(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u64)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u64(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __f32)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.f32(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __f64)
	{
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.f64(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __bool)
		-> append(if n then "true" else "false");


	func prepend(cref str: string)
		-> insert(0__u32, str.m_cstr, str.m_size);

	#[nosuggest] func prepend(n: __bool)
		-> insert(0u, if n then "true" else "false");

	func prepend(n: bool)
		-> prepend(n.pod);

	#[nosuggest] func prepend(str: __pointer, size: __u32)
		-> insert(0__u32, str, size);


	func prepend(ascii: std.Ascii)
	{
		var oldsize = m_size;
		var newsize = oldsize + 1__u32;
		if m_capacity < newsize then
			doGrow(newsize);

		var p = m_cstr;
		if oldsize != 0__u32 then
			std.memory.copyOverlap(p + 1__u32, p, 1__u64);
		!!store.u8(p, ascii.asU8.pod);
		m_size = newsize;
	}


	func insert(offset: u32, cref str: string)
		-> insert(offset.pod, str.m_cstr, str.m_size);

	func insert(offset: u32, cref str: string, size: u32)
		-> insert(offset.pod, str.m_cstr, (if str.size < size then str.size else size));

	func insert(offset: u32, cref value)
		-> insert(offset, ((new string) += value));

	func insert(offset: u32, cref value, size: u32)
		-> insert(offset, ((new string) += value), size);


	func insert(offset: __u32, str: __pointer, size: __u32)
	{
		if size != 0__u32 then
		{
			assert(str != null);
			var oldsize = m_size;

			if offset < oldsize then
			{
				var newsize = oldsize + size;
				if m_capacity < newsize then
					doGrow(newsize);

				var p = m_cstr + offset;
				std.memory.copyOverlap(dst: p + size, src: p, size: 0__u64 + (oldsize - offset));
				std.memory.copy(dst: p, src: str, size: 0__u64 + size);
				m_size = newsize;
			}
			else
				append(str, size);
		}
	}


	/*!
	** \brief Fill the string with a given pattern
	*/
	func fill(cref ascii: std.Ascii)
	{
		if m_size != 0__u32 then
			std.memory.fill(m_cstr, 0__u64 + m_size, ascii.asU8.pod);
	}


	/*!
	** \brief Get if the string contains a given ascii
	*/
	func contains(cref ascii: std.Ascii): bool
	{
		if m_size != 0__u32 then
		{
			var i = 0u;
			var size = m_size;
			var needle = ascii.asU8.pod;
			var p = m_cstr;
			do
			{
				if needle == !!load.u8(p + i.pod) then
					return true;
			}
			while (i += 1u) < size;
		}
		return false;
	}


	func index(cref predicate): u32
		-> index(0u, predicate);


	func index(offset: u32, cref predicate): u32
	{
		var size = m_size;
		if offset < size then
		{
			var p = m_cstr + offset.pod;
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


	func index(cref ascii: std.Ascii): u32
		-> index(0u, ascii);


	func index(offset: u32, cref ascii: std.Ascii): u32
	{
		var size = m_size;
		if offset < size then
		{
			var p = m_cstr + offset.pod;
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


	func index(cref needle: string): u32
		-> index(0u, needle);


	func index(offset: u32, cref needle: string): u32
	{
		var size = m_size;
		var needlesize = needle.size;
		if needlesize != 0u and (offset + needlesize <= size) then
		{
			var maxsize = size - needlesize.pod;
			do
			{
				if maxsize != offset then
				{
					offset = index(offset, needle.at(0u));
					if not (offset <= maxsize) then
						return new u32(size);
				}

				if std.memory.equals(m_cstr + offset.pod, needle.m_cstr, 0__u64 + needlesize.pod) then
					return offset;

				offset = offset + 1u;
			}
			while offset < maxsize;
		}
		return new u32(size);
	}


	func lastIndex(cref ascii: std.Ascii): u32
		-> lastIndex(new u32(m_size), ascii);

	func lastIndex(offset: u32, cref ascii: std.Ascii): u32
	{
		var size = m_size;
		if size != 0__u32 then
		{
			if offset.pod >= size then
				offset = size - 1u;

			var p = m_cstr + offset.pod;
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


	/*!
	** \brief Get the number of ascii in the string
	*/
	func count(cref ascii: std.Ascii): u32
	{
		var c = 0u;
		if m_size != 0__u32 then
		{
			var i = 0u;
			var size = m_size;
			var needle = ascii.asU8.pod;
			do
			{
				if needle == !!load.u8(m_cstr + i.pod) then
					c += 1u;
			}
			while (i += 1u) < size;
		}
		return c;
	}

	/*!
	** \brief Get the total number of ascii in the string (same as 'size')
	*/
	func count: u32
		-> new u32(m_size);


	/*!
	** \brief Determines whether the string begins with the characters of another string
	*/
	func startsWith(cref prefix: string): bool
	{
		return (m_size != 0__u32 and prefix.m_size <= m_size)
			and std.memory.equals(m_cstr, prefix.m_cstr, 0__u64 + prefix.m_size);
	}


	/*!
	** \brief Determines whether the string ends with the characters of another string
	*/
	func endsWith(cref suffix: string): bool
	{
		var fsize = suffix.m_size;
		return (m_size != 0__u32 and fsize <= m_size)
			and std.memory.equals(m_cstr + m_size - fsize, prefix.m_cstr, 0__u64 + fsize);
	}


	/*!
	** \brief Remove the 'count' ascii from the end of the string
	*/
	func chop(ascii: u32)
	{
		if ascii != 0u then
		{
			var size = m_size;
			if size >= ascii.pod then
				m_size = size - ascii.pod;
			else
				m_size = 0__u32;
		}
	}


	/*!
	** \brief Remove the 'count' ascii from the begining of the string
	*/
	func consume(ascii: u32)
	{
		if ascii != 0u then
		{
			var size = m_size;
			if size > ascii.pod then
			{
				size = size - ascii.pod;
				std.memory.copyOverlap(m_cstr, m_cstr + ascii.pod, 0__u64 + size);
				m_size = size;
			}
			else
				m_size = 0__u32;
		}
	}


	/*!
	** \brief Reduce the size of the string
	*/
	func truncate(count: u32)
	{
		if m_size > count.pod then
			m_size = count.pod;
	}


	func resize(count: u32)
	{
		if m_capacity < count then
			reserve(count);
		m_size = count.pod;
	}

	#[nosuggest] func resize(count: __u32)
	{
		if m_capacity < count then
			reserve(count);
		m_size = count;
	}


	/*!
	** \brief Remove last ascii if any
	*/
	func removeLastAscii
	{
		var size = m_size;
		if size != 0__u32 then
			m_size = size - 1__u32;
	}


	/*!
	** \brief Get a new string with the first N characters
	*/
	func left(count: u32): ref string
	{
		// TODO: utf8 instead of ascii
		if count.pod < m_size then
			return new string(self, count);
		return new string();
	}

	/*!
	** \brief Get the Nth part of the string
	*/
	func part(index: u32): ref string
		-> part(index, func (cref ascii) -> ascii.blank);

	/*!
	** \brief Get the Nth part of the string
	*/
	func part(index: u32, cref separator): ref string
	{
		for p in self:split(separator) do
		{
			if index == 0u then
				return p;
			index -= 1u;
		}
		return new string;
	}



	/*!
	** \brief Remove whitespace from both sides of the string
	*/
	func trim
		-> trim(func (cref ascii) -> ascii.blank);

	/*!
	** \brief Remove all ascii matching the predicate from both sides of the string
	*/
	func trim(cref predicate)
	{
		trimRight(predicate);
		trimLeft(predicate);
	}


	/*!
	** \brief Remove whitespace from the right side of the string
	*/
	func trimRight
		-> trimRight(func (cref ascii) -> ascii.blank);


	/*!
	** \brief Remove whitespace from the left side of the string
	*/
	func trimLeft
		-> trimLeft(func (cref ascii) -> ascii.blank);


	/*!
	** \brief Remove all ascii matching the predicate from the left side of the string
	*/
	func trimLeft(cref predicate)
	{
		var size = new u32(m_size);
		if size != 0u32 then
		{
			var i = 0u32;
			var p = m_cstr;
			var ascii = new std.Ascii;
			do
			{
				ascii.asU8 = !!load.u8(p + i.pod);

				if not predicate(ascii) then
				{
					if i != 0u then
					{
						m_size = m_size - i.pod;
						std.memory.copyOverlap(m_cstr, m_cstr + i.pod, 0__u64 + m_size);
					}
					return;
				}

				if (i += 1u) == size then
				{
					m_size = 0__u32;
					return;
				}
			}
			while __true;
		}
	}

	/*!
	** \brief Remove all ascii matching the predicate from the right side of the string
	*/
	func trimRight(cref predicate)
	{
		var size = new u32(m_size);
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
					m_size = size.pod + 1__u32;
					return;
				}
				if size == 0u32 then
				{
					m_size = 0__u32;
					return;
				}
			}
			while __true;
		}
	}


	/*!
	** \brief Get a string that has whitespace removed from the start and the end
	** \see blank
	*/
	var trimmed
		-> makeTrimmed();


	/*!
	** \brief Get the ascii at offset 'i' (without any check)
	*/
	func at(cref i: u32): ref std.Ascii
	{
		assert(i < m_size);
		return new std.Ascii(!!load.u8(m_cstr + i.pod));
	}


	func adopt(ptr: __pointer, size: __u32, capacity: __u32)
	{
		std.memory.dispose(m_cstr, 0__u64 + m_capacity);
		m_cstr = ptr;
		m_size = size;
		m_capacity = capacity;
		print("adopt: \(ptr), size: \(size), capa: \(capacity)\n");
	}

	/*!
	** \brief View on each UTF8 character
	** \TODO UTF8 support (UTF8cpp?)
	*/
	view (cref filter): ref
		-> makeViewAscii(filter);

	/*!
	** \brief View on each ascii
	*/
	view ascii(cref filter): ref
		-> makeViewAscii(filter);

	/*!
	** \brief View on each ascii as u8
	*/
	view bytes(cref filter): ref
		-> makeViewBytes(filter);

	/*!
	** \brief Split the string
	*/
	view split(cref filter): ref
		-> makeViewSplit(filter, 1u, func (cref ascii) -> ascii.blank);

	/*!
	** \brief Split the string
	*/
	view split(cref filter, cref separator: std.Ascii): ref
	{
		ref sep = separator;
		return makeViewSplit(filter, 1u, func (cref ascii) -> ascii == sep);
	}

	/*!
	** \brief Split the string
	*/
	view split(cref filter, cref pattern: string): ref
		-> makeViewSplit(filter, pattern.size, pattern);

	/*!
	** \brief Split the string
	*/
	view split(cref filter, cref predicate): ref
		-> makeViewSplit(filter, 1u, predicate);

	/*!
	** \brief Split the string
	*/
	view lines(cref filter): ref
		-> makeViewSplitByLines(filter);

	/*!
	** \brief Find occurences
	*/
	view index(cref filter, cref pattern): ref
		-> makeViewIndex(filter, 0u, pattern);

	/*!
	** \brief Find occurences
	*/
	view index(cref filter, offset: u32, cref pattern): ref
		-> makeViewIndex(filter, offset, pattern);


	/*!
	** \brief Extend the string by appending a value (see 'append')
	** \return self
	*/
	operator += (cref n): ref string
	{
		append(n);
		return self;
	}


	/*!
	** \brief Get the ascii at offset 'i' ('\0' if 'i' is out of bound)
	*/
	operator [] (cref i: u32)
		-> new std.Ascii(if i < m_size then !!load.u8(m_cstr + i.pod) else 0__u8);



private:
	//! Increase the inner storage
	func doGrow(newsize: __u32)
	{
		var oldcapa = new u32(m_capacity);
		var newcapa = oldcapa;
		do
		{
			newcapa = (if newcapa < 64u then 64u
				else (if newcapa < 4096u then newcapa * 2u else newcapa += 4096u));
		}
		while newcapa < newsize;

		// create a new array
		m_capacity = newcapa.pod;
		m_cstr = std.memory.reallocate(m_cstr, 0u64 + oldcapa, 0u64 + newcapa);
	}


	func doSqueeze
	{
		if m_size == 0__u32 then
		{
			std.memory.dispose(m_cstr, 0__u64 + m_capacity);
			m_capacity = 0__u32;
			m_cstr = null;
		}
	}


	func isBlank: bool
	{
		var size = new u32(m_size);
		if size != 0u32 then
		{
			var i = 0u32;
			var p = m_cstr;
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


	func makeTrimmed: ref
	{
		var newstr = new string(self);
		newstr.trim();
		return newstr;
	}


	func makeViewAscii(cref filter): ref
	{
		ref m_parentString = self;
		ref m_parentFilter = filter;
		return new class {
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


	func makeViewBytes(cref filter): ref
	{
		ref m_parentString = self;
		ref m_parentFilter = filter;
		return new class {
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


	func makeViewSplit(cref filter, separatorLength: u32, cref predicate): ref
	{
		ref m_parentString = self;
		ref m_parentFilter = filter;
		ref m_parentPredicate = predicate;
		ref m_parentSepLength = separatorLength;
		return new class {
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


	func makeViewSplitByLines(cref filter): ref
	{
		ref m_parentString = self;
		ref m_parentFilter = filter;
		return new class {
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


	func makeViewIndex(cref filter, offset: u32, cref pattern): ref
	{
		ref m_parentString = self;
		ref m_parentFilter = filter;
		ref m_parentPattern = pattern;
		ref m_parentOffset = offset;
		return new class {
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




internal:
	// note: u32 by default to have a consistent behavior accross all operating systems
	// note: currently `__u32` instead of `u32` due to the lack of optimizations by the compiler

	//! The current size of the container
	var m_size = 0__u32;
	//! The current capacity of the container
	var m_capacity = 0__u32;
	//! Contents, not zero-terminated
	var m_cstr: __pointer = null;

} // class string






public operator + (cref s1: string, cref s2: string): ref string
	-> ((new string(s1)) += s2);

public operator + (cref s1: string, cref s2: std.Ascii): ref string
	-> ((new string(s1)) += s2);

public operator + (cref s1: std.Ascii, cref s2: string): ref string
	-> ((new string(s1)) += s2);


public operator == (cref s1: string, cref s2: string): bool
	-> (s1.size == s2.size)
	and (s1.empty or std.memory.equals(s1.m_cstr, s2.m_cstr, 0__u64 + s1.m_size));

public operator != (cref s1: string, cref s2: string): bool
	-> (s1.size != s2.size)
	or (not s1.empty and not std.memory.equals(s1.m_cstr, s2.m_cstr, 0__u64 + s1.m_size));


public operator == (cref s1: string, cref s2: std.Ascii): bool
	-> s1.size == 1u and s1.at(0u) == s2;

public operator == (cref s1: std.Ascii, cref s2: string): bool
	-> s2.size == 1u and s2.at(0u) == s1;

public operator != (cref s1: string, cref s2: std.Ascii): bool
	-> not (s1.size == 1u and s1.at(0u) == s2);

public operator != (cref s1: std.Ascii, cref s2: string): bool
	-> not (s2.size == 1u and s2.at(0u) == s1);


public operator << (ref s: string, cref value): ref string
	-> s += value;






// -*- mode: nany;-*-
// vim: set filetype=nany:
