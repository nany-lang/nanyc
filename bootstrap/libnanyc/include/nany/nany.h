/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_NANY_C_H__
#define __LIBNANYC_NANY_C_H__
#include "../nanyc/types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! \name Types */
/*@{*/
#ifndef LIBNANYC_NYANYSTR_T
#define LIBNANYC_NYANYSTR_T
/*! Create an nyanystr_t from a c-string */
static inline nyanystr_t nyanystr(const char* const text) {
	nyanystr_t s = {((text) ? (uint32_t) strlen(text) : 0), text};
	return s;
}

/*! Create an nyanystr_t from a c-string and a given length */
static inline nyanystr_t nyanystr_ex(const char* const text, uint32_t len) {
	nyanystr_t s = {len, text};
	return s;
}

/*! Create an nyanystr_t from a reallocated c-string */
static inline void nyanystr_duplicate(nyanystr_t* const out, const nyanystr_t* const src) {
	uint32_t len = src->size;
	const char* srcstr = src->c_str;
	char* str = (char*) malloc(sizeof(char) * (len + 1));
	memcpy(str, srcstr, len);
	str[len] = '\0';
	out->c_str = str;
	out->size = len;
}
#endif



/*! Opaque Thread Object */
typedef struct nythread_t nythread_t;
/*! Opaque Project Object */
typedef struct nyproject_t nyproject_t;
/*! Opaque Target Object */
typedef struct nytarget_t nytarget_t;
/*! Opaque structure to a source */
typedef struct nysource_t nysource_t;
/*! Build */
typedef struct nybuild_t nybuild_t;
/*! VM Program */
typedef struct nyprogram_t nyprogram_t;
/*! VM Thread Context */
typedef struct nythread_t nytctx_t;
/*@}*/


/*! \name Memory allocator */
/*@{*/
struct nyallocator_t;

typedef struct nyallocator_cf_t {
	void* userdata;
	/*! Create a multi-threaded allocator */
	nybool_t (*create_mt)(struct nyallocator_t*, const struct nyallocator_cf_t*);
	/*! Create a single-threaded allocator */
	nybool_t (*create_st)(struct nyallocator_t*, const struct nyallocator_cf_t*);
	/*! event: not enough memory */
	void (*on_not_enough_memory)(struct nyallocator_t*, nybool_t limit_reached);
}
nyallocator_cf_t;

typedef struct nyallocator_t {
	void* userdata;
	/*! Allocates some memory */
	void* (*allocate)(struct nyallocator_t*, size_t);
	/*! Re-allocate */
	void* (*reallocate)(struct nyallocator_t*, void* ptr, size_t oldsize, size_t newsize);
	/*! free */
	void (*deallocate)(struct nyallocator_t*, void* ptr, size_t);
	/*! Special values that may not be used directly but are here for performance reasons */
	volatile size_t reserved_mem0;
	/*! Memory usage limit (in bytes) */
	size_t limit_mem_size;
	/*! event: not enough memory */
	void (*on_not_enough_memory)(struct nyallocator_t*, nybool_t limit_reached);
	/*! event: callback for aborting program execuion */
	void (*on_internal_abort)(struct nyallocator_t*);
	/*! Flush STDERR */
	void (*release)(struct nyallocator_t*);
}
nyallocator_t;


NY_EXPORT void nyallocator_cf_init(nyallocator_cf_t*);

/*! Set callbacks to the standard C memory allocator */
NY_EXPORT void nany_memalloc_set_default(nyallocator_t*);
/*! Set callbacks to the std C memory allocator with bounds checking */
NY_EXPORT void nany_memalloc_set_with_limit(nyallocator_t*, size_t limit);
/*! Copy allocator */
void nany_memalloc_copy(nyallocator_t* out, const nyallocator_t* const src);
/*@}*/




