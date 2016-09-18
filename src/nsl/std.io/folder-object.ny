// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.io;




public class Folder
{
public:
	operator new;
	operator new(self cref path: string);


	//! Get if the folder exists
	var exists
		-> std.io.folder.exists(path);

	//! Get if the folder is empty (no contents)
	var empty
		-> std.io.folder.empty(path);

	//! Get (compute recursively) the size of a folder (in bytes)
	var size
		-> std.io.folder.size(path);


	//! Create the folder recursively if not exists
	func create: bool
		-> std.io.folder.create(path);


	//! Create the folder if it does not exist
	func create(recursive: bool): bool
		-> std.io.folder.create(path, recursive);


	//! Copy a folder and its content to another location
	func copy(cref to: string): bool
		-> std.io.folder.copy(path, to);


	//! Copy the contents of a folder to another location
	func copyContents(cref to: string): bool
		-> std.io.folder.copyContents(path, to);


	//! Move a folder and its content to another location
	func move(cref to: string): bool
		-> std.io.folder.move(path, to);


	//! Move the contents of a folder to another location
	func moveContents(cref to: string): bool
		-> std.io.folder.moveContents(path, to);


	//! Remove the folder and its content
	func erase: bool
		-> std.io.folder.erase(path);


	//! Remove the contents of the folder
	func clear: bool
		-> std.io.folder.clear(path);


	func read: ref string
		-> std.io.folder.read(path);


	//! Get a view on all files and folders of the path (not recursive)
	view (ref filter)
		-> makeViewFromFolder(filter, recursive: false, files: true, folders: true);

	//! Get a view on all files and folders of the path (recursive)
	view recursive(ref filter)
		-> makeViewFromFolder(filter, recursive: true, files: true, folders: true);


	view subfolders(ref filter)
		-> makeViewFromFolder(filter, recursive: false, files: false, folders: true);


	view subfolders(ref filter, recursive: bool)
		-> makeViewFromFolder(filter, recursive: recursive, files: false, folders: true);


	view files(ref filter)
		-> makeViewFromFolder(filter, recursive: false, files: true, folders: false);


	view files(ref filter, recursive: bool)
		-> makeViewFromFolder(filter, recursive: recursive, files: true, folders: false);


	view entries(ref filter, recursive: bool, files: bool, folders: bool)
		-> makeViewFromFolder(filter, recursive: recursive, files: files, folders: folders);


public:
	class Element
	{
		//! The path of the current element
		var filename -> m_fullname;
		//! Name of the current element
		var name -> m_name;
		//! The size in bytes of the current element (0 if not a file)
		var size -> m_size;
		//! Extension of the element
		var extension -> std.io.path.extension(m_name);

		//! Get if the element is a file
		var isFile -> m_kind;
		//! Get if the element is a folder
		var isFolder -> not m_kind;

	private:
		func importFromIterator(p: __pointer)
		{
			m_fullname.clear();
			m_fullname.appendCString(!!__nanyc_io_folder_iterator_fullpath(p));
			m_name.clear();
			m_name.appendCString(!!__nanyc_io_folder_iterator_name(p));
			m_size = new u64(!!__nanyc_io_folder_iterator_size(p));
			m_kind = true;
		}

		var m_fullname = "";
		var m_name = "";
		var m_size = 0u64;
		var m_kind = true; // TODO use appropriate enum here: true: file
	}


	func makeViewFromFolder(cref filter, recursive: bool, files: bool, folders: bool): ref
	{
		ref m_parentFolder = self;
		ref m_parentFilter = filter;
		ref m_parentRecursive = recursive;
		ref m_parentFiles = files;
		ref m_parentFolders = folders;
		return new class {
			func cursor: ref
			{
				ref origFolder = m_parentFolder;
				ref accept = m_parentFilter;
				ref recursive = m_parentRecursive;
				ref files = m_parentFiles;
				ref folders = m_parentFolders;
				return new class
				{
					operator dispose
					{
						!!__nanyc_io_folder_iterator_close(m_iterator);
					}

					func findFirst: bool
					{
						var cn = std.io.path.canonicalize(origFolder.path);
						m_iterator =
							!!__nanyc_io_folder_iterate(cn.m_cstr, cn.size.pod, recursive.pod, files.pod, folders.pod);
						return new bool(m_iterator != null) and next();
					}

					func next: bool
					{
						do
						{
							var hasNext =  !!__nanyc_io_folder_iterator_next(m_iterator);
							if not hasNext then
								return false;
							m_element.importFromIterator(m_iterator);
						}
						while not accept(m_element);
						return true;
					}

					func get: ref
						-> m_element;

					var m_iterator = null;
					var m_element = new Element;
				};
			}

			view (filter): ref
				-> m_parentFolder.makeViewFromFolder(func (cref i) -> m_parentFilter(i) and filter(i),
					m_parentRecursive, m_parentFiles, m_parentFolders);
		};
	}


private:
	var path = "";
}





// -*- mode: nany;-*-
// vim: set filetype=nany:
