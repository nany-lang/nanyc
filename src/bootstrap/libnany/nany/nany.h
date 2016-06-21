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
#  ifdef __GNUC__
#    define LIBNANY_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#    define LIBNANY_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#  else
#    define LIBNANY_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#    define LIBNANY_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#  endif
#else
#  define LIBNANY_VISIBILITY_EXPORT     __attribute__((visibility("default")))
#  define LIBNANY_VISIBILITY_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBNANY_DLL_EXPORT)
#  define LIBNANY_DLL_EXPORT
#endif


#if defined(__clang__) || defined(__GNUC__)
#  define LIBNANY_ATTR_ALLOCSIZE(A)    __attribute__((alloc_size(A)))
#  define LIBNANY_ATTR_ALLOCSIZE2(A,B) __attribute__((alloc_size(A, B)))
#else
#  define LIBNANY_ATTR_ALLOCSIZE(A)
#  define LIBNANY_ATTR_ALLOCSIZE2(A, B)
#endif

/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBNANY_DLL_EXPORT)
#	define NY_EXPORT LIBNANY_VISIBILITY_EXPORT
#else
#	define NY_EXPORT LIBNANY_VISIBILITY_IMPORT
#endif








#ifdef __cplusplus
extern "C" {
#endif

/*! Boolean type */
typedef enum nybool_t {nyfalse, nytrue} nybool_t;




/*! \name Information about libnany */
/*@{*/
/*!
** \brief Print information about nany for bug reporting
*/
NY_EXPORT void nany_print_info_for_bugreport();

/*!
** \brief Export information about nany for bug reporting
**
** \param[out] length Length of the returned c-string (can be null)
** \return A C-String, which must be released by `free (3)`. NULL if an error occured
*/
NY_EXPORT char* nany_get_info_for_bugreport(uint32_t* length);

/*!
** \brief Get the nany's website
*/
NY_EXPORT const char* nany_website_url();

/*!
** \brief Get the version of nany
**
** \param[out] major Major version (eX: 2.4.1 -> 2) (can be null)
** \param[out] major Minor version (eX: 2.4.1 -> 4) (can be null)
** \param[out] major Patch (eX: 2.4.1 -> 1) (can be null)
** \return The full version within a single integer (ex: 2.4.1 -> 204001)
*/
NY_EXPORT uint32_t nany_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch);

/*! Get the full version of nany (string, ex: 2.4.1-beta+2e738ae) */
NY_EXPORT const char* nany_version();
/*! Get the version metadata (ex: '2e738ae', can be null or empty) */
NY_EXPORT const char* nany_version_metadata();
/*! Get the pre-release version (ex: 'beta', can be null or empty) */
NY_EXPORT const char* nany_version_prerelease();

/*!
** \brief Check if the version is compatible with the library
**
** This function can be used to avoid unwanted behaviors when
** a program is able to use several versions of libnany
** \code
** if (nyfalse == nany_check_compatible_version(0, 2))
**     fprintf(stderr, "incompatible version\n");
** \endcode
*/
NY_EXPORT nybool_t nany_check_compatible_version(uint32_t major, uint32_t minor);
/*@}*/








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
	nyt_ptr,
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

} nytype_t;

enum {
	/*! The total number of intrinsic types */
	nyt_count = nyt_f64 + 1,
};





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

} nyvisibility_t;

enum {
	/*! The total number of visibility types */
	nyv_count = nyv_published + 1,
};




/*! Intrnsic attributes */
typedef uint32_t nybind_flag_t;

/*! Flags for intrinsic attributes */
enum {
	/*! Default settings */
	nybind_default = 0
};




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



/*! \name Memory allocator */
/*@{*/
typedef struct nyallocator_t
{
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

	/*! Flush STDERR */
	void (*release)(const struct nyallocator_t*);
}
nyallocator_t;


/*!
** \nbrief Switch to the standard C memory allocator
*/
NY_EXPORT void nany_memalloc_set_default(nyallocator_t*);
/*!
** \nbrief Switch to the std C memory allocator with bounds checking
*/
NY_EXPORT void nany_memalloc_set_with_limit(nyallocator_t*, size_t limit);
/*!
** \nbrief Copy allocator
*/
void nany_memalloc_copy(nyallocator_t* out, const nyallocator_t* const src);
/*@}*/







/*! \name Console */
/*@{*/
typedef enum nyconsole_output_t
{
	/*! Print to the cout */
	nycout = 1,
	/*! Print to the cerr */
	nycerr = 2,
}
nyconsole_output_t;


/*! Color constants */
typedef enum nycolor_t
{
	/*! None / reset */
	nyc_none = 0,
	/*! Black */
	nyc_black,
	/*! Red */
	nyc_red,
	/*! Green */
	nyc_green,
	/*! Yellow */
	nyc_yellow,
	/*! Dark Blue */
	nyc_blue,
	/*! Purple */
	nyc_purple,
	/*! Gray */
	nyc_gray,
	/*! White */
	nyc_white,
	/*! Light blue */
	nyc_lightblue
}
nycolor_t;

enum {
	/*! The total number of colors */
	nyc_count = nyc_lightblue + 1
};


typedef struct nyconsole_t
{
	/*! Write some data to STDOUT */
	void (*write_stdout)(void*, const char* text, size_t length);
	/*! Write some data to STDERR */
	void (*write_stderr)(void*, const char* text, size_t length);
	/*! Flush output */
	void (*flush)(void*, nyconsole_output_t);
	/*! Set the text color */
	void (*set_color)(void*, nyconsole_output_t, nycolor_t);

	/*! Internal opaque pointer*/
	void* internal;
	/*! Flush STDERR */
	void (*release)(const struct nyconsole_t*);
}
nyconsole_t;

/*! Initialize a project configuration */
NY_EXPORT void nany_console_cf_set_stdcout(nyconsole_t*);

/*!
** \nbrief Copy Cf
*/
void nany_console_cf_copy(nyconsole_t* out, const nyconsole_t* const src);
/*@}*/








/*! \name Project management */
/*@{*/
/*! Project Configuration */
typedef struct nyproject_cf_t
{
	/*! Memory allocator */
	nyallocator_t allocator;

	/*! A project has been created */
	void (*on_create)(nyproject_t*);
	/*! A project has been destroyed */
	void (*on_destroy)(nyproject_t*);

	/*! A new target has been added */
	void (*on_target_added)(nyproject_t*, nytarget_t*, const char* name, uint32_t len);
	/*! A target has been removed */
	void (*on_target_removed)(nyproject_t*, nytarget_t*, const char* name, uint32_t len);
}
nyproject_cf_t;


/*!
** \brief Create a new nany project
**
** \param cf Configuration (can be null)
** \return A ref-counted pointer to the new project. NULL if the operation failed. The returned
**   object must be released by `nany_project_unref`
*/
NY_EXPORT nyproject_t* nany_project_create(const nyproject_cf_t* cf);

/*!
** \brief Acquire a project
** \param project Project pointer (can be null)
*/
NY_EXPORT void nany_project_ref(nyproject_t* project);

/*!
** \brief Unref a project and destroy it if required
** \param project A Project pointer (can be null)
*/
NY_EXPORT void nany_project_unref(nyproject_t* project);

/*! Initialize a project configuration */
NY_EXPORT void nany_project_cf_init(nyproject_cf_t*);


/*! Add a source file to the default target */
NY_EXPORT nybool_t nany_project_add_source_from_file(nyproject_t*, const char* filename);
/*! Add a source file to the default target (with filename length) */
NY_EXPORT nybool_t nany_project_add_source_from_file_n(nyproject_t*, const char* filename, size_t);

/*! Add a source to the default target */
NY_EXPORT nybool_t nany_project_add_source(nyproject_t*, const char* text);
/*! Add a source to the default target (with filename length) */
NY_EXPORT nybool_t nany_project_add_source_n(nyproject_t*, const char* text, size_t);


/*! Lock a project */
NY_EXPORT void nany_lock(const nyproject_t*);
/*! Unlock a project */
NY_EXPORT void nany_unlock(const nyproject_t*);
/*! Try to lock a project */
NY_EXPORT nybool_t nany_trylock(const nyproject_t*);
/*@}*/




/*! \name Build */
/*@{*/
/*! Project Configuration */
typedef struct nybuild_cf_t
{
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! Console output */
	nyconsole_t console;

	/*! Make all warnings into errors */
	nybool_t warnings_into_errors;

	/*! A project has been created */
	void (*on_create)(nybuild_t*, nyproject_t*);
	/*! A project has been destroyed */
	void (*on_destroy)(nybuild_t*, nyproject_t*);

	/*! Query if a new build can be started */
	nybool_t (*on_query)(const nyproject_t*);
	/*! A new build has been started */
	void (*on_begin)(const nyproject_t*, nybuild_t*);

	/*! Progress report */
	nybool_t (*on_progress)(const nyproject_t*, nybuild_t*, const char* id, const char* element, uint32_t percent);
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
NY_EXPORT nybuild_t* nany_build_prepare(nyproject_t*, const nybuild_cf_t*);

/*!
** \brief Build the project
*/
NY_EXPORT nybool_t nany_build(nybuild_t*);

/*!
** \brief Print the build report to the console
**
** \param build A build object (can be null, will do nothing)
** \param print_header nytrue to add information about the compiler
*/
NY_EXPORT void nany_build_print_report_to_console(nybuild_t* build, nybool_t print_header);



/*!
** \brief Acquire a build
** \param build build pointer (can be null)
*/
NY_EXPORT void nany_build_ref(nybuild_t* build);
/*!
** \brief Unref a build and destroy it if required
** \param build A build pointer (can be null)
*/
NY_EXPORT void nany_build_unref(nybuild_t* build);

/*! Initialize a project configuration */
NY_EXPORT void nany_build_cf_init(nybuild_cf_t* cf, const nyproject_t* project);
/*@}*/







/*! \name Program */
/*@{*/
typedef struct nybacktrace_entry_t
{
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
typedef struct nyprogram_cf_t
{
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! Console output */
	nyconsole_t console;

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
}
nyprogram_cf_t;


/*! Context at runtime for native C calls */
typedef struct nyvm_t
{
	/*! Allocator */
	nyallocator_t* allocator;
	/*! Current program */
	nyprogram_t* program;
	/*! Current thread */
	nytctx_t* tctx;
	/*! Console */
	nyconsole_t* console;
}
nyvm_t;


/*!
** \brief Create a byte code program from a given build
**
** \param build A build
*/
NY_EXPORT nyprogram_t* nany_program_prepare(nybuild_t* build, const nyprogram_cf_t* cf);

/*!
** \brief Execute a Nany program
**
** \param program Bytecode nany program
** \param argc Number of arguments (always >= 1)
** \param argv A null terminated list of arguments. The first argument is the full path
**    to the program/script. Arguments must use the UTF8 encoding
** \return Exit status code
*/
NY_EXPORT int nany_program_main(nyprogram_t* program, uint32_t argc, const char** argv);



/*!
** \brief Acquire a program
** \param program program pointer (can be null)
*/
NY_EXPORT void nany_program_ref(nyprogram_t* program);
/*!
** \brief Unref a program and destroy it if required
** \param program A program pointer (can be null)
*/
NY_EXPORT void nany_program_unref(nyprogram_t* program);

/*! Initialize a project configuration */
NY_EXPORT void nany_program_cf_init(nyprogram_cf_t* cf, const nybuild_cf_t*);
/*@}*/








/*! \name Utilities */
/*@{*/
/*!
** \brief Print the AST of a nany source file
**
** \warning Writing to the same FD by multiple threads is not thread-safe on all platforms
*/
nybool_t nany_print_ast_from_file(const char* filename, int fd, nybool_t unixcolors);

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
NY_EXPORT nybool_t nany_print_ast_from_memory(const char* content, int fd, nybool_t unixcolors);
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
NY_EXPORT nybool_t nany_try_parse_file(const char* const filename);
/*!
** \brief Check if a filename is a valid nany source code
**
** \param filename An arbitrary filename (utf-8 c-string)
** \param length Length of the filename
** \return nytrue if the file has been successfully parsed, false otherwise
*/
NY_EXPORT nybool_t nany_try_parse_file_n(const char* filename, size_t length);




/*! \name Utilities for internal types */
/*@{*/
/*!
** \brief Convert a C-String representing a visibility level
**
** An empty value will represent a "default" visibility (nyv_undefined)
*/
NY_EXPORT nyvisibility_t  nany_cstring_to_visibility(const char* const text);
/*!
** \brief Convert a C-String representing a visibility level (with given length)
**
** An empty value will represent a "default" visibility (nyv_undefined)
*/
NY_EXPORT nyvisibility_t  nany_cstring_to_visibility_n(const char* const text, size_t length);

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
NY_EXPORT nytype_t nany_cstring_to_type(const char* const text);

/*!
** \brief Convert a string into the builtin type (with length provided)
**
** \param text An arbitrary text (ex: "__uint64")
** \return The corresponding type (ex: nyt_uint64)
*/
NY_EXPORT nytype_t nany_cstring_to_type_n(const char* const text, size_t length);

/*!
** \brief Convert a type into a c-string
*/
NY_EXPORT const char* nany_type_to_cstring(nytype_t);

/*!
** \brief Get the size in bytes of a Nany builtin type
*/
NY_EXPORT uint32_t nany_type_sizeof(nytype_t);
/*@}*/






/*! \name Convenient wrappers */
/*@{*/
typedef struct nyrun_cf_t
{
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! Console */
	nyconsole_t console;

	/*! Default prject settings */
	nyproject_cf_t project;
	/*! Default build settings */
	nybuild_cf_t build;
	/*! Default program settings */
	nyprogram_cf_t program;

	/*! A non-zero value for verbose mode */
	int verbose;
}
nyrun_cf_t;

/*! Initialize a template object */
NY_EXPORT void nany_run_cf_init(nyrun_cf_t*);

/*! Release resources held by a template object */
NY_EXPORT void nany_run_cf_release(const nyrun_cf_t*);
/*!
** \brief Compile & run a nany program
**
** \param cf A template for settings (can be null)
** \param source A nany program (can be null)
** \param argc The number of additional input arguments (ignored if <= 0)
** \param argv Input arguments (ignored if null)
** \return Exit status code
*/
NY_EXPORT int nany_run(const nyrun_cf_t* cf, const char* source, uint32_t argc, const char** argv);

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
NY_EXPORT int nany_run_n(const nyrun_cf_t* cf, const char* source, size_t length, uint32_t argc, const char** argv);


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
NY_EXPORT int nany_run_file(const nyrun_cf_t* cf, const char* file, uint32_t argc, const char** argv);

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
NY_EXPORT int nany_run_file_n(const nyrun_cf_t* cf, const char* file, size_t length, uint32_t argc, const char** argv);
/*@}*/




#ifdef __cplusplus
}
#endif

#endif /* __LIBNANY_NANY_C_H__ */