/*! \name Console */
/*@{*/
typedef enum nyconsole_output_t {
	/*! Print to the cout */
	nycout = 1,
	/*! Print to the cerr */
	nycerr = 2,
}
nyconsole_output_t;


/*! Color constants */
typedef enum nyoldcolor_t {
	/*! None / reset */
	nycold_none = 0,
	/*! Black */
	nycold_black,
	/*! Red */
	nycold_red,
	/*! Green */
	nycold_green,
	/*! Yellow */
	nycold_yellow,
	/*! Dark Blue */
	nycold_blue,
	/*! Purple */
	nycold_purple,
	/*! Gray */
	nycold_gray,
	/*! White */
	nycold_white,
	/*! Light blue */
	nycold_lightblue
}
nyoldcolor_t;

enum {
	/*! The total number of colors */
	nycold_count = nycold_lightblue + 1
};


typedef struct nyoldconsole_t {
	/*! Write some data to STDOUT */
	void (*write_stdout)(void*, const char* text, size_t length);
	/*! Write some data to STDERR */
	void (*write_stderr)(void*, const char* text, size_t length);
	/*! Flush output */
	void (*flush)(void*, nyconsole_output_t);
	/*! Set the text color */
	void (*set_color)(void*, nyconsole_output_t, nyoldcolor_t);
	/*! Set the text color */
	nybool_t (*has_color)(void*, nyconsole_output_t);

	/*! Internal opaque pointer*/
	void* internal;
	/*! Flush STDERR */
	void (*release)(const struct nyoldconsole_t*);
}
nyoldconsole_t;

/*! Initialize a project configuration */
NY_EXPORT void nyconsole_cf_set_stdcout(nyoldconsole_t*);

void nyconsole_cf_copy(nyoldconsole_t* out, const nyoldconsole_t* const src);
/*@}*/






/*! \name Project management */
/*@{*/
/*! Project Configuration */
typedef struct nyproject_cf_t {
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! A new target has been added */
	void (*on_target_added)(nyproject_t*, nytarget_t*, const char* name, uint32_t len);
	/*! A target has been removed */
	void (*on_target_removed)(nyproject_t*, nytarget_t*, const char* name, uint32_t len);
	/*! Load unittsts from nsl */
	nybool_t with_nsl_unittests;
}
nyproject_cf_t;


/*!
** \brief Create a new nany project
**
** \param cf Configuration (can be null)
** \return A ref-counted pointer to the new project. NULL if the operation failed. The returned
**   object must be released by `nyproject_unref`
*/
NY_EXPORT nyproject_t* nyproject_create(const nyproject_cf_t* cf);

/*!
** \brief Acquire a project
** \param project Project pointer (can be null)
*/
NY_EXPORT void nyproject_ref(nyproject_t* project);

/*!
** \brief Unref a project and destroy it if required
** \param project A Project pointer (can be null)
*/
NY_EXPORT void nyproject_unref(nyproject_t* project);

/*! Initialize a project configuration */
NY_EXPORT void nyproject_cf_init(nyproject_cf_t*);


/*! Add a source file to the default target */
NY_EXPORT nybool_t nyproject_add_source_from_file(nyproject_t*, const char* filename);
/*! Add a source file to the default target (with filename length) */
NY_EXPORT nybool_t nyproject_add_source_from_file_n(nyproject_t*, const char* filename, size_t);

/*! Add a source to the default target */
NY_EXPORT nybool_t nyproject_add_source(nyproject_t*, const char* text);
/*! Add a source to the default target (with filename length) */
NY_EXPORT nybool_t nyproject_add_source_n(nyproject_t*, const char* text, size_t);


/*! Lock a project */
NY_EXPORT void nyproject_lock(const nyproject_t*);
/*! Unlock a project */
NY_EXPORT void nyproject_unlock(const nyproject_t*);
/*! Try to lock a project */
NY_EXPORT nybool_t nyproject_trylock(const nyproject_t*);
/*@}*/






/*! \name Build */
/*@{*/
/*! Project Configuration */
typedef struct nybuild_cf_t {
	/*! Console output */
	nyoldconsole_t console;

	/*! Entry point to compile (ex: "main") */
	nyanystr_t entrypoint;
	/*! Ignore atoms when parsing - do not produce any IR code for atoms */
	/* (can be used to only retrieve the list of atoms or unittests) */
	nybool_t ignore_atoms;

	/*! Userdata */
	void* userdata;

	/*! AN unittest has been found */
	void (*on_unittest)(void* userdata, const char* mod, uint32_t mlen, const char* name, uint32_t nlen);

	/*! Query if a new build can be started */
	nybool_t (*on_query)(const nyproject_t*);
	/*! A new build has been started */
	void (*on_begin)(const nyproject_t*, nybuild_t*);

	/*! Progress report */
	nybool_t (*on_progress)(const nyproject_t*, nybuild_t*, const char* id, const char* element,
							uint32_t percent);
	/*! Try to discover a new binding */
	nybool_t (*on_binding_discovery)(nybuild_t*, const char* name, uint32_t size);

	/*! A build has terminated */
	void (*on_end)(const nyproject_t*, nybuild_t*, nybool_t success);
	void (*on_error_file_eacces)(const nyproject_t*, nybuild_t*, const char* filename, uint32_t length);
}
nybuild_cf_t;



/*!
** \brief Create a new build
*/
NY_EXPORT nybuild_t* nybuild_prepare(nyproject_t*, const nybuild_cf_t*);

/*!
** \brief Build the project
*/
NY_EXPORT nybool_t nybuild(nybuild_t*);

/*!
** \brief Print the build report to the console
**
** \param build A build object (can be null, will do nothing)
** \param print_header nytrue to add information about the compiler
*/
NY_EXPORT void nybuild_print_report_to_console(nybuild_t* build, nybool_t print_header);

/*!
** \brief Acquire a build
** \param build build pointer (can be null)
*/
NY_EXPORT void nybuild_ref(nybuild_t* build);
/*!
** \brief Unref a build and destroy it if required
** \param build A build pointer (can be null)
*/
NY_EXPORT void nybuild_unref(nybuild_t* build);

/*! Initialize a project configuration */
NY_EXPORT void nybuild_cf_init(nybuild_cf_t* cf);
/*@}*/




/*! \name IO */
/*@{*/
/*! Opaque pointer to a file */
typedef struct nyfile_t nyfile_t;


typedef enum nyio_type_t {
	nyiot_failed = 0,
	nyiot_file,
	nyiot_folder,
}
nyio_type_t;


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


typedef enum nyio_automout_flag_t {
	/*! Automount all */
	nyioaf_all   = (uint32_t) - 1,
	/*! No automount */
	nyioaf_none  = 0,
	/*! Automount /root, to acces to the local filesystem */
	nyioaf_root  = (1 << 0),
	/*! Automount /tmp, temporary folder from the process point of view */
	nyioaf_tmp   = (1 << 1),
	/*! Automount /home, (/home/Desktop, /home/Documents...) */
	nyioaf_home  = (1 << 2),
	/*! Automount */
	nyioaf_users = (1 << 3),
}
nyio_automout_flag_t;




/*! Callback for iterating through the list of opened files */
typedef nybool_t (*nyio_opened_files_it_t)(const char* vpath, uint32_t len, const char* localpath,
	uint32_t lplen);


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


typedef struct nyio_cf_t {
	/*! event: an url has been mounted */
	nyio_err_t (*on_mount_query)(nyprogram_t*, nytctx_t*, const char* url, const char* path, uint32_t len);
	/*! event: create an adapter from an url */
	nyio_err_t (*on_adapter_create)(nyprogram_t*, nytctx_t*, nyio_adapter_t**, const char* url,
		nyio_adapter_t* parent);

	/*! Flag to automatically mount some standard paths */
	/*! \see nyio_automout_flag_t */
	uint32_t automount_flags;
}
nyio_cf_t;

/*!
** \brief Create an adapter to access to a local folder
*/
NY_EXPORT void nyio_adapter_create_from_local_folder(nyio_adapter_t*, nyallocator_t*,
	const char* localfolder, size_t len);
/*@}*/





/*! \name Program */
/*@{*/
typedef struct nybacktrace_entry_t {
	/*! Atom name (e.g. `func mynamespace.MyClass.foo(p1: Type1, p2: Type2): RetType`)*/
	const char* atom;
	/*! Source filename (utf8 - can be null) */
	const char* filename;
	/*! Length in bytes of the atom name (can be null) */
	uint32_t atom_size;
	/*! Length in bytes of the source filename (can be null) */
	uint32_t filename_size;
	/*! Line index (1-based, 0 if unknown) within the source file */
	uint32_t line;
	/*! Column index (1-based, 0 if unknown) within the source file for the given line */
	uint32_t column;
}
nybacktrace_entry_t;


/*! Program Configuration */
typedef struct nyprogram_cf_t {
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! Console output */
	nyoldconsole_t console;

	/*!
	** \brief A new program has been started
	** return nytrue to continue the execution, nyfalse to abort it
	*/
	nybool_t (*on_execute)(nyprogram_t*);

	/*!
	** \brief A new thread is created
	** return nytrue to continue the execution of the thread. nyfalse to abort
	*/
	nybool_t (*on_thread_create)(nyprogram_t*, nytctx_t*, nythread_t* parent, const char* name, uint32_t size);
	/*!
	** \brief A thread has been destroyed
	** \note This callback won't be called if `on_thread_create` failed
	*/
	void (*on_thread_destroy)(nyprogram_t*, nythread_t*);

	/*! Error has been received during the execution of the code */
	/* wip - void (*on_error)(const nyprogram_t*, const nybacktrace_entry_t** backtrace, uint32_t bt_len); */
	/*!
	** \brief The program is terminated
	** \note This callback won't be called if `on_execute` failed
	*/
	void (*on_terminate)(const nyprogram_t*, nybool_t error, int exitcode);

	/*! IO configuration */
	nyio_cf_t io;

	/*! Entry point to compile (ex: "main") */
	nyanystr_t entrypoint;
}
nyprogram_cf_t;


/*! Context at runtime for native C calls */
typedef struct nyoldvm_t {
	/*! Allocator */
	nyallocator_t* allocator;
	/*! Current program */
	nyprogram_t* program;
	/*! Current thread */
	nytctx_t* tctx;
	/*! Console */
	nyoldconsole_t* console;
}
nyoldvm_t;


/*!
** \brief Create a byte code program from a given build
**
** \param build A build
*/
NY_EXPORT nyprogram_t* nyprogram_prepare(nybuild_t* build, const nyprogram_cf_t* cf);

/*!
** \brief Execute a Nany program
**
** \param program Bytecode nany program
** \param argc Number of arguments (always >= 1)
** \param argv A null terminated list of arguments. The first argument is the full path
**    to the program/script. Arguments must use the UTF8 encoding
** \return Exit status code
*/
NY_EXPORT int nyprogram_main(nyprogram_t* program, uint32_t argc, const char** argv);


/*!
** \brief Acquire a program
** \param program program pointer (can be null)
*/
NY_EXPORT void nyprogram_ref(nyprogram_t* program);
/*!
** \brief Unref a program and destroy it if required
** \param program A program pointer (can be null)
*/
NY_EXPORT void nyprogram_unref(nyprogram_t* program);

/*! Initialize a project configuration */
NY_EXPORT void nyprogram_cf_init(nyprogram_cf_t* cf, const nybuild_cf_t*);
/*@}*/






/*! \name Utilities */
/*@{*/
/*!
** \brief Print the AST of a nany source file
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
*/
nybool_t nyprint_ast_from_file(const char* filename, int fd, nybool_t unixcolors);

/*!
** \brief Print the AST of a nany source file
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
*/
NY_EXPORT nybool_t nyprint_ast_from_file_n(const char* filename, size_t length, int fd, nybool_t unixcolors);

/*!
** \brief Print the AST of some nany code in memory
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
** \param content Arbitrary utf-8 content (c-string)
*/
NY_EXPORT nybool_t nyprint_ast_from_memory(const char* content, int fd, nybool_t unixcolors);
/*!
** \brief Print the AST of some nany code in memory
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
** \param content Arbitrary utf-8 content (c-string)
*/
NY_EXPORT  nybool_t nyprint_ast_from_memory_n(const char* content, size_t length, int fd,
	nybool_t unixcolors);

/*!
** \brief Check if a filename is a valid nany source code
**
** \param filename An arbitrary filename (utf-8 c-string)
** \return nytrue if the file has been successfully parsed, false otherwise
*/
NY_EXPORT nybool_t nytry_parse_file(const char* const filename);
/*!
** \brief Check if a filename is a valid nany source code
**
** \param filename An arbitrary filename (utf-8 c-string)
** \param length Length of the filename
** \return nytrue if the file has been successfully parsed, false otherwise
*/
NY_EXPORT nybool_t nytry_parse_file_n(const char* filename, size_t length);




/*! \name Convenient wrappers */
/*@{*/
typedef struct nyrun_cf_t {
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! Console */
	nyoldconsole_t console;
	/*! Default prject settings */
	nyproject_cf_t project;
	/*! Default build settings */
	nybuild_cf_t build;
	/*! Default program settings */
	nyprogram_cf_t program;
	/*! A non-zero value for verbose mode */
	nybool_t verbose;
}
nyrun_cf_t;

/*! Initialize a template object */
NY_EXPORT void nyrun_cf_init(nyrun_cf_t*);

/*! Release resources held by a template object */
NY_EXPORT void nyrun_cf_release(nyrun_cf_t*);
/*!
** \brief Compile & run a nany program
**
** \param cf A template for settings (can be null)
** \param source A nany program (can be null)
** \param argc The number of additional input arguments (ignored if <= 0)
** \param argv Input arguments (ignored if null)
** \return Exit status code
*/
NY_EXPORT int nyrun(const nyrun_cf_t* cf, const char* source, uint32_t argc, const char** argv);

/*!
** \brief Compile & run a nany program
**
** \param cf A template for settings (can be null)
** \param source A nany program (can be null)
** \param length Length in bytes of the nany program
** \param argc The number of additional input arguments (ignored if <= 0)
** \param argv Input arguments (ignored if null)
** \return Exit status code
*/
NY_EXPORT int nyrun_n(const nyrun_cf_t* cf, const char* source, size_t length, uint32_t argc,
	const char** argv);


/*!
** \brief Compile & run a nany script file
**
** \param cf A template for settings (can be null)
** \param file a filename (can be null)
** \param argc The number of additional input arguments (ignored if <= 0)
** \param argv Input arguments (ignored if null)
** \return Exit status code
*/
NY_EXPORT int nyrun_file(const nyrun_cf_t* cf, const char* file, uint32_t argc, const char** argv);

/*!
** \brief Compile & run a nany script file
**
** \param cf A template for settings (can be null)
** \param file a filename (can be null)
** \param length Length in bytes of the filename
** \param argc The number of additional input arguments (ignored if <= 0)
** \param argv Input arguments (ignored if null)
** \return Exit status code
*/
NY_EXPORT int nyrun_file_n(const nyrun_cf_t* cf, const char* file, size_t length, uint32_t argc,
	const char** argv);

NY_EXPORT int nyrun_filelist(const nyrun_cf_t* cf, const char** files, uint32_t file_count, uint32_t argc,
	const char** argv);
/*@}*/


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_NANY_C_H__ */
