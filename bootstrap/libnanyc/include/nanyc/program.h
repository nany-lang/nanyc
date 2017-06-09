/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_PROGRAM_H__
#define __LIBNANYC_PROGRAM_H__

#include <nanyc/types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct nysource_opts_t {
	nyanystr_t filename;
	nyanystr_t content; // null when read from filesystem
}
nysource_opts_t;

typedef struct nysourcelist_opts_t {
	nysource_opts_t* items;
	uint32_t count;
}
nysourcelist_opts_t;




/* *** */




typedef struct nycompile_opts_t {
	void* userdata;
	nysourcelist_opts_t sources;
	nybool_t verbose;
	nybool_t with_nsl_unittests;
	void* (*on_build_start)(void* userdata);
	void (*on_build_stop)(void* userdata, nybool_t success);
	void (*on_file_eaccess)(void*, const nysource_opts_t*);
	void (*on_unittest)(void* userdata, const char* mod, uint32_t mlen, const char* name, uint32_t nlen);
}
nycompile_opts_t;

/*! Opaque struct for representing a compiled nanyc program */
typedef struct nyprogram_t nyprogram_t;

/*!
** \brief Compile a program from a single source file
**
** \note Ignore sources given in the options
** \param opts Compilation options [optional]
** \param filename utf8 filename
** \param len Length in bytes of the filename
*/
NY_EXPORT nyprogram_t* nyprogram_compile_from_file(nycompile_opts_t* opts, const char* filename, size_t len);

/*!
** \brief Compile a program from a content string
**
** \note Ignore sources given in the options
** \param opts Compilation options [optional]
** \param content utf8 code source
** \param len Length in bytes
*/
NY_EXPORT nyprogram_t* nyprogram_compile_from_content(nycompile_opts_t* opts, const char* content, size_t len);

/*!
** \brief Compile a program
**
** \param opts Compilation options [required]
*/
NY_EXPORT nyprogram_t* nyprogram_compile(nycompile_opts_t* opts);

/*!
** \brief Release all resources held by a program
*/
NY_EXPORT void nyprogram_free(nyprogram_t*);


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_PROGRAM_H__ */
