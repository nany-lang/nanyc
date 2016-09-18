// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.io.path;




/*!
** \brief Make the path absolute and normalize it
*/
public func canonicalize(cref path: string): ref string
	-> normalize(absolute(path));

/*!
** \brief Make the path absolute and normalize it
*/
public func canonicalize(cref path: string, cref root: string): ref string
	-> normalize(absolute(path, root));

/*!
** \brief Get the absolute path
*/
public func absolute(cref path: string): ref string
	-> if isAbsolute(path)
		then new string(path)
		else ((std.io.folder.cwd += '/') += path);

/*!
** \brief Get the absolute path (with a given root path)
*/
public func absolute(cref path: string, cref root: string): ref string
	-> if isAbsolute(path)
		then new string(path)
		else ((new string(root) += '/') += path);


/*!
** \brief Get if a path is absolute
*/
public func isAbsolute(cref path: string): bool
	-> path.first == '/';


/*!
** \brief Get if a path is relative to a current folder
*/
public func isRelative(cref path: string): bool
	-> path.first != '/';



/*!
** \brief Simplifies a pth by removing all navigation elements
*/
public func normalize(cref path: string): ref string
{
	// if the input path is 1 ascii long, nothing really to
	// (it can be empty or '/' or '.' or whatever)
	if path.size <= 1u then
		return new string(path);

	ref np = new string;
	var pathWasAbsolute = (path.at(0u) == '/');

	if pathWasAbsolute then
		np += '/';

	for part in path:split('/') do
	{
		if part.size == 0u then
		{
			// multiple slashes - like './'
		}
		else if part.size == 1u then
		{
			if part != '.' then // current folder
				np << part << '/';
		}
		else if part.size == 2u then
		{
			if part != ".." or np.empty then
			{
				// "../" must be preserved for relative paths (np.empty)
				np << part << '/';
			}
			else
			{
				// go back to parent folder
				if pathWasAbsolute then
				{
					assert(not np.empty);
					// try to remove the last part (ignore the final slash)
					if np.size > 1u then
					{
						var li = np.lastIndex(offset: np.size - 2u, ascii: '/');
						if li < np.size then
							np.truncate(li + 1u);
						else
							np.assign('/');
					}
					else
						np.assign('/');
				}
				else
				{
					// go back to the parent. if the last part of the normalized
					// path is "..", the new part should be appended instead
					if np.size > 1u then
					{
						var li = np.lastIndex(offset: np.size - 2u, ascii: '/');
						if li < np.size then
						{
							li += 1u;
							if (np.size - 1u - li != 2u) or (np.at(li) != '.') or (np.at(li + 1u) != '.') then
								np.truncate(li);
							else
								np += "../";
						}
						else
						{
							if np != "../" then
								np.clear();
							else
								np += "../";
						}
					}
					else
						np.clear();
				}
			}
		}
		else
			np << part << '/';
	}

	if not np.empty then
	{
		if np.size != 1u and np.last == '/' then
			np.removeLastAscii();
	}
	else
		np += '.';

	return np;
}



/*!
** \brief Get the extension of a path (with the dot)
*/
public func extension(cref path: string): ref string
	-> extension(path, withDot: true);


/*!
** \brief Get the extension of a path (with or without the dot)
*/
public func extension(cref path: string, withDot: bool): ref string
{
	ref ext = new string;
	var size = path.size;
	if size != 0u then
	{
		var offset = size - 1u;
		var p = path.m_cstr + offset.pod;
		var sep = '/'.asU8.pod;
		var dot = '.'.asU8.pod;
		do
		{
			var r8 = !!load.u8(p);

			if dot == r8 then
			{
				var extsize = size - offset;
				if not withDot then
				{
					extsize -= 1u;
					ext.append(p + 1__u32, extsize);
				}
				else
					ext.append(p, extsize);
				return ext;
			}
			if sep == r8 then
				return ext;

			if offset == 0u then
				return ext; // break
			offset -= 1u;
			p = p - 1__u32;
		}
		while true;
	}
	return ext;
}


/*!
** \brief Get if a path contains an extension
*/
public func hasExtension(cref path: string): bool
{
	var size = path.size;
	if size != 0u then
	{
		var offset = size - 1u;
		var p = path.m_cstr + offset.pod;
		var sep = '/'.asU8.pod;
		var dot = '.'.asU8.pod;
		do
		{
			var r8 = !!load.u8(p);

			if dot == r8 then
				return true;
			if sep == r8 then
				return false;

			if offset == 0u then
				return false; // break
			offset -= 1u;
			p = p - 1__u32;
		}
		while true;
	}
	return false;
}


/*!
** \brief Get the folder from a path
*/
public func folder(cref path: string): ref string
{
	var rindex = path.lastIndex('/');
	var clen = path.size;
	return new string(path, size: if rindex < clen then rindex else clen);
}


/*!
** \brief Get the parent folder from a path
*/
public func parent(cref path: string): ref string
{
	var rindex = path.lastIndex('/');
	var clen = path.size;
	if rindex < clen and rindex != 0u then
		rindex = path.lastIndex(rindex - 1u, '/');
	return new string(path, size: if rindex < clen then rindex else clen);
}


/*!
 ** \brief Get the filename part of a path
 */
public func filename(cref path: string): ref string
{
	var rindex = path.lastIndex('/');
	return new string(path, offset: if rindex < path.size then rindex + 1u else 0u);
}


/*!
** \brief Get the filename of the given path without the extension
*/
public func stem(cref path: string): ref string
{
	ref filename = std.io.path.filename(path);
	var rindex = filename.lastIndex('.');
	if rindex < filename.size then
		return new string(filename, size: rindex);
	return filename;
}






// -*- mode: nany;-*-
// vim: set filetype=nany:
