 /*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANY_NANY_C_H__
#define __LIBNANY_NANY_C_H__
#include <string.h>
#include <stdint.h>


#if defined(_WIN32) || defined(__CYGWIN__)
#	ifdef __GNUC__
#		define LIBNANY_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#		define LIBNANY_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#	else
#		define LIBNANY_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#		define LIBNANY_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#	endif
#else
#	define LIBNANY_VISIBILITY_EXPORT       __attribute__((visibility("default")))
#	define LIBNANY_VISIBILITY_IMPORT       __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBNANY_DLL_EXPORT)
# define LIBNANY_DLL_EXPORT
#endif


#if defined(__clang__) || defined(__GNUC__)
# define LIBNANY_ATTR_ALLOCSIZE(A)    __attribute__((alloc_size(A)))
# define LIBNANY_ATTR_ALLOCSIZE2(A,B) __attribute__((alloc_size(A, B)))
#else
# define LIBNANY_ATTR_ALLOCSIZE(A)
# define LIBNANY_ATTR_ALLOCSIZE2(A, B)
#endif



/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBNANY_DLL_EXPORT)
#	define NY_EXPORT   LIBNANY_VISIBILITY_EXPORT
#else
#	define NY_EXPORT   LIBNANY_VISIBILITY_IMPORT
#endif








#ifdef __cplusplus
extern "C" {
#endif


/*! \name Information about libnany */
/*@{*/
/*! Export information about nany for bug reporting (must be released by free())*/
NY_EXPORT char* nany_get_info_for_bugreport();

/*! Get the version of nany (string, ex: 3.2.1-beta+2e738ae) */
NY_EXPORT const char* nany_version();
/*! Get the version metadata (ex: 2e738ae, can be null or empty) */

NY_EXPORT const char* nany_version_metadata();

/*! Get the pre-release version (ex: beta, can be null or empty) */
NY_EXPORT const char* nany_version_prerelease();

/*! Get the version of nany */
NY_EXPORT int nany_get_version(int* major, int* minor, int* patch);

/*! Get the nany's website */
NY_EXPORT const char* nany_website_url();
/*@}*/









/*! nybool_t */
typedef enum nybool_t {nyfalse, nytrue} nybool_t;


/*! Task status */
enum nytask_status_t
{
	/*! The task has failed */
	nys_failed = 0,
	/*! The task has succeeded */
	nys_succeeded,
	/*! The task is still running */
	nys_running,
	/*! The task is waiting for being launched */
	nys_idle,
	/*! The task has been canceled */
	nys_canceled,
};
typedef enum nytask_status_t  nytask_status_t;


/*!
** \brief Nany builtin types
*/
typedef enum /* nytype_t */
{
	/*! No type */
	nyt_void = 0,
	/*! Custom user type */
	nyt_any,
	/*! Raw pointer (arch dependent) */
	nyt_pointer,
	/*! Boolean (nytrue/nyfalse) */
	nyt_bool,
	/*! Unsigned 8  bits integer */
	nyt_u8,
	/*! Unsigned 16 bits integer */
	nyt_u16,
	/*! Unsigned 32 bits integer */
	nyt_u32,
	/*! Unsigned 64 bits integer */
	nyt_u64,
	/*! Signed 8  bits integer */
	nyt_i8,
	/*! Signed 16 bits integer */
	nyt_i16,
	/*! Signed 32 bits integer */
	nyt_i32,
	/*! Signed 64 bits integer */
	nyt_i64,
	/*! Floating-point number 32 bits */
	nyt_f32,
	/*! Floating-point number 64 bits */
	nyt_f64,

	/*! The total number of intrinsic types */
	nyt_count,

} nytype_t;



/*!
** \brief Type of a target (set of source files)
*/
typedef enum /* nytarget_type_t */
{
	/*! Unknown target */
	nyg_unknown  = 0,
	/*! target built as a module */
	nyg_module,
	/*! target built as a shared library */
	nyg_library,
	/*! target built as a standalone program */
	nyg_program,

} nytarget_type_t;



/*!
** \brief Identifiers' visibility
**
** \internal All values are strictly ordered
*/
typedef enum /* nyvisibility_t */
{
	/*! no valid visibility */
	nyv_undefined,
	/*! default: public or internal, according the context */
	nyv_default,
	/*! private: accessible only by the class */
	nyv_private,
	/*! protected: accessible only by the class and all derived classes */
	nyv_protected,
	/*! internal: accessible only from the correspondig target */
	nyv_internal,
	/*! public: accessible by everyone */
	nyv_public,
	/*! published: same as public, but accessible from an IDE */
	nyv_published,
	/*! dummy */
	nyv_max

} nyvisibility_t;



/*! \name Opaque Types */
/*@{*/
/*! Context for nany code compilation/execution */
typedef struct nycontext_t nycontext_t;
/*! Opaque structure to a target (executable or static/shared library) */
typedef struct nytarget_t  nytarget_t;
/*! Opaque structure to a source */
typedef struct nysource_t  nysource_t;
/*! Opaque structure for reporting */
typedef struct nyreport_t  nyreport_t;
/*! Opaque structure for object */
typedef struct nyobject_t  nyobject_t;

/*! Nany compiled program */
typedef struct nyprogram_t nyprogram_t;


/*! Opaque structure for queueservice */
typedef struct nyqueueservice_t nyqueueservice_t;
/*! Opaque structure for a single task */
typedef struct nytask_t nytask_t;
/*@}*/


/*! File handle */
typedef void* nyfd_t;

/*! File attributes */
typedef uint32_t nyio_flags_t;

/*! Flags for file attributes */
enum
{
	/*! Read access */
	nyio_flag_read                  = (1 << 0),
	/*! Write access */
	nyio_flag_write                 = (1 << 1),
	/*! Try to create the file if it does not exist */
	nyio_flag_create_if_not_exist   = (1 << 4),
	/*! Append content instead of rewrite */
	nyio_flag_append                = (1 << 5),
	/*! Reset the size of the file to 0 */
	nyio_flag_truncate              = (1 << 6),
};


/*!
** \brief Error levels
**
** \internal All values are strictly ordered
*/
typedef enum /* nyerr_level_t */
{
	/*! ICE (Internal Compiler Error) */
	nyee_ice,
	/*! Error level */
	nyee_error,
	/*! Warning level */
	nyee_warning,

	/*! Hint level */
	nyee_hint,
	/*! Suggestion level */
	nyee_suggest,
	/*! Info level */
	nyee_info,

	/*! Verbose level */
	nyee_verbose,
	/*! Debug level */
	nyee_trace = 0,
	/*! no message */
	nyee_none,

} nyerr_level_t;



/*! Intrnsic attributes */
typedef uint32_t nybind_flag_t;

/*! Flags for intrinsic attributes */
enum
{
	/*! Default settings */
	nybind_default = 0,
};











typedef struct nycontext_t nycontext_t;

typedef struct nytctx_t nytctx_t;

/*!
** \brief Nany Thread Context
*/
typedef struct nytctx_t
{
	/*! Parent context */
	nycontext_t* context;

	/*! event: a new thread is destroyed */
	void (*on_thread_destroy)(nytctx_t*);

	/*! Opaque internal data */
	void* internal;
}
nytctx_t;



typedef struct nycontext_memory_t
{
	/*! Allocates some memory */
	void* (*allocate)(nycontext_t*, size_t);
	/*! Re-allocate */
	void* (*reallocate)(nycontext_t*, void* ptr, size_t oldsize, size_t newsize);
	/*! free */
	void (*release)(nycontext_t*, void* ptr, size_t);

	/*! Memory usage limit (in bytes) */
	size_t limit_mem_size;

	/*! event: not enough memory */
	void (*on_not_enough_memory)(nycontext_t*, nybool_t limit_reached);
}
nycontext_memory_t;



/*!
** \brief Nany context
*/
typedef struct nycontext_t
{
	/* -- runtime hooks -- */
	/*! User data */
	void* userdata;

	/*! Memory management */
	nycontext_memory_t  memory;

	struct io_t
	{
		/*! Open a new handle to a given filename */
		nyfd_t (*open)(nycontext_t*, const char* filename, size_t length, uint32_t flags);
		/*! Close a handle */
		void (*close)(nycontext_t*, nyfd_t fd);
		/*! Read */
		size_t (*read)(nycontext_t*, nyfd_t fd, uint8_t* buffer, size_t size);
		/*! Write */
		size_t (*write)(nycontext_t*, nyfd_t fd, uint8_t* buffer, size_t size);
		/*! chroot path for opening files (final slash required if non-null) */
		char* chroot_path;
		/*! chroot path size (can be null to disable chroot) */
		size_t chroot_path_size;
	}
	io;

	struct console_t
	{
		/*! Write some data to STDOUT */
		void (*write_stdout)(nycontext_t*, const char* text, size_t length);
		/*! Write some data to STDERR */
		void (*write_stderr)(nycontext_t*, const char* text, size_t length);
		/*! Flush STDOUT */
		void (*flush_stdout)(nycontext_t*);
		/*! Flush STDERR */
		void (*flush_stderr)(nycontext_t*);
	}
	console;


	struct mt_t
	{
		/*! queueservice for concurrent jobs */
		nyqueueservice_t* queueservice;

		/*! Maximum number of threads (0 means 'disabled') */
		uint32_t limit_thread_count;
		/*! Maximum number of tasks (0 means 'disabled') */
		uint32_t limit_task_count;

		/*! event: a new thread is about to be created */
		nybool_t (*on_task_create_query)(nycontext_t*, nytask_t* parent);
		/*! event: a new thread is created */
		void (*on_task_create)(nycontext_t*, nytask_t*, nytask_t* parent);
		/*! event: a new thread is destroyed */
		void (*on_task_destroy)(nycontext_t*, nytask_t*);

		/*! event: a new thread is about to be created */
		nybool_t (*on_thread_create_query)(nycontext_t*, nytctx_t* parent);
		/*! event: a new thread is created */
		void (*on_thread_create)(nytctx_t* thread, nytctx_t* parent);
		/*! event: a new thread is destroyed */
		void (*on_thread_destroy)(nytctx_t*);
	}
	mt;


	struct program_t
	{
		/*! event: the program has started */
		nybool_t (*on_start)(nycontext_t*, int argc, const char** argv);
		/*! event: the program has stopped */
		int (*on_stop)(nycontext_t*, int exitcode);
		/*! event: the program has encountered a runtime error */
		int (*on_failed)(nycontext_t*, int exitcode);
	}
	program;


	struct build_t
	{
		/*! event: a context is about to be created (from another one) */
		nybool_t (*on_context_create_query)(const nycontext_t* ctxtemplate);
		/*! event: a context has been created (from another one) */
		void (*on_context_create)(nycontext_t*, const nycontext_t* ctxtemplate);
		/*! event: a context is about to be destroyed */
		void (*on_context_destroy)(nycontext_t*);

		/*! event: a build is about to begin */
		nybool_t (*on_build_query)(nycontext_t*);
		/*! event: a build has begun */
		void (*on_build_begin)(nycontext_t*);
		/*! event: a build has ended (report will be null if empty) */
		void (*on_build_end)(nycontext_t*, nybool_t success, const nyreport_t* report, int64_t duration_ms);

		/*! event: load-on-demand intrinsic not already bound */
		nybool_t (*on_intrinsic_discover)(nycontext_t*, const char* name, size_t);

		/*! event: failed to load source file */
		void (*on_err_file_access)(nycontext_t*, const char* filename, size_t length);
	}
	build;

	/*! Opaque internal data */
	void* internal;

	/*! Special values that may not be used directly but are here for performance reasons */
	struct nycontext_reserved_t {
		volatile size_t mem0;
	}
	reserved;
}
nycontext_t;







/*!
** \name Contexts
**
** \note Contexts are not thread-safe (for performance reasons).
**  If concurrent access to a context is required, please consider
**  `nany_lock` and `nany_unlock` routines
*/
/*@{*/
/*!
** \brief Create and initialize a new context
**
** \param inherit A context to inherit from (can be NULL)
** \param allocator Custom allocator functions (can be NULL)
** \see nany_dispose()
*/
NY_EXPORT nycontext_t*
nany_create(const nycontext_t* inherit, const nycontext_memory_t* allocator);
/*!
** \brief Release a context previously created by nany_create
** \param ctx A context (can be null) to release. The pointer will be set to NULL
** \see nany_create()
*/
NY_EXPORT void nany_dispose(nycontext_t** ctx);

/*!
** \brief Initialize a context already allocated
**
** This method can be used with the context is allocated on the stack
** \param inherit A context to inherit from (can be NULL)
** \param allocator Custom allocator functions (can be NULL)
**
** \note All internal resources are not created immediatly (since it may
** require more time or more memory for nothing)
** \note The event on_context_create_query will not be triggered
*/
NY_EXPORT nybool_t
nany_initialize(nycontext_t*, const nycontext_t* inherit, const nycontext_memory_t* allocator);
/*!
** \brief Uninitialize a context (without freeing the memory)
**
** This method can be used with the context is stack allocated on the stack
** \param inherit A context to inherit from (can be NULL)
*/
NY_EXPORT void nany_uninitialize(nycontext_t*);




/*!
** \nbrief Set to the default C memory allocator
*/
NY_EXPORT void nany_memalloc_init_default(nycontext_memory_t*);

/*!
** \nbrief Set to the default C memory allocator with
*/
NY_EXPORT void nany_mem_alloc_init_with_limit(nycontext_memory_t*, size_t limit);






/*!
** \brief Build all sources of a given context
**
** \param ctx A nany context
** \param report A log report (may be null)
*/
NY_EXPORT nybool_t nany_build(nycontext_t* ctx, nyreport_t** report);

/*!
** \brief Run the func 'main'
**
** \note 'ctx->argc' and 'ctx->argv' must be properly set
** \note The context must be built *before*
** \return The exit code. INT_MIN if any error has occured (no context, compilation has failed...)
*/
NY_EXPORT int nany_run_main(nycontext_t* ctx, int argc, const char** argv);


/*! Lock a valid context */
NY_EXPORT void nany_lock(const nycontext_t*);
/*! Unlock a valid context */
NY_EXPORT void nany_unlock(const nycontext_t*);
/*! Try to lock a valid context */
NY_EXPORT nybool_t nany_trylock(const nycontext_t*);
/*@}*/



/*! \name Sources */
/*@{*/
/*! Add a source file */
inline nybool_t nany_source_add_from_file(nycontext_t*, const char* const filename);
/*! Add a source file */
NY_EXPORT nybool_t nany_source_add_from_file_n(nycontext_t*, const char* const filename, size_t len);

/*! Add a source */
inline nybool_t nany_source_add(nycontext_t*, const char* const text);
/*! Add a source */
NY_EXPORT nybool_t nany_source_add_n(nycontext_t*, const char* const text, size_t len);
/*@}*/



/*! \name Reporting */
/*@{*/
/*!
** \brief Error level
*/
/*! Create a new empty report */
NY_EXPORT nyreport_t*  nany_report_create();

/*! Acquire a report (refcount) */
NY_EXPORT void nany_report_ref(nyreport_t* report);
/*! Release a report (refcount) */
NY_EXPORT void nany_report_unref(nyreport_t** report);

//! Add an info message
NY_EXPORT void nany_info(nyreport_t* report, const char* text);
//! Add a warning message
NY_EXPORT void nany_warning(nyreport_t* report, const char* text);
//! Add an error message
NY_EXPORT void nany_error(nyreport_t* report, const char* text);

