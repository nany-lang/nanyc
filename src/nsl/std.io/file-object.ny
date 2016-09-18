// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.io;




public class File
{
public:
	operator new;

	operator new(cref ro: string)
	{
		doOpen(ro, __true, __false, __false, __false);
	}

	operator new(cref rw: string)
	{
		doOpen(rw, __true, __true, __false, __false);
	}

	operator new(cref rw: string, truncate: bool)
	{
		doOpen(rw, __true, __true, __false, truncate.pod);
	}

	operator new(cref rw: string, append: bool)
	{
		doOpen(rw, __true, __true, append.pod, __false);
	}

	operator new(cref wo: string)
	{
		doOpen(wo, __false, __true, __false, __false);
	}

	operator new(cref wo: string, truncate: bool)
	{
		doOpen(wo, __false, __true, __false, truncate.pod);
	}

	operator new(cref wo: string, append: bool)
	{
		doOpen(wo, __false, __true, append.pod, __false);
	}


	operator dispose
	{
		if m_fd != null then
			!!__nanyc_io_file_close(m_fd);
	}


	func open(cref ro: string): ref bool
		-> doOpen(ro, __true, __false, __false, __false);


	func open(cref rw: string): ref bool
		-> doOpen(rw, __true, __true, __false, __false);

	func open(cref rw: string, truncate: bool): ref bool
		-> doOpen(rw, __true, __true, __false, truncate.pod);

	func open(cref rw: string, append: bool): ref bool
		-> doOpen(rw, __true, __true, append.pod, __false);


	func open(cref wo: string): ref bool
		-> doOpen(wo, __false, __true, __false, __false);

	func open(cref wo: string, truncate: bool): ref bool
		-> doOpen(wo, __true, __true, __false, truncate.pod);

	func open(cref wo: string, append: bool): ref bool
		-> doOpen(wo, __true, __true, append.pod, __false);


	func close
	{
		if m_fd != null then
		{
			!!__nanyc_io_file_close(m_fd);
			m_fd = null;
		}
	}


	var opened
		-> new bool(m_fd != null);


	var eof
		-> new bool(if m_fd != null then !!__nanyc_io_file_eof(m_fd) else __false);

	var tell
		-> new i64(if m_fd != null then !!__nanyc_io_file_tell(m_fd) else 0__i64);


	func seek(set: u64)
	{
		if m_fd != null then
			!!__nanyc_io_file_seek(m_fd, set.pod);
	}

	func seek(end: i64)
	{
		if m_fd != null then
			!!__nanyc_io_file_seek_from_end(m_fd, end.pod);
	}

	func seek(current: i64)
	{
		if m_fd != null then
			!!__nanyc_io_file_seek_cur(m_fd, current.pod);
	}


	func rewind
		-> seek(set: 0u64);


	func flush
	{
		if m_fd != null then
			!!__nanyc_io_file_flush(m_fd);
	}


	func read(buffer: __pointer, size: __u64): __u64
	{
		return if m_fd != null
			then !!__nanyc_io_file_read(m_fd, buffer, size)
			else 0__u64;
	}

	func readline: ref string
		-> readline(256u, 2u * 1024u * 1024u * 1024u);

	func readline(chunk: u32): ref string
		-> readline(chunk, 2u * 1024u * 1024u * 1024u);


	func readline(chunk: u32, limit: u32): ref string
	{
		ref str = new string;
		if m_fd == null then
			return str;

		var capacity = chunk;
		var offset = 0u;
		do
		{
			str.reserve((capacity += chunk));
			var numread = read(str.m_cstr + offset, 0__u64 + chunk.pod);
			if numread == 0__u64 then
				return str;

			str.resize(offset + !!__reinterpret(numread, #[__nanyc_synthetic] __u32));
			var linefeed = str.index(offset, '\n');
			if linefeed < str.size then
			{
				str.resize(linefeed);
				if str.last == '\r' then
					str.removeLastAscii();
				// TODO fix signed / unsigned convertions...
				var oldcursor = !!__nanyc_io_file_tell(m_fd);
				var cursor = !!__nanyc_io_file_tell(m_fd) - numread + linefeed.pod + 1__u32;
				print("... reposition: str: \(str.size), from \(oldcursor) to \(cursor)\n");
				!!__nanyc_io_file_seek(m_fd, cursor);
				return str;
			}

			offset = offset + chunk.pod;
			capacity += chunk;
		}
		while capacity < limit;
		return str;
	}


	func write(buffer: __pointer, size: __u32): __u32
	{
		return if m_fd != null
			then !!__nanyc_io_file_write(m_fd, buffer, size)
			else 0__u32;
	}


	func write(cref str: string): u32
		-> new u32(str.m_cstr, str.size.pod);


	view (cref filter)
		-> makeViewLineByLine(filter, 256u, 2u * 1024u * 1024u * 1024u);


	view lines(cref filter)
		-> makeViewLineByLine(filter, 256u, 2u * 1024u * 1024u * 1024u);


	view lines(cref filter, chunk: u32)
		-> makeViewLineByLine(filter, chunk, 2u * 1024u * 1024u * 1024u);


	view lines(cref filter, chunk: u32, limit: u32)
		-> makeViewLineByLine(filter, chunk, limit);


	operator += (cref str: string): ref File
	{
		write(str);
		return self;
	}


private:
	func doOpen(cref filename: string, readm: __bool, writem: __bool, appendm: __bool, truncatem: __bool): ref bool
	{
		// close the file first
		if m_fd != null then
			!!__nanyc_io_file_close(m_fd);

		if not filename.empty then
		{
			var fd = !!__nanyc_io_file_open(filename.m_cstr, filename.size.pod, readm, writem, appendm, truncatem);
			m_fd = fd;
			return new bool(fd != null);
		}
		else
			m_fd = null;
		return false;
	}


	func makeViewLineByLine(cref filter, chunk: u32, limit: u32): ref
	{
		ref m_parentFile = self;
		ref m_parentFilter = filter;
		ref m_parentChunk = chunk;
		ref m_parentLimit = limit;
		return new class {
			func cursor: ref
			{
				ref origfile = m_parentFile;
				ref accept = m_parentFilter;
				ref chunk = m_parentChunk;
				ref limit = m_parentLimit;
				return new class
				{
					func findFirst: bool
						-> (origfile.m_fd != null) and next();

					func next: bool
					{
						while not origfile.eof do
						{
							// TODO implement a more efficient algo...
							m_str = origfile.readline();
							if accept(m_str) then
								return true;
						}
						return false;
					}

					func get: ref
						-> m_str;

				private:
					ref m_str = new string;
				};
			}
		};
	}


private:
	//! Internal file description
	var m_fd: __pointer = null;

} // File






public operator << (ref f: std.io.File, cref content): ref std.io.File
	-> (f += content);






// -*- mode: nany;-*-
// vim: set filetype=nany:
