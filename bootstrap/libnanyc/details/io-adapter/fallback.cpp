#include "details/vm/thread-context.h"

using namespace Yuni;


static nyio_type_t nanyc_io_fallback_stat(nyio_adapter_t*, const char*, uint32_t) {
	return nyiot_failed;
}


static nyio_type_t nanyc_io_fallback_statex(nyio_adapter_t*, const char*, uint32_t, uint64_t* size,
		int64_t* modified) {
	assert(modified);
	assert(size);
	*modified = 0;
	*size = 0;
	return nyiot_failed;
}


static uint64_t nanyc_io_fallback_file_read(void*, void*, uint64_t) {
	return 0;
}


static uint64_t nanyc_io_fallback_file_write(void*, const void*, uint64_t) {
	return 0;
}


static void* nanyc_io_fallback_file_open(nyio_adapter_t*, const char*, uint32_t,
		nybool_t, nybool_t, nybool_t, nybool_t) {
	return nullptr;
}


static void nanyc_io_fallback_file_close(void*) {
}


static nybool_t nanyc_io_fallback_file_eof(void*) {
	return nytrue;
}


static nyio_err_t nanyc_io_fallback_file_seek(void*, int64_t) {
	return nyioe_failed;
}


static nyio_err_t nanyc_io_fallback_file_seek_set(void*, uint64_t) {
	return nyioe_failed;
}


static uint64_t nanyc_io_fallback_file_tell(void*) {
	return 0;
}


static void nanyc_io_fallback_file_flush(void*) {
}


static uint64_t nanyc_io_fallback_file_size(nyio_adapter_t*, const char*, uint32_t) {
	return 0;
}


static nyio_err_t nanyc_io_fallback_file_resize(nyio_adapter_t*, const char*, uint32_t, uint64_t) {
	return nyioe_access;
}


static nyio_err_t nanyc_io_fallback_file_erase(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}


static nyio_err_t nanyc_io_fallback_file_get_contents(nyio_adapter_t*, char** content, uint64_t* size,
		uint64_t* capacity, const char*, uint32_t) {
	*size = 0;
	*capacity = 0;
	*content = nullptr;
	return nyioe_access;
}


static nyio_err_t
nanyc_io_fallback_file_set_contents(nyio_adapter_t*, const char*, uint32_t, const char*, uint32_t) {
	return nyioe_access;
}


static nyio_err_t
nanyc_io_fallback_file_append_contents(nyio_adapter_t*, const char*, uint32_t, const char*, uint32_t) {
	return nyioe_access;
}


static uint64_t nanyc_io_fallback_folder_size(nyio_adapter_t*, const char*, uint32_t) {
	return 0; // FIXME get folder size
}


static nyio_err_t nanyc_io_fallback_folder_create(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}


static nyio_err_t nanyc_io_fallback_folder_erase(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}


static nyio_err_t nanyc_io_fallback_folder_exists(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}


static nyio_err_t nanyc_io_fallback_folder_clear(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}


static nyio_iterator_t* nanyc_io_fallback_folder_iterate(nyio_adapter_t*, const char* path, uint32_t len,
		nybool_t recursive, nybool_t files, nybool_t folders) {
	// TODO implement virtual iterator on mountpoints
	return nullptr;
}


static nyio_iterator_t* nanyc_io_fallback_folder_next(nyio_iterator_t*) {
	return nullptr;
}


static const char* nanyc_io_fallback_folder_iterator_fullpath(nyio_adapter_t*, nyio_iterator_t*) {
	return nullptr;
}


static const char* nanyc_io_fallback_folder_iterator_name(nyio_iterator_t*) {
	return nullptr;
}


static uint64_t nanyc_io_fallback_folder_iterator_size(nyio_iterator_t*) {
	return 0;
}


static nyio_type_t nanyc_io_fallback_folder_iterator_type(nyio_iterator_t*) {
	return nyiot_failed;
}


static void nanyc_io_fallback_folder_iterator_close(nyio_iterator_t*) {
}


static nyio_err_t nanyc_io_fallback_file_exists(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}


namespace ny {
namespace vm {


void ThreadContext::initFallbackAdapter(nyio_adapter_t& adapter) {
	memset(&adapter, 0x0, sizeof(nyio_adapter_t));
	adapter.stat = nanyc_io_fallback_stat;
	adapter.statex = nanyc_io_fallback_statex;
	adapter.file_read = nanyc_io_fallback_file_read;
	adapter.file_write = nanyc_io_fallback_file_write;
	adapter.file_open = nanyc_io_fallback_file_open;
	adapter.file_close = nanyc_io_fallback_file_close;
	adapter.file_seek_from_end = nanyc_io_fallback_file_seek;
	adapter.file_seek = nanyc_io_fallback_file_seek_set;
	adapter.file_seek_cur = nanyc_io_fallback_file_seek;
	adapter.file_tell = nanyc_io_fallback_file_tell;
	adapter.file_flush = nanyc_io_fallback_file_flush;
	adapter.file_eof = nanyc_io_fallback_file_eof;
	adapter.file_size = nanyc_io_fallback_file_size;
	adapter.file_resize = nanyc_io_fallback_file_resize;
	adapter.file_erase = nanyc_io_fallback_file_erase;
	adapter.file_exists = nanyc_io_fallback_file_exists;
	adapter.file_get_contents = nanyc_io_fallback_file_get_contents;
	adapter.file_set_contents = nanyc_io_fallback_file_set_contents;
	adapter.file_append_contents = nanyc_io_fallback_file_append_contents;
	adapter.folder_create = nanyc_io_fallback_folder_create;
	adapter.folder_erase = nanyc_io_fallback_folder_erase;
	adapter.folder_clear = nanyc_io_fallback_folder_clear;
	adapter.folder_size = nanyc_io_fallback_folder_size;
	adapter.folder_exists = nanyc_io_fallback_folder_exists;
	adapter.folder_iterate = nanyc_io_fallback_folder_iterate;
	adapter.folder_next = nanyc_io_fallback_folder_next;
	adapter.folder_iterator_type = nanyc_io_fallback_folder_iterator_type;
	adapter.folder_iterator_name = nanyc_io_fallback_folder_iterator_name;
	adapter.folder_iterator_size = nanyc_io_fallback_folder_iterator_size;
	adapter.folder_iterator_fullpath = nanyc_io_fallback_folder_iterator_fullpath;
	adapter.folder_iterator_close = nanyc_io_fallback_folder_iterator_close;
}


} // namespace vm
} // namespace ny