/*! Print a report to stdout */
NY_EXPORT nybool_t nany_report_print_stdout(const nyreport_t* report);
/*! Print a report to stderr */
NY_EXPORT nybool_t nany_report_print_stderr(const nyreport_t* report);

/*! Print information about the nany library */
NY_EXPORT nybool_t nany_report_add_compiler_headerinfo(nyreport_t* report);
/*@}*/




/*! \name Intrinsic bindings */
/*@{*/
/*!
** \brief Bind a C function
**
** \code
** static double square(nytctx_t*, uint32_t value)
** {
**	return value * value;
** }
**
** int main()
** {
**	nycontext_t ctx;
**	nany_initialize(&ctx);
**	// ...
**	nany_bind(&ctx, "square", nybind_default, square, nyt_f64, nyt_u32);
**	// ...
**	nany_uninitialize(&ctx);
**	return 0;
** }
** \endcode
**
** \note 'extern "C"' is required only for C++ code
**
** \param name Name of the intrinsic (ex: 'myprogram.hello.world')
** \param flags List of flags (see nybind_flag_t). nybind_default or 0 if unsure
** \param callback C callback at runtime
** \param ret The return type
** \param ret... List of parameter types, followed by 'nyt_void'
** \return nytrue if the operation succeeded
*/
NY_EXPORT nybool_t
nany_bind(nycontext_t*, const char* name, nybind_flag_t flags, void* callback, nytype_t ret, ...);

/*!
** \brief Bind a C function
**
** \code
** static double square(nytctx_t*, uint32_t value)
** {
**	return value * value;
** }
**
** int main()
** {
**	nycontext_t ctx;
**	nany_initialize(&ctx);
**	// ...
**	nany_bind(&ctx, "square", nybind_default, square, nyt_f64, nyt_u32);
**	// ...
**	nany_uninitialize(&ctx);
**	return 0;
** }
** \endcode
**
** \note 'extern "C"' is required only for C++ code
**
** \param name Name of the intrinsic (ex: 'myprogram.hello.world')
** \param length Length of the intrinsic name
** \param flags List of flags (see nybind_flag_t). nybind_default or 0 if unsure
** \param callback C callback at runtime
** \param ret The return type
** \param ret... List of parameter types, followed by 'nyt_void'
** \return nytrue if the operation succeeded
*/
NY_EXPORT nybool_t
nany_bind_n(nycontext_t*, const char* name, size_t length, nybind_flag_t flags, void* callback, nytype_t ret, ...);
/*@}*/



/*! \name Concurrent Tasks */
/*@{*/
/*!
** \brief Create a new queue service
**
** \return A new queue service (if not null), which must be released by `nany_queueservice_release()`
*/
NY_EXPORT nyqueueservice_t* nany_queueservice_create();

/*! Acquire a queueservice (refcount) */
NY_EXPORT void nany_queueservice_ref(nyqueueservice_t* queue);
/*! Release a queueservice (refcount) */
NY_EXPORT void nany_queueservice_unref(nyqueueservice_t** queue);

/*! Cancel all taks and stop a queueservice */
NY_EXPORT void nany_queueservice_stop(nyqueueservice_t*);

/*! Ask to stop gracefully a task */
NY_EXPORT void nany_cancel(nytask_t*);

/*! Wait for the completion of a task */
NY_EXPORT nytask_status_t nany_wait(nytask_t*);
/*! Wait for the completion of a task (with a timeout in ms) */
NY_EXPORT nytask_status_t nany_wait_timeout(nytask_t*, uint32_t ms);

/*! Wait for the completion of a task (and cancel it if timeout is reached) */
NY_EXPORT nytask_status_t nany_wait_or_cancel(nytask_t*, uint32_t ms);

/*! Acquire a task (refcount) */
NY_EXPORT void nany_task_ref(nytask_t* task);
/*! Release a task (refcount) */
NY_EXPORT void nany_task_unref(nytask_t** task);
/*@}*/



/*! \name Utilities */
/*@{*/
/*!
** \brief Print the AST of a nany source file
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
*/
inline nybool_t nany_print_ast_from_file(const char* filename, int fd, nybool_t unixcolors);

