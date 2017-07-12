#include <nanyc/io.h>
#include "libnanyc.h"
#include <cstring>


namespace {

nyio_type_t nyinx_io_devnull_stat(nyio_adapter_t*, const char*, uint32_t) {
	return nyiot_failed;
}

nyio_type_t nyinx_io_devnull_statex(nyio_adapter_t*, const char*, uint32_t, uint64_t* size,
		int64_t* modified) {
	assert(modified);
	assert(size);
	*modified = 0;
	*size = 0;
	return nyiot_failed;
}

uint64_t nyinx_io_devnull_file_read(void*, void*, uint64_t) {
	return 0;
}

uint64_t nyinx_io_devnull_file_write(void*, const void*, uint64_t) {
	return 0;
}

void* nyinx_io_devnull_file_open(nyio_adapter_t*, const char*, uint32_t,
		nybool_t, nybool_t, nybool_t, nybool_t) {
	return nullptr;
}

void nyinx_io_devnull_file_close(void*) {
}

nybool_t nyinx_io_devnull_file_eof(void*) {
	return nytrue;
}

nyio_err_t nyinx_io_devnull_file_seek(void*, int64_t) {
	return nyioe_failed;
}

nyio_err_t nyinx_io_devnull_file_seek_set(void*, uint64_t) {
	return nyioe_failed;
}

uint64_t nyinx_io_devnull_file_tell(void*) {
	return 0;
}

void nyinx_io_devnull_file_flush(void*) {
}

uint64_t nyinx_io_devnull_file_size(nyio_adapter_t*, const char*, uint32_t) {
	return 0;
}

nyio_err_t nyinx_io_devnull_file_resize(nyio_adapter_t*, const char*, uint32_t, uint64_t) {
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_file_erase(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_file_get_contents(nyio_adapter_t*, char** content, uint64_t* size,
		uint64_t* capacity, const char*, uint32_t) {
	*size = 0;
	*capacity = 0;
	*content = nullptr;
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_file_set_contents(nyio_adapter_t*, const char*, uint32_t, const char*, uint32_t) {
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_file_append_contents(nyio_adapter_t*, const char*, uint32_t, const char*, uint32_t) {
	return nyioe_access;
}

uint64_t nyinx_io_devnull_folder_size(nyio_adapter_t*, const char*, uint32_t) {
	return 0; // FIXME get folder size
}

nyio_err_t nyinx_io_devnull_folder_create(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_folder_erase(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_folder_exists(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}

nyio_err_t nyinx_io_devnull_folder_clear(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}

nyio_iterator_t* nyinx_io_devnull_folder_iterate(nyio_adapter_t*, const char* path, uint32_t len,
		nybool_t recursive, nybool_t files, nybool_t folders) {
	return nullptr;
}

nyio_iterator_t* nyinx_io_devnull_folder_next(nyio_iterator_t*) {
	return nullptr;
}

const char* nyinx_io_devnull_folder_iterator_fullpath(nyio_adapter_t*, nyio_iterator_t*) {
	return nullptr;
}

const char* nyinx_io_devnull_folder_iterator_name(nyio_iterator_t*) {
	return nullptr;
}

uint64_t nyinx_io_devnull_folder_iterator_size(nyio_iterator_t*) {
	return 0;
}

nyio_type_t nyinx_io_devnull_folder_iterator_type(nyio_iterator_t*) {
	return nyiot_failed;
}

void nyinx_io_devnull_folder_iterator_close(nyio_iterator_t*) {
}

nyio_err_t nyinx_io_devnull_file_exists(nyio_adapter_t*, const char*, uint32_t) {
	return nyioe_access;
}

} // namespace

extern "C" void nyio_adapter_init_devnull(nyio_adapter_t* adapter) {
	if (unlikely(!adapter))
		return;
	memset(adapter, 0x0, sizeof(nyio_adapter_t));
	adapter->stat = nyinx_io_devnull_stat;
	adapter->statex = nyinx_io_devnull_statex;
	adapter->file_read = nyinx_io_devnull_file_read;
	adapter->file_write = nyinx_io_devnull_file_write;
	adapter->file_open = nyinx_io_devnull_file_open;
	adapter->file_close = nyinx_io_devnull_file_close;
	adapter->file_seek_from_end = nyinx_io_devnull_file_seek;
	adapter->file_seek = nyinx_io_devnull_file_seek_set;
	adapter->file_seek_cur = nyinx_io_devnull_file_seek;
	adapter->file_tell = nyinx_io_devnull_file_tell;
	adapter->file_flush = nyinx_io_devnull_file_flush;
	adapter->file_eof = nyinx_io_devnull_file_eof;
	adapter->file_size = nyinx_io_devnull_file_size;
	adapter->file_resize = nyinx_io_devnull_file_resize;
	adapter->file_erase = nyinx_io_devnull_file_erase;
	adapter->file_exists = nyinx_io_devnull_file_exists;
	adapter->file_get_contents = nyinx_io_devnull_file_get_contents;
	adapter->file_set_contents = nyinx_io_devnull_file_set_contents;
	adapter->file_append_contents = nyinx_io_devnull_file_append_contents;
	adapter->folder_create = nyinx_io_devnull_folder_create;
	adapter->folder_erase = nyinx_io_devnull_folder_erase;
	adapter->folder_clear = nyinx_io_devnull_folder_clear;
	adapter->folder_size = nyinx_io_devnull_folder_size;
	adapter->folder_exists = nyinx_io_devnull_folder_exists;
	adapter->folder_iterate = nyinx_io_devnull_folder_iterate;
	adapter->folder_next = nyinx_io_devnull_folder_next;
	adapter->folder_iterator_type = nyinx_io_devnull_folder_iterator_type;
	adapter->folder_iterator_name = nyinx_io_devnull_folder_iterator_name;
	adapter->folder_iterator_size = nyinx_io_devnull_folder_iterator_size;
	adapter->folder_iterator_fullpath = nyinx_io_devnull_folder_iterator_fullpath;
	adapter->folder_iterator_close = nyinx_io_devnull_folder_iterator_close;
}
