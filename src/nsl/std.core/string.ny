// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \brief   String implementation
/// \ingroup std.core


//! UTF-8 encoded sequences of characters
public class string {
	//! \name Constructors
	//@{
	operator new;

	operator new (cref value) {
		append(value);
	}

	operator new (cref str: string, size: u32) {
		var clen = str.size;
		append(str.m_cstr, (if size < clen then size else clen));
	}

	operator new (cref str: string, offset: u32) {
		var clen = str.size;
		if offset < clen then
			append(str.m_cstr + offset.pod, clen - offset);
	}

	operator new (cref str: string, size: u32, offset: u32) {
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

	operator new (str: __pointer) {
		appendCString(str);
	}

	operator new (str: __pointer, size: __u32) {
		append(str, size);
	}

	operator new (str: __pointer, cref size: u32) {
		append(str, size.pod);
	}

	operator clone(cref other: string) {
		m_size = 0__u32;
		m_capacity = 0__u32;
		m_cstr = null;
		append(other.m_cstr, other.m_size);
	}

	operator dispose {
		std.memory.dispose(m_cstr, 0__u64 + m_capacity);
	}
	//@}

	//! Empty the string
	func clear {
		m_size = 0__u32;
	}

	//! Get if the string is empty
	var empty
		-> new bool(m_size == 0__u32);

	//! Get if the string is empty or contains only blank characters
	var blank
		-> std.details.string.isBlank(self);

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

	//! Get the internal raw pointer
	var data
		-> m_cstr;

	//! Increase the capacity of the container if necessary
	func reserve(bytes: u32) {
		if m_capacity < size.pod then
			doGrow(size.pod);
	}

	#[nosuggest] func reserve(bytes: __u32) {
		if m_capacity < size then
			doGrow(size);
	}

	func squeeze
		-> doSqueeze();

	//! Assign a new value to the string
	func assign(cref text) {
		m_size = 0__u32;
		append(text);
	}

