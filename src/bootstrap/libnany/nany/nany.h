/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANY_NANY_C_H__
#define __LIBNANY_NANY_C_H__
#include "../nany/types.h"
#include "../nany/memalloc.h"
#include "../nany/console.h"




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
	void (*on_target_added)(nyproject_t*, nytarget_t*);
	/*! A target has been removed */
	void (*on_target_removed)(nyproject_t*, nytarget_t*);
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

	void (*on_error_file_eacces)(const nyproject_t*, nybuild_t*, const char*, uint32_t);
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
** \param build A build object (can be null)
*/
NY_EXPORT void nany_build_print_report_to_console(nybuild_t* build);



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
/*! Program Configuration */
typedef struct nyprogram_cf_t
{
	/*! Memory allocator */
	nyallocator_t allocator;
	/*! Console output */
	nyconsole_t console;

	/*! A new program has been started */
	nybool_t (*on_begin)(nyprogram_t*);
	/*! A new thread is created */
	nybool_t (*on_thread_create)(nyprogram_t*, nytctx_t*, nythread_t* parent, const char* name, uint32_t size);
	/*! A thread has been destroyed */
	void (*on_thread_destroy)(nyprogram_t*, nythread_t*);

	/*! Error */
	void (*on_error)(const nyprogram_t*, const char** backtrace, uint32_t size);
	/*! A program has stopped */
	void (*on_end)(const nyprogram_t*, int exitcode);
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
NY_EXPORT int nany_main(nyprogram_t* program, int argc, const char** argv);



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
inline nybool_t nany_try_parse_file(const char* const filename);
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
inline nyvisibility_t  nany_cstring_to_visibility(const char* const text);
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
inline nytype_t  nany_cstring_to_type(const char* const text);

/*!
** \brief Convert a string into the builtin type (with length provided)
**
** \param text An arbitrary text (ex: "__uint64")
** \return The corresponding type (ex: nyt_uint64)
*/
NY_EXPORT nytype_t  nany_cstring_to_type_n(const char* const text, size_t length);

/*!
** \brief Convert a type into a c-string
*/
NY_EXPORT const char* nany_type_to_cstring(nytype_t);

/*!
** \brief Get the size in bytes of a Nany type
*/
NY_EXPORT size_t  nany_type_sizeof(nytype_t);
/*@}*/








#ifdef __cplusplus
}
#endif

#include "nany.hxx" // inline functions

#endif /* __LIBNANY_NANY_C_H__ */
