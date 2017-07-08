/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_VM_H__
#define __LIBNANYC_VM_H__

#include <nanyc/types.h>
#include <nanyc/console.h>
#include <nanyc/allocator.h>
#include <nanyc/program.h>
#include <nanyc/io.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct nyvmthread_t {
	void* internal;
	nyio_adapter_t* (*io_resolve)(nyvmthread_t*, nyanystr_t* relpath, const nyanystr_t* path);
	const char* (*io_get_cwd)(nyvmthread_t*, uint32_t* len);
	nyio_err_t (*io_set_cwd)(nyvmthread_t*, const char*, uint32_t);
	nyio_err_t (*io_add_mountpoint)(nyvmthread_t*, const char*, uint32_t, nyio_adapter_t*);
	//! Temporary structure for complex return values by intrinsics
	struct {
		uint64_t size;
		uint64_t capacity;
		void* data;
	}
	returnValue;
	void* userdata;
	nyallocator_t allocator;
	nyconsole_t cout;
	nyconsole_t cerr;
	const nyprogram_t* program;
}
nyvmthread_t;

typedef struct nyvm_opts_t {
	void* userdata;
	nyallocator_t allocator;
	nyconsole_t cout;
	nyconsole_t cerr;
}
nyvm_opts_t;

//! Init VM options with default values
NY_EXPORT void nyvm_opts_init_defaults(nyvm_opts_t*);

/*!
** \brief Run the program
** \return nyfalse if any internal error occured during runtime
*/
NY_EXPORT nybool_t nyvm_run_entrypoint(const nyvm_opts_t*, const nyprogram_t*);


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_VM_H__ */
