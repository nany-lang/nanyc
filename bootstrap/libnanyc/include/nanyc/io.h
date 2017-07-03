/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_IO_H__
#define __LIBNANYC_IO_H__

#include <nanyc/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nyio_err_t {
	/*! Success */
	nyioe_ok = 0,
	/*! Generic unknown error */
	nyioe_failed,
	/*! Operation not supported by the adapter */
	nyioe_unsupported,
	/*! Failed to allocate memory */
	nyioe_memory,
	/*! Do not exist or not enough permissions */
	nyioe_access,
	/*! Already exists */
	nyioe_exists,
}
nyio_err_t;

typedef enum nyio_type_t {
	nyiot_failed = 0,
	nyiot_file,
	nyiot_folder,
}
nyio_type_t;

/*! IO Adapter */
typedef struct nyio_adapter_t nyio_adapter_t;

typedef struct nyio_iterator_t nyio_iterator_t;

/*!
** \brief Adapter for a filesystem
**
** \warning The implementation MUST consider that input strings are NOT zero-terminated
*/
struct nyio_adapter_t {
	/*! Internal opaque pointer */
	void* internal;
	/*! Value considered as invalid file descriptor */
	void* invalid_fd;

	/*! Stat a node */
	nyio_type_t (*stat)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Stat a node */
	nyio_type_t (*statex)(nyio_adapter_t*, const char* path, uint32_t len, uint64_t* size, int64_t* modified);

	/*! Read content from a file */
	uint64_t (*file_read)(void*, void* buffer, uint64_t bufsize);
	/*! Write content to an opened file */
	uint64_t (*file_write)(void*, const void* buffer, uint64_t bufsize);
	/*! Open a local file for the current thread */
	void* (*file_open)(nyio_adapter_t*, const char* path, uint32_t len,
		nybool_t readm, nybool_t writem, nybool_t appendm, nybool_t truncm);
	/*! Close a file */
	void (*file_close)(void*);
	/*! End of file */
	nybool_t (*file_eof)(void*);

	/*! Seek from the begining of the file */
	nyio_err_t (*file_seek)(void*, uint64_t offset);
	/*! Seek from the end of the file */
	nyio_err_t (*file_seek_from_end)(void*, int64_t offset);
	/*! Seek from the current cursor position */
	nyio_err_t (*file_seek_cur)(void*, int64_t offset);
	/*! Tell the current cursor position */
	uint64_t (*file_tell)(void*);

	/*! Flush a file */
	void (*file_flush)(void*);

	/*! Get the file size or the folder size (in bytes) */
	uint64_t (*file_size)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Get the file size or the folder size (in bytes) */
	nyio_err_t (*file_erase)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Get if a file exists */
	nyio_err_t (*file_exists)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Resize a file */
	nyio_err_t (*file_resize)(nyio_adapter_t*, const char* path, uint32_t len, uint64_t newsize);

	/*! Retrieve the content of a file */
	nyio_err_t (*file_get_contents)(nyio_adapter_t*, char** content, uint64_t* size, uint64_t* capacity,
		const char* path, uint32_t len);
	/*! Set the content of a file */
	nyio_err_t (*file_set_contents)(nyio_adapter_t*, const char* path, uint32_t len, const char* content,
		uint32_t ctlen);
	/*! Append the content to a file */
	nyio_err_t (*file_append_contents)(nyio_adapter_t*, const char* path, uint32_t len, const char* content,
		uint32_t ctlen);


	/*! Create a new folder */
	nyio_err_t (*folder_create)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Delete a folder and all its content */
	nyio_err_t (*folder_erase)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Delete the contents of a folder */
	nyio_err_t (*folder_clear)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Get the folder size (in bytes) */
	uint64_t (*folder_size)(nyio_adapter_t*, const char* path, uint32_t len);
	/*! Get if a folder exists */
	nyio_err_t (*folder_exists)(nyio_adapter_t*, const char* path, uint32_t len);

	/*! Iterate through folder */
	nyio_iterator_t* (*folder_iterate)(nyio_adapter_t*, const char* path, uint32_t len,
		nybool_t recursive, nybool_t files, nybool_t folders);
	/*! Go to the next element */
	nyio_iterator_t* (*folder_next)(nyio_iterator_t*);
	/*! Get the full path (i.e. /baz/foo,txt) of the current element */
	const char* (*folder_iterator_fullpath)(nyio_adapter_t*, nyio_iterator_t*);
	/*! Get the filename (i.e. foo.txt) of the current element */
	const char* (*folder_iterator_name)(nyio_iterator_t*);
	/*! Get the size in bytes of the current element */
	uint64_t (*folder_iterator_size)(nyio_iterator_t*);
	/*! Get the type of the current element */
	nyio_type_t (*folder_iterator_type)(nyio_iterator_t*);
	/*! Close an iterator */
	void (*folder_iterator_close)(nyio_iterator_t*);

	/*! Clone the adapter, most likely for a new thread */
	void (*clone)(nyio_adapter_t* parent, nyio_adapter_t* dst);
	/*! Release the adapter */
	void (*release)(nyio_adapter_t*);
};

/*!
** \brief Create an adapter to access to a local folder
*/
NY_EXPORT void nyio_adapter_create_from_local_folder(nyio_adapter_t*, const char*, size_t);

#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_IO_H__ */
