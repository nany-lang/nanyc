#include <yuni/yuni.h>
#include <yuni/core/system/console/console.h>
#include "nany/nany.h"
#include <yuni/io/directory.h>
#include <yuni/io/file.h>
#include <yuni/io/directory/info/info.h>
#include "libnanyc-config.h"
#include <iostream>

using namespace Yuni;


namespace { // anonymous


struct Internal {
	Internal(const AnyString& localfolder, nyallocator_t* allocator)
		: allocator(allocator) {
		if (not System::windows) {
			if (localfolder != '/')
				localpath = localfolder;
		}
		else {
			if (not localfolder.empty()) {
				localpath = localfolder;
				localpath.trimRight('\\');
				localpath.trimRight('/');
			}
		}
	}

	String localpath;
	//! Temporary string for ensuring zero-terminated strings
	String tmppath;
	//! Memory allocator
	nyallocator_t* const allocator;
};


inline nyallocator_t& retrieveAllocator(nyio_adapter_t* adapter) {
	auto& internal = *reinterpret_cast<Internal*>(adapter->internal);
	return *(internal.allocator);
}


String& toLocalPath(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(len != 0);
	assert(path != nullptr and path[0] == '/');
	auto& internal = *reinterpret_cast<Internal*>(adapter->internal);
	if (not System::windows) {
		internal.tmppath.assign(internal.localpath);
		internal.tmppath.append(path, len);
	}
	else {
		if (not internal.localpath.empty()) {
			internal.tmppath.assign(internal.localpath);
			internal.tmppath.append(path, len);
		}
		else {
			internal.tmppath.clear();
			// convertir paths like `/d/subpaths`
			if (len > 2) {
				if (path[2] == '/' and String::IsAlpha(path[1])) {
					internal.localpath << path[1];
					internal.localpath.append(":\\", 2);
					internal.localpath.append(path + 3, len - 3);
				}
			}
			else if (len == 2) {
				if (String::IsAlpha(path[1]))
					(internal.localpath << path[1]).append(":\\", 2);
			}
			else {
				// error, invalid path, going nowhere
			}
		}
	}
	return internal.tmppath;
}


} // anonymous namespace


static void nanyc_io_localfolder_release(nyio_adapter_t* adapter) {
	assert(adapter != nullptr);
	delete reinterpret_cast<Internal*>(adapter->internal);
	adapter->internal = nullptr; // avoid misuse
}

static void nanyc_io_localfolder_clone(nyio_adapter_t* parent, nyio_adapter_t* dst) {
	assert(dst != nullptr);
	assert(parent != nullptr);
	memcpy(dst, parent, sizeof(nyio_adapter_t));
	auto& internal = *reinterpret_cast<Internal*>(parent->internal);
	dst->internal = new Internal{internal.localpath, internal.allocator};
}


static nyio_type_t nanyc_io_localfolder_stat(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	uint64_t size;
	int64_t modified;
	switch (IO::FetchFileStatus(toLocalPath(adapter, path, len), size, modified)) {
		case IO::typeFile:
			return nyiot_file;
		case IO::typeFolder:
			return nyiot_folder;
		default:
			return nyiot_failed;
	}
}


static nyio_type_t nanyc_io_localfolder_statex(nyio_adapter_t* adapter, const char* path, uint32_t len,
		uint64_t* size, int64_t* modified) {
	assert(adapter != nullptr);
	assert(modified);
	assert(size);
	switch (IO::FetchFileStatus(toLocalPath(adapter, path, len), *size, *modified)) {
		case IO::typeFile:
			return nyiot_file;
		case IO::typeFolder:
			return nyiot_folder;
		default:
			return nyiot_failed;
	}
	return nyiot_failed;
}


static uint64_t nanyc_io_localfolder_file_read(void* file, void* buffer, uint64_t bufsize) {
	assert(file != nullptr and "invalid file pointer for file_read");
	return (*reinterpret_cast<IO::File::Stream*>(file)).read((char*)buffer, bufsize);
}


static uint64_t nanyc_io_localfolder_file_write(void* file, const void* buffer, uint64_t bufsize) {
	assert(file != nullptr and "invalid file pointer for file_write");
	return (*reinterpret_cast<IO::File::Stream*>(file)).write(buffer, bufsize);
}


static void* nanyc_io_localfolder_file_open(nyio_adapter_t* adapter, const char* path, uint32_t len,
		nybool_t readm, nybool_t writem, nybool_t appendm, nybool_t truncm) {
	assert(adapter != nullptr);
	AnyString localpath = toLocalPath(adapter, path, len);
	int mode = 0;
	if (readm != nyfalse)
		mode |= IO::OpenMode::read;
	if (writem != nyfalse) {
		mode |= IO::OpenMode::write;
		if (appendm != nyfalse)
			mode |= IO::OpenMode::append;
		if (truncm != nyfalse)
			mode |= IO::OpenMode::truncate;
	}
	auto* file = new IO::File::Stream(localpath, mode);
	if (file) {
		if (file->opened())
			return file;
		delete file;
	}
	return nullptr;
}


static void nanyc_io_localfolder_file_close(void* file) {
	assert(file != nullptr);
	delete reinterpret_cast<IO::File::Stream*>(file);
}


static nyio_err_t nanyc_io_localfolder_file_seek(void* file, uint64_t offset) {
	assert(file != nullptr);
	auto& ynfile = (*reinterpret_cast<IO::File::Stream*>(file));
	bool success = ynfile.seek(static_cast<::ssize_t>(offset), IO::File::seekOriginBegin);
	return success ? nyioe_ok : nyioe_failed;
}


static uint64_t nanyc_io_localfolder_file_tell(void* file) {
	assert(file != nullptr);
	auto& ynfile = (*reinterpret_cast<IO::File::Stream*>(file));
	return static_cast<uint64_t>(ynfile.tell());
}


static nyio_err_t nanyc_io_localfolder_file_seek_from_end(void* file, int64_t offset) {
	assert(file != nullptr);
	auto& ynfile = (*reinterpret_cast<IO::File::Stream*>(file));
	bool success = ynfile.seek(offset, IO::File::seekOriginEnd);
	return success ? nyioe_ok : nyioe_failed;
}


static nyio_err_t nanyc_io_localfolder_file_seek_cur(void* file, int64_t offset) {
	assert(file != nullptr);
	auto& ynfile = (*reinterpret_cast<IO::File::Stream*>(file));
	bool success = ynfile.seek(offset, IO::File::seekOriginCurrent);
	return success ? nyioe_ok : nyioe_failed;
}


static void nanyc_io_localfolder_file_flush(void* file) {
	assert(file != nullptr and "invalid file pointer for file_flush");
	(*reinterpret_cast<IO::File::Stream*>(file)).flush();
}


static nybool_t nanyc_io_localfolder_file_eof(void* file) {
	assert(file != nullptr and "invalid file pointer for file_eof");
	bool eof = (*reinterpret_cast<IO::File::Stream*>(file)).eof();
	return (!eof) ? nyfalse : nytrue;
}


static uint64_t nanyc_io_localfolder_file_size(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	return IO::File::Size(toLocalPath(adapter, path, len));
}


static nyio_err_t nanyc_io_localfolder_file_resize(nyio_adapter_t* adapter,
		const char* path, uint32_t len, uint64_t newsize) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::File::Resize(localpath, newsize);
	return success ? nyioe_ok : nyioe_failed;
}


static nyio_err_t nanyc_io_localfolder_file_erase(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto err = IO::File::Delete(toLocalPath(adapter, path, len));
	return (err == IO::errNone) ? nyioe_ok : nyioe_access;
}


namespace { // anonymous


nyio_err_t get_contents_by_small_chunks(IO::File::Stream& f, nyio_adapter_t* adapter,
		char** content, uint64_t* size, uint64_t* capacity) {
	constexpr static uint32_t fragmentSize = 1024;
	uint64 readsize = 0;
	uint64_t newcapacity = 0;
	auto& allocator = retrieveAllocator(adapter);
	char* buffer = nullptr;
	do {
		buffer = (char*) allocator.reallocate(&allocator, buffer, newcapacity, newcapacity + fragmentSize);
		if (YUNI_UNLIKELY(!buffer))
			return nyioe_memory;
		uint64 numread = f.read(buffer + newcapacity, fragmentSize);
		newcapacity += fragmentSize;
		readsize += numread;
		if (numread != fragmentSize)
			break;
	}
	while (true);
	if (readsize != 0) {
		// ensuring that there is enough space
		if (newcapacity < readsize + ny::config::extraObjectSize)
			buffer = (char*) allocator.reallocate(&allocator, buffer, newcapacity, newcapacity + fragmentSize);
		*size = readsize;
		*content = buffer;
		*capacity = newcapacity;
	}
	else
		allocator.deallocate(&allocator, buffer, newcapacity);
	return nyioe_ok;
}


} // anonymous namespace


static nyio_err_t nanyc_io_localfolder_file_get_contents(nyio_adapter_t* adapter,
		char** content, uint64_t* size, uint64_t* capacity, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	assert(content != nullptr);
	assert(size != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	*size = 0;
	*content = nullptr;
	*capacity = 0;
	IO::File::Stream f(localpath);
	if (not f.opened())
		return nyioe_access;
	// retrieve the file size in bytes
	f.seekFromEndOfFile(0);
	uint64 filesize = (uint64) f.tell();
	if (filesize == 0) {
		if (System::unix) {
			// On unix, some special files can have a size equals to zero, but
			// not being empty (like files on /proc)
			// swithing to the standard method for reading a file
			return get_contents_by_small_chunks(f, adapter, content, size, capacity);
		}
		return nyioe_ok;
	}
	// replace the cursor at the beginning of the file
	f.seekFromBeginning(0);
	// even if tempting, it is not very wise to read the file
	// with a single read. When the file is really big, it may prevent
	// the thread from being interruptible
	constexpr static uint32_t fragmentSize = 1 * 1024 * 1024;
	uint64_t newcapacity;
	char* buffer;
	if (filesize < fragmentSize) {
		newcapacity = filesize + ny::config::extraObjectSize;
		auto& allocator = retrieveAllocator(adapter);
		buffer = (char*) allocator.allocate(&allocator, newcapacity);
		if (YUNI_UNLIKELY(!buffer))
			return nyioe_memory;
		uint64_t numread = f.read(buffer, filesize);
		if (numread != filesize) {
			allocator.deallocate(&allocator, buffer, newcapacity);
			return nyioe_access;
		}
	}
	else {
		// rounding the capacity to avoid any unexpected buffer overflow
		// when reading the file
		newcapacity = ((filesize + fragmentSize / 2) / fragmentSize) * fragmentSize;
		if (newcapacity < filesize + ny::config::extraObjectSize)
			newcapacity += fragmentSize;
		assert(newcapacity >= filesize);
		auto& allocator = retrieveAllocator(adapter);
		buffer = (char*) allocator.allocate(&allocator, newcapacity);
		if (YUNI_UNLIKELY(!buffer))
			return nyioe_memory;
		uint64_t offset = 0;
		uint64_t numread;
		do {
			numread = f.read(buffer + offset, fragmentSize);
			offset += numread;
		}
		while (offset < filesize and 0 != numread);
		if (offset != filesize) // early abort
			filesize = offset;
	}
	*size = filesize;
	*content = buffer;
	*capacity = filesize;
	return nyioe_ok;
}


static nyio_err_t nanyc_io_localfolder_file_set_contents(nyio_adapter_t* adapter,
		const char* path, uint32_t len, const char* content, uint32_t ctlen) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::File::SetContent(localpath, AnyString{content, ctlen});
	return (success ? nyioe_ok : nyioe_access);
}


static nyio_err_t nanyc_io_localfolder_file_append_contents(nyio_adapter_t* adapter,
		const char* path, uint32_t len, const char* content, uint32_t ctlen) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::File::AppendContent(localpath, AnyString{content, ctlen});
	return (success ? nyioe_ok : nyioe_access);
}


static nyio_err_t nanyc_io_localfolder_folder_create(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::Directory::Create(localpath);
	return (success ? nyioe_ok : nyioe_access);
}


static nyio_err_t nanyc_io_localfolder_folder_erase(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::Directory::Remove(localpath);
	return (success ? nyioe_ok : nyioe_access);
}


static nyio_err_t nanyc_io_localfolder_folder_exists(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::Directory::Exists(localpath);
	return (success ? nyioe_ok : nyioe_access);
}


static nyio_err_t nanyc_io_localfolder_folder_clear(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = false;
	IO::Directory::Info info{localpath};
	auto end = info.end();
	for (auto it = info.begin(); it != end; ++it) {
		if (it.isFile())
			success &= (IO::errNone == IO::File::Delete(it.filename()));
		else
			success &= IO::Directory::Remove(it.filename());
	}
	return (success ? nyioe_ok : nyioe_access);
}


static nyio_iterator_t* nanyc_io_localfolder_folder_iterate(nyio_adapter_t* adapter, const char* path,
		uint32_t len,
		nybool_t recursive, nybool_t files, nybool_t folders) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	uint32_t flags = 0;
	if (recursive)
		flags |= IO::Directory::Info::itRecursive;
	if (files)
		flags |= IO::Directory::Info::itFile;
	if (folders)
		flags |= IO::Directory::Info::itFolder;
	auto* data = Private::IO::Directory::IteratorDataCreate(localpath, flags);
	return reinterpret_cast<nyio_iterator_t*>(data);
}


static void nanyc_io_localfolder_folder_iterator_close(nyio_iterator_t* it) {
	assert(it != nullptr);
	auto* itdata = reinterpret_cast<Private::IO::Directory::IteratorData*>(it);
	Private::IO::Directory::IteratorDataFree(itdata);
}


static nyio_iterator_t* nanyc_io_localfolder_folder_next(nyio_iterator_t* it) {
	assert(it);
	auto* itdata = reinterpret_cast<Private::IO::Directory::IteratorData*>(it);
	itdata = Private::IO::Directory::IteratorDataNext(itdata);
	return reinterpret_cast<nyio_iterator_t*>(itdata);
}


static const char* nanyc_io_localfolder_folder_iterator_fullpath(nyio_adapter_t* adapter,
		nyio_iterator_t* it) {
	assert(it);
	auto& internal = *reinterpret_cast<Internal*>(adapter->internal);
	auto* itdata = reinterpret_cast<Private::IO::Directory::IteratorData*>(it);
	const char* p = Private::IO::Directory::IteratorDataFilename(itdata).c_str();
	// remove the part
	p += internal.localpath.size();
	return p;
}


static const char* nanyc_io_localfolder_folder_iterator_name(nyio_iterator_t* it) {
	assert(it);
	auto* itdata = reinterpret_cast<Private::IO::Directory::IteratorData*>(it);
	return Private::IO::Directory::IteratorDataName(itdata).c_str();
}


static uint64_t nanyc_io_localfolder_folder_iterator_size(nyio_iterator_t* it) {
	assert(it);
	auto* itdata = reinterpret_cast<Private::IO::Directory::IteratorData*>(it);
	return Private::IO::Directory::IteratorDataSize(itdata);
}


static nyio_type_t nanyc_io_localfolder_folder_iterator_type(nyio_iterator_t* it) {
	assert(it);
	auto* itdata = reinterpret_cast<Private::IO::Directory::IteratorData*>(it);
	return (Private::IO::Directory::IteratorDataIsFile(itdata))
		   ? nyiot_file : nyiot_folder;
}


static uint64_t nanyc_io_localfolder_folder_size(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	uint64_t bytes = 0;
	IO::Directory::Info info{localpath};
	auto end = info.recursive_file_end();
	for (auto it = info.recursive_file_begin(); it != end; ++it)
		bytes += it->size();
	return bytes;
}


static nyio_err_t
nanyc_io_localfolder_file_exists(nyio_adapter_t* adapter, const char* path, uint32_t len) {
	assert(adapter != nullptr);
	auto& localpath = toLocalPath(adapter, path, len);
	bool success = IO::File::Exists(localpath);
	return success ? nyioe_ok : nyioe_access;
}


extern "C" void nyio_adapter_create_from_local_folder(nyio_adapter_t* adapter, nyallocator_t* allocator,
		const char* lfol, size_t len) {
	assert(adapter != nullptr);
	assert(allocator != nullptr);
	if (YUNI_UNLIKELY(!adapter or !adapter))
		return;
	memset(adapter, 0x0, sizeof(nyio_adapter_t));
	AnyString localfolder;
	if (len < 32767u) // hard limit on most filesystems
		localfolder.adapt(lfol, static_cast<uint32_t>(len));
	if (YUNI_UNLIKELY(localfolder.empty()))
		localfolder = "/";
	adapter->internal = new Internal{localfolder, allocator};
	adapter->release  = nanyc_io_localfolder_release;
	adapter->clone    = nanyc_io_localfolder_clone;
	adapter->stat = nanyc_io_localfolder_stat;
	adapter->statex = nanyc_io_localfolder_statex;
	adapter->file_read = nanyc_io_localfolder_file_read;
	adapter->file_write = nanyc_io_localfolder_file_write;
	adapter->file_open = nanyc_io_localfolder_file_open;
	adapter->file_close = nanyc_io_localfolder_file_close;
	adapter->file_seek_from_end = nanyc_io_localfolder_file_seek_from_end;
	adapter->file_seek = nanyc_io_localfolder_file_seek;
	adapter->file_seek_cur = nanyc_io_localfolder_file_seek_cur;
	adapter->file_tell = nanyc_io_localfolder_file_tell;
	adapter->file_flush = nanyc_io_localfolder_file_flush;
	adapter->file_eof = nanyc_io_localfolder_file_eof;
	adapter->file_size = nanyc_io_localfolder_file_size;
	adapter->file_resize = nanyc_io_localfolder_file_resize;
	adapter->file_erase = nanyc_io_localfolder_file_erase;
	adapter->file_exists = nanyc_io_localfolder_file_exists;
	adapter->file_get_contents = nanyc_io_localfolder_file_get_contents;
	adapter->file_set_contents = nanyc_io_localfolder_file_set_contents;
	adapter->file_append_contents = nanyc_io_localfolder_file_append_contents;
	adapter->folder_create = nanyc_io_localfolder_folder_create;
	adapter->folder_erase = nanyc_io_localfolder_folder_erase;
	adapter->folder_clear = nanyc_io_localfolder_folder_clear;
	adapter->folder_size = nanyc_io_localfolder_folder_size;
	adapter->folder_exists = nanyc_io_localfolder_folder_exists;
	adapter->folder_iterate = nanyc_io_localfolder_folder_iterate;
	adapter->folder_next = nanyc_io_localfolder_folder_next;
	adapter->folder_iterator_close = nanyc_io_localfolder_folder_iterator_close;
	adapter->folder_iterator_type = nanyc_io_localfolder_folder_iterator_type;
	adapter->folder_iterator_name = nanyc_io_localfolder_folder_iterator_name;
	adapter->folder_iterator_size = nanyc_io_localfolder_folder_iterator_size;
	adapter->folder_iterator_fullpath = nanyc_io_localfolder_folder_iterator_fullpath;
}