	#[nosuggest] func assign(str: __pointer, size: __u32) {
		m_size = 0__u32;
		append(str, size);
	}

	//! Extend the string by appending a C-string
	func assign(str: __pointer, cref size: u32) {
		m_size = 0__u32;
		append(str, size.pod);
	}

	//! Extend the string by appending another string
	func append(cref text: string)
		-> append(text.m_cstr, text.m_size);

	func appendCString(str: __pointer)
		-> append(str, !!strlen32(str));

	#[nosuggest] func append(str: __pointer, size: __u32) {
		if size != 0__u32 then {
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
	func append(cref ascii: std.Ascii) {
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

	#[nosuggest] func append(n: __pointer) {
		if n != null then {
			if m_capacity < m_size + 64__u32 then
				doGrow(m_size + 64__u32);
			m_size = m_size + !!__nanyc.string.append.ptr(m_cstr + m_size, n);
		}
		else
			append("0x0");
	}

	#[nosuggest] func append(n: __i8) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i8(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __i16) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i16(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __i32) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i32(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __i64) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.i64(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u8) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u8(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u16) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u16(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u32) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u32(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __u64) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.u64(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __f32) {
		if m_capacity < m_size + 64__u32 then doGrow(m_size + 64__u32);
		m_size = m_size + !!__nanyc.string.append.f32(m_cstr + m_size, n);
	}

	#[nosuggest] func append(n: __f64) {
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


	func prepend(ascii: std.Ascii) {
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


	func insert(offset: __u32, str: __pointer, size: __u32) {
		if size != 0__u32 then {
			assert(str != null);
			var oldsize = m_size;
			if offset < oldsize then {
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

	//! Fill the string with a given pattern
	func fill(cref ascii: std.Ascii) {
		if m_size != 0__u32 then
			std.memory.fill(m_cstr, 0__u64 + m_size, ascii.asU8.pod);
	}


	func join(cref list): ref {
		ref str = "";
		if not list.empty then {
			for i in list do
				str << i << self;
			str.chop(new u32(m_size));
		}
		return str;
	}

	//! Get if the string contains a given ascii
	func contains(cref ascii: std.Ascii): bool
		-> std.details.string.contains(self, ascii);

	func index(cref ascii: std.Ascii): u32
		-> std.details.string.index(self, 0u, ascii);

	func index(offset: u32, cref ascii: std.Ascii): u32
		-> std.details.string.index(self, offset, ascii);

	func index(cref needle: string): u32
		-> std.details.string.index(self, 0u, needle);

	func index(offset: u32, cref needle: string): u32
		-> std.details.string.index(self, offset, needle);

	func index(cref predicate): u32
		-> std.details.string.index(self, 0u, predicate);

	func index(offset: u32, cref predicate): u32
		-> std.details.string.index(self, offset, predicate);

	func lastIndex(cref ascii: std.Ascii): u32
		-> std.details.string.lastIndex(self, new u32(m_size), ascii);

	func lastIndex(offset: u32, cref ascii: std.Ascii): u32
		-> std.details.string.lastIndex(self, offset, ascii);

	//! Get the number of ascii in the string
	func count(cref ascii: std.Ascii): u32
		-> std.details.string.count(self, ascii);

	//! Get the total number of ascii in the string (same as 'size')
	func count: u32
		-> new u32(m_size);

	//! Determines whether the string begins with the characters of another string
	func startsWith(cref prefix: string): bool
		-> std.details.string.startsWith(self, prefix);

	//! Determines whether the string ends with the characters of another string
	func endsWith(cref suffix: string): bool
		-> endsWith(self, suffix);

	//! Remove the 'count' ascii from the end of the string
	func chop(bytes: u32) {
		if bytes != 0u then {
			var size = m_size;
			if size >= bytes.pod then
				m_size = size - bytes.pod;
			else
				m_size = 0__u32;
		}
	}


	//! Remove the 'count' ascii from the begining of the string
	func consume(bytes: u32) {
		if bytes != 0u then {
			var size = m_size;
			if size > bytes.pod then {
				size = size - bytes.pod;
				std.memory.copyOverlap(m_cstr, m_cstr + bytes.pod, 0__u64 + size);
				m_size = size;
			}
			else
				m_size = 0__u32;
		}
	}

	//! Reduce the size of the string
	func truncate(count: u32) {
		if m_size > count.pod then
			m_size = count.pod;
	}

	func resize(bytes: u32) {
		if m_capacity < bytes then
			reserve(bytes);
		m_size = bytes.pod;
	}

	#[nosuggest] func resize(bytes: __u32) {
		if m_capacity < bytes then
			reserve(bytes);
		m_size = bytes;
	}

	//! Remove last ascii if any
	func removeLastAscii {
		var size = m_size;
		if size != 0__u32 then
			m_size = size - 1__u32;
	}

	//! Get a new string with the first N characters
	func left(bytes: u32): ref string
		-> std.details.string.left(self, bytes);

	//! Get the Nth part of the string
	func part(index: u32): ref string
		-> part(index, func (cref ascii) -> ascii.blank);

	//! Get the Nth part of the string
	func part(index: u32, cref separator): ref string {
		for p in self:split(separator) do {
			if index == 0u then
				return p;
			index -= 1u;
		}
		return new string;
	}

	//! Remove whitespace from both sides of the string
	func trim
		-> trim(func (cref ascii) -> ascii.blank);

	//! Remove all ascii matching the predicate from both sides of the string
	func trim(cref predicate) {
		trimRight(predicate);
		trimLeft(predicate);
	}

	//! Remove whitespace from the right side of the string
	func trimRight
		-> trimRight(func (cref ascii) -> ascii.blank);

	//! Remove whitespace from the left side of the string
	func trimLeft
		-> trimLeft(func (cref ascii) -> ascii.blank);

	//! Remove all ascii matching the predicate from the left side of the string
	func trimLeft(cref predicate) {
		var size = new u32(m_size);
		if size != 0u32 then {
			var i = 0u32;
			var p = m_cstr;
			var ascii = new std.Ascii;
			do {
				ascii.asU8 = !!load.u8(p + i.pod);
				if not predicate(ascii) then {
					if i != 0u then {
						m_size = m_size - i.pod;
						std.memory.copyOverlap(m_cstr, m_cstr + i.pod, 0__u64 + m_size);
					}
					return;
				}
				if (i += 1u) == size then {
					m_size = 0__u32;
					return;
				}
			}
			while __true;
		}
	}

	//! Remove all ascii matching the predicate from the right side of the string
	func trimRight(cref predicate) {
		var size = new u32(m_size);
		if size != 0u32 then {
			var p = m_cstr;
			var ascii = new std.Ascii;
			do {
				size -= 1u;
				ascii.asU8 = !!load.u8(p + size.pod);
				if not predicate(ascii) then {
					m_size = size.pod + 1__u32;
					return;
				}
				if size == 0u32 then {
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
		-> std.details.string.makeTrimmed(self);

	//! Get the ascii at offset 'i' (without any check)
	func at(cref i: u32): ref std.Ascii {
		assert(i < m_size);
		return new std.Ascii(!!load.u8(m_cstr + i.pod));
	}

	func adopt(ptr: __pointer, size: __u32, capacity: __u32) {
		std.memory.dispose(m_cstr, 0__u64 + m_capacity);
		m_cstr = ptr;
		m_size = size;
		m_capacity = capacity;
	}

	/*!
	** \brief View on each UTF8 character
	** \TODO UTF8 support (UTF8cpp?)
	*/
	view (ref filter): ref
		-> std.details.string.makeViewAscii(self, filter);

	//! View on each ascii
	view ascii(ref filter): ref
		-> std.details.string.makeViewAscii(self, filter);

	//! View on each ascii as u8
	view bytes(ref filter): ref
		-> std.details.string.makeViewBytes(self, filter);

	//! Split the string
	view split(ref filter): ref
		-> std.details.string.makeViewSplit(self, filter, 1u, func (cref ascii) -> ascii.blank);

	//! Split the string
	view split(ref filter, cref separator: std.Ascii): ref {
		ref sep = separator;
		return std.details.string.makeViewSplit(self, filter, 1u, func (cref ascii) -> ascii == sep);
	}

	//! Split the string
	view split(ref filter, ref pattern: string): ref
		-> std.details.string.makeViewSplit(self, filter, pattern.size, pattern);

	//! Split the string
	view split(ref filter, ref predicate): ref
		-> std.details.string.makeViewSplit(self, filter, 1u, predicate);

	//! Split the string
	view lines(ref filter): ref
		-> std.details.string.makeViewSplitByLines(self, filter);

	//! Find occurences
	view index(ref filter, cref pattern): ref
		-> std.details.string.makeViewIndex(filter, 0u, pattern);

	//! Find occurences
	view index(ref filter, offset: u32, cref pattern): ref
		-> std.details.string.makeViewIndex(filter, offset, pattern);

	/*!
	** \brief Extend the string by appending a value (see 'append')
	** \return self
	*/
	operator += (cref n): ref string {
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
	func doGrow(newsize: __u32) {
		var oldcapa = new u32(m_capacity);
		var newcapa = oldcapa;
		do {
			newcapa = (if newcapa < 64u then 64u
				else (if newcapa < 4096u then newcapa * 2u else newcapa += 4096u));
		}
		while newcapa < newsize;
		// create a new array
		m_capacity = newcapa.pod;
		m_cstr = std.memory.reallocate(m_cstr, 0u64 + oldcapa, 0u64 + newcapa);
	}

	func doSqueeze {
		if m_size == 0__u32 then {
			std.memory.dispose(m_cstr, 0__u64 + m_capacity);
			m_capacity = 0__u32;
			m_cstr = null;
		}
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