/*!
** \brief Print the AST of a nany source file
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
*/
NY_EXPORT nybool_t nany_print_ast_from_file_n(const char* filename, size_t length, int fd, nybool_t unixcolors);

/*!
** \brief Print the AST of some nany code in memory
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
** \param content Arbitrary utf-8 content (c-string)
*/
inline nybool_t nany_print_ast_from_memory(const char* content, int fd, nybool_t unixcolors);
/*!
** \brief Print the AST of some nany code in memory
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
** \param content Arbitrary utf-8 content (c-string)
*/
NY_EXPORT  nybool_t nany_print_ast_from_memory_n(const char* content, size_t length, int fd, nybool_t unixcolors);

/*!
** \brief Check if a filename is a valid nany source code
**
** \param filename An arbitrary filename (utf-8 c-string)
** \return nytrue if the file has been successfully parsed, false otherwise
*/
inline nybool_t nany_utility_validator_check_file(const char* filename);
/*!
** \brief Check if a filename is a valid nany source code
**
** \param filename An arbitrary filename (utf-8 c-string)
** \param length Length of the filename
** \return nytrue if the file has been successfully parsed, false otherwise
*/
NY_EXPORT nybool_t nany_utility_validator_check_file_n(const char* filename, size_t length);


/*!
** \brief Convert a C-String representing a visibility level
**
** An empty value will represent a "default" visibility (nyv_undefined)
*/
inline nyvisibility_t  nany_cstring_to_visibility(const char* text);
/*!
** \brief Convert a C-String representing a visibility level (with given length)
**
** An empty value will represent a "default" visibility (nyv_undefined)
*/
NY_EXPORT nyvisibility_t  nany_cstring_to_visibility_n(const char* text, size_t length);

/*!
** \brief Convert a visibility to a C-String representation
*/
NY_EXPORT const char* nany_visibility_to_cstring(nyvisibility_t);


/*!
** \brief Convert a string into the builtin type
**
** \param text An arbitrary text (ex: "__uint64")
** \return The corresponding type (ex: nyt_uint64)
*/
inline nytype_t  nany_cstring_to_type(const char* text);

/*!
** \brief Convert a string into the builtin type (with length provided)
**
** \param text An arbitrary text (ex: "__uint64")
** \return The corresponding type (ex: nyt_uint64)
*/
NY_EXPORT nytype_t  nany_cstring_to_type_n(const char* text, size_t length);

/*!
** \brief Convert a type into a c-string
*/
NY_EXPORT const char* nany_type_to_cstring(nytype_t);

NY_EXPORT size_t  nany_type_sizeof(nytype_t);
/*@}*/










/*! \name context callbacks (default implementation) */
/*@{*/
NY_EXPORT void nanysdbx_write_stdout(nycontext_t*, const char* text, size_t length);
NY_EXPORT void nanysdbx_write_stderr(nycontext_t*, const char* text, size_t length);
NY_EXPORT void nanysdbx_flush_stdout(nycontext_t*);
NY_EXPORT void nanysdbx_flush_stderr(nycontext_t*);
NY_EXPORT nyfd_t nanysdbx_io_open(nycontext_t*, const char* path, size_t length, uint32_t flags);
NY_EXPORT void nanysdbx_io_close(nycontext_t*, nyfd_t fd);
NY_EXPORT size_t nanysdbx_io_read(nycontext_t*, nyfd_t fd, uint8_t* buffer, size_t size);
NY_EXPORT size_t nanysdbx_io_write(nycontext_t*, nyfd_t fd, uint8_t* buffer, size_t size);

NY_EXPORT void  nanysdbx_not_enough_memory(nycontext_t*, nybool_t);

NY_EXPORT void nanysdbx_build_on_err_file_access(nycontext_t*, const char* filename, size_t length);
/*@}*/


#ifdef __cplusplus
}
#endif

#include "nany.hxx" // inline functions

#endif /* __LIBNANY_NANY_C_H__ */
