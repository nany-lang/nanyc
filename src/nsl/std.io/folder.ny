// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.io.folder;



public func entries(cref path: string): ref Folder
	-> new Folder(path);


/*!
** \brief Current working folder (CWD)
**
** \note: The current working directory is per-thread
*/
var cwd -> {
	get: new string(!!__nanyc_io_get_cwd),
	set: setCurrentWorkingFolder(value) };


/*!
** \brief Get if a node exists and is a folder
*/
public func exists(cref path: string): bool
	-> new bool(!!__nanyc_io_folder_exists(path.m_cstr, path.size.pod));


//! Get if a folder is empty (no content)
public func empty(cref path: string): bool
{
	for f in entries(path) do
		return false; // something here
	return true;
}


//! Create the folder (and sub folders) if it does not exist
public func create(cref path: string): bool
	-> new bool(!!__nanyc_io_folder_create(path.m_cstr, path.size.pod));


//! Copy a folder and its content to another location
//public func copy(cref path: string, cref to: string): bool
//	-> new bool(!!__nanyc_io_folder_copy(path.pod, to.pod, __false));


//! Copy the contents of a folder to another location
//public func copyContents(cref path: string, cref to: string): bool
//	-> new bool(!!__nanyc_io_folder_copy(path.pod, to.pod, __true));


//! Move a folder and its content to another location
//public func move(cref path: string, cref to: string): bool
//	-> new bool(!!__nanyc_io_folder_move(path.pod, to.pod, __false));


//! Move the contents of a folder to another location
//public func moveContents(cref path: string, cref to: string): bool
//	-> new bool(!!__nanyc_io_folder_move(path.pod, to.pod, __true));


//! Remove the folder and its content
public func erase(cref path: string): bool
	-> new bool(!!__nanyc_io_folder_erase(path.m_cstr, path.size.pod));


//! Remove the contents of the folder
public func clear(cref path: string): bool
	-> new bool(!!__nanyc_io_folder_clear(path.m_cstr, path.size.pod));


/*!
** \brief Get the size (in bytes) of a folder
**
** \param path Absolute or relative path
*/
public func size(cref path: string): u64
	-> new u64(!!__nanyc_io_folder_size(path.m_cstr, path.size.pod));


public func read(cref path: string): ref string
{
	ref content = "";
	for f in entries(path) do
	{
		if f.isFile then
			content << " -  " << f.name << " (" << f.size << " bytes)\n";
		else
			content << "[+] " << f.name << "\n";
	}
	return content;
}


func setCurrentWorkingFolder(cref path: string): bool
{
	var cn = std.io.path.canonicalize(path);
	return new bool(!!__nanyc_io_set_cwd(cn.m_cstr, cn.m_size));
}





// -*- mode: nany;-*-
// vim: set filetype=nany:
