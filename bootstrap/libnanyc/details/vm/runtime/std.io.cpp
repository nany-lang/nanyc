#include "std.core.h"
#include "runtime.h"
#include "details/intrinsic/catalog.h"
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/io/file.h>
#include "details/vm/context.h"
#include "details/vm/console.h"

using namespace Yuni;


struct nyfile_t {
	nyio_adapter_t* adapter;
	void* fd;
};



namespace { // anonymous


// TODO avoid multiple allocations
struct IntrinsicIOIterator {
	explicit IntrinsicIOIterator(nyio_adapter_t& adapter)
		: adapter(adapter) {
	}

	// Adapter specific iterator data
	nyio_iterator_t* internal;

	//! The initial request size (see request)
	uint32_t requestInitialSize;
	//! The initial path requested, to recreate a full virtual path
	// (can be used as a temporary string for this virtual path - always refer to requestInitialSize)
	String request;
	//! The resolved mountpoint for the request
	nyio_adapter_t& adapter;
	//! Size of the adapter resolved path
	uint32_t adapterpathSize;
};


} // anonymous namespace


static bool nanyc_io_set_cwd(nyoldvm_t* vm, void* string, uint32_t size) {
	assert(vm != nullptr);
	return nyioe_ok == vm->io_set_cwd(vm, (const char*)string, size);
}


static const void* nanyc_io_get_cwd(nyoldvm_t* vm) {
	assert(vm != nullptr);
	return vm->io_get_cwd(vm, nullptr);
}


static void* nanyc_io_folder_iterate(nyoldvm_t* vm, const char* path, uint32_t len,
		bool recursive, bool files, bool folders) {
	AnyString adapterpath;
	AnyString requestedPath{path, len};
	nyanystr_t relpath;
	nyanystr_t ipath;
	ipath.c_str = path;
	ipath.len = len;
	auto& adapter = *(vm->io_resolve(vm, &relpath, &ipath));
	nyio_iterator_t* internal = adapter.folder_iterate(&adapter, adapterpath.c_str(), adapterpath.size(),
								to_nybool(recursive), to_nybool(files), to_nybool(folders));
	if (internal) {
		auto* iterator = new IntrinsicIOIterator(adapter);
		iterator->internal = internal;
		iterator->requestInitialSize = requestedPath.size();
		iterator->request = requestedPath;
		iterator->adapterpathSize = adapterpath.size();
		return iterator;
	}
	return nullptr;
}


static void nanyc_io_folder_iterator_close(nyoldvm_t*, void* ptr) {
	if (ptr) {
		auto* iterator = reinterpret_cast<IntrinsicIOIterator*>(ptr);
		if (iterator->internal)
			iterator->adapter.folder_iterator_close(iterator->internal);
		delete iterator;
	}
}


static uint64_t nanyc_io_folder_iterator_size(nyoldvm_t*, void* ptr) {
	auto* iterator = reinterpret_cast<IntrinsicIOIterator*>(ptr);
	if (iterator and iterator->internal)
		return iterator->adapter.folder_iterator_size(iterator->internal);
	return 0;
}


static const char* nanyc_io_folder_iterator_name(nyoldvm_t*, void* ptr) {
	auto* iterator = reinterpret_cast<IntrinsicIOIterator*>(ptr);
	if (iterator and iterator->internal)
		return iterator->adapter.folder_iterator_name(iterator->internal);
	return nullptr;
}


static const char* nanyc_io_folder_iterator_fullpath(nyoldvm_t*, void* ptr) {
	auto* iterator = reinterpret_cast<IntrinsicIOIterator*>(ptr);
	if (iterator and iterator->internal) {
		// restore the initial request
		iterator->request.truncate(iterator->requestInitialSize);
		// append the relative path used by adapter
		// .. absolute path from the adapter
		const char* p = iterator->adapter.folder_iterator_fullpath(&iterator->adapter, iterator->internal);
		// .. - mountpoint path
		p += iterator->adapterpathSize;
		// .. merging
		iterator->request += p;
		return iterator->request.c_str();
	}
	return nullptr;
}


static bool nanyc_io_folder_iterator_next(nyoldvm_t*, void* ptr) {
	auto* iterator = reinterpret_cast<IntrinsicIOIterator*>(ptr);
	if (iterator and iterator->internal) {
		nyio_iterator_t* p = iterator->adapter.folder_next(iterator->internal);
		iterator->internal = p;
		bool success = (p != nullptr);
		return success;
	}
	return false;
}


static bool nanyc_io_folder_create(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.folder_create(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
	return (err == nyioe_ok);
}


static bool nanyc_io_folder_erase(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.folder_erase(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
	return (err == nyioe_ok);
}


static bool nanyc_io_folder_clear(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.folder_clear(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
	return (err == nyioe_ok);
}


static uint64_t nanyc_io_folder_size(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	return adapter.folder_size(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
}


static bool nanyc_io_folder_exists(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.folder_exists(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
	return (err == nyioe_ok);
}


static bool nanyc_io_folder_copy(nyoldvm_t* vm, const char* path, uint32_t len, const char* to, uint32_t tolen) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	nyanystr_t adapterPathTo;
	nyanystr_t requestedPathTo;
	requestedPathTo.c_str = to;
	requestedPathTo.len = tolen;
	auto& adapterTo = *vm->io_resolve(vm, &adapterPathTo, &requestedPathTo);
	// TODO implement !!__nanyc_io_folder_copy
	if (&adapter == &adapterTo) {
		// optimisation when copying data with an unique adapter
	}
	else {
		// failsafe
	}
	return false;
}


static bool nanyc_io_file_exists(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.file_exists(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
	return (err == nyioe_ok);
}


static uint64_t nanyc_io_file_size(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	return adapter.file_size(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
}


static bool nanyc_io_file_erase(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.file_erase(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len);
	return (err == nyioe_ok);
}


static bool nanyc_io_file_resize(nyoldvm_t* vm, const char* path, uint32_t len, uint64_t newsize) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	auto err = adapter.file_resize(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len, newsize);
	return (err == nyioe_ok);
}


static bool nanyc_io_file_set_contents(nyoldvm_t* vm, const char* path, uint32_t len,
		const char* content, uint32_t clen, bool append) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	AnyString newcontent{content, clen};
	nyio_err_t err = (not append)
		? adapter.file_set_contents(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len, content, clen)
		: adapter.file_append_contents(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len, content, clen);
	return (err == nyioe_ok);
}


static void* nanyc_io_file_get_contents(nyoldvm_t* vm, const char* path, uint32_t len) {
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	char* content = nullptr;
	uint64_t size = 0;
	uint64_t capacity = 0;
	nyio_err_t err = adapter.file_get_contents(&adapter,
		&content, &size, &capacity, adapterPath.c_str, (uint32_t) adapterPath.len);
	if (err == nyioe_ok) {
		vm->returnValue.size     = size;
		vm->returnValue.capacity = capacity;
		vm->returnValue.data     = content;
		return &vm->returnValue;
	}
	return nullptr;
}


static nyfile_t* nanyc_io_file_open(nyoldvm_t* vm, const char* path, uint32_t len,
		bool readm, bool writem, bool appendm, bool truncm) {
	assert(vm);
	assert(path);
	assert(len != 0);
	assert(readm or writem);
	nyanystr_t adapterPath;
	nyanystr_t requestedPath;
	requestedPath.c_str = path;
	requestedPath.len = len;
	auto& adapter = *vm->io_resolve(vm, &adapterPath, &requestedPath);
	void* fd = adapter.file_open(&adapter, adapterPath.c_str, (uint32_t) adapterPath.len,
		to_nybool(readm), to_nybool(writem), to_nybool(appendm), to_nybool(truncm));
	if (unlikely(fd == adapter.invalid_fd))
		return nullptr;
	auto* f = (nyfile_t*) malloc(sizeof(struct nyfile_t));
	if (unlikely(!f)) {
		adapter.file_close(fd);
		return nullptr;
	}
	f->adapter = &adapter;
	f->fd = fd;
	return f;
}


static void nanyc_io_file_close(nyoldvm_t* /*vm*/, nyfile_t* file) {
	assert(file != nullptr);
	assert(file->adapter != nullptr);
	// close the file handle
	file->adapter->file_close(file->fd);
	// release internal struct
	free(file); // sizeof(struct nyfile_t));
}


static void nanyc_io_file_flush(nyoldvm_t*, nyfile_t* file) {
	assert(file != nullptr);
	file->adapter->file_flush(file->fd);
}


static uint64_t nanyc_io_file_write(nyoldvm_t*, nyfile_t* file, const char* buffer, uint64_t size) {
	assert(file != nullptr);
	assert(file->adapter != nullptr);
	return file->adapter->file_write(file->fd, buffer, size);
}


static uint64_t nanyc_io_file_read(nyoldvm_t*, nyfile_t* file, char* buffer, uint64_t size) {
	assert(file != nullptr);
	assert(file->adapter != nullptr);
	return file->adapter->file_read(file->fd, buffer, size);
}


static bool nanyc_io_file_eof(nyoldvm_t*, nyfile_t* file) {
	assert(file != nullptr);
	assert(file->adapter != nullptr);
	return (nyfalse != file->adapter->file_eof(file->fd));
}


static bool nanyc_io_file_seek_set(nyoldvm_t*, nyfile_t* file, uint64_t offset) {
	assert(file != nullptr);
	auto err = file->adapter->file_seek(file->fd, offset);
	return (err == nyioe_ok);
}


static bool nanyc_io_file_seek_from_end(nyoldvm_t*, nyfile_t* file, int64_t offset) {
	assert(file != nullptr);
	auto err = file->adapter->file_seek_from_end(file->fd, offset);
	return (err == nyioe_ok);
}


static bool nanyc_io_file_seek_cur(nyoldvm_t*, nyfile_t* file, int64_t offset) {
	assert(file != nullptr);
	auto err = file->adapter->file_seek_cur(file->fd, offset);
	return (err == nyioe_ok);
}


static uint64_t nanyc_io_file_tell(nyoldvm_t*, nyfile_t* file) {
	assert(file != nullptr);
	return file->adapter->file_tell(file->fd);
}


static bool nanyc_io_mount_local(nyoldvm_t* vm, const char* path, uint32_t len, const char* local,
		uint32_t locallen) {
	if (path and len and local and locallen) {
		nyio_adapter_t adapter;
		nyio_adapter_init_localfolder(&adapter, local, locallen);
		bool success = nyioe_ok == vm->io_add_mountpoint(vm, path, len, &adapter);
		if (adapter.release)
			adapter.release(&adapter);
		return success;
	}
	return false;
}


namespace ny {
namespace nsl {
namespace import {


void io(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc_io_set_cwd",  nanyc_io_set_cwd);
	intrinsics.emplace("__nanyc_io_get_cwd",  nanyc_io_get_cwd);
	intrinsics.emplace("__nanyc_io_folder_create",  nanyc_io_folder_create);
	intrinsics.emplace("__nanyc_io_folder_erase",  nanyc_io_folder_erase);
	intrinsics.emplace("__nanyc_io_folder_clear",  nanyc_io_folder_clear);
	intrinsics.emplace("__nanyc_io_folder_size",  nanyc_io_folder_size);
	intrinsics.emplace("__nanyc_io_folder_exists",  nanyc_io_folder_exists);
	intrinsics.emplace("__nanyc_io_folder_copy",  nanyc_io_folder_copy);
	intrinsics.emplace("__nanyc_io_folder_iterate",  nanyc_io_folder_iterate);
	intrinsics.emplace("__nanyc_io_folder_iterator_close",  nanyc_io_folder_iterator_close);
	intrinsics.emplace("__nanyc_io_folder_iterator_size",  nanyc_io_folder_iterator_size);
	intrinsics.emplace("__nanyc_io_folder_iterator_name",  nanyc_io_folder_iterator_name);
	intrinsics.emplace("__nanyc_io_folder_iterator_fullpath",  nanyc_io_folder_iterator_fullpath);
	intrinsics.emplace("__nanyc_io_folder_iterator_next",  nanyc_io_folder_iterator_next);
	intrinsics.emplace("__nanyc_io_file_exists", nanyc_io_file_exists);
	intrinsics.emplace("__nanyc_io_file_size",   nanyc_io_file_size);
	intrinsics.emplace("__nanyc_io_file_resize", nanyc_io_file_resize);
	intrinsics.emplace("__nanyc_io_file_erase",  nanyc_io_file_erase);
	intrinsics.emplace("__nanyc_io_file_set_contents",  nanyc_io_file_set_contents);
	intrinsics.emplace("__nanyc_io_file_get_contents",  nanyc_io_file_get_contents);
	intrinsics.emplace("__nanyc_io_file_open",   nanyc_io_file_open);
	intrinsics.emplace("__nanyc_io_file_close",  nanyc_io_file_close);
	intrinsics.emplace("__nanyc_io_file_flush",  nanyc_io_file_flush);
	intrinsics.emplace("__nanyc_io_file_write",  nanyc_io_file_write);
	intrinsics.emplace("__nanyc_io_file_read",   nanyc_io_file_read);
	intrinsics.emplace("__nanyc_io_file_eof",   nanyc_io_file_eof);
	intrinsics.emplace("__nanyc_io_file_seek",   nanyc_io_file_seek_set);
	intrinsics.emplace("__nanyc_io_file_seek_from_end",   nanyc_io_file_seek_from_end);
	intrinsics.emplace("__nanyc_io_file_seek_cur",   nanyc_io_file_seek_cur);
	intrinsics.emplace("__nanyc_io_file_tell",   nanyc_io_file_tell);
	intrinsics.emplace("__nanyc_io_mount_local",   nanyc_io_mount_local);
}


} // namespace import
} // namespace nsl
} // namespace ny
