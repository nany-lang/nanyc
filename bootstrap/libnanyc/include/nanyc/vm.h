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

#ifdef __cplusplus
extern "C" {
#endif


typedef struct nyvmthread_t {
	void* internal;
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

/*!
** \brief Run the program
** \return nyfalse if any internal error occured during runtime
*/
NY_EXPORT nybool_t nyvm_run_entrypoint(const nyvm_opts_t*, const nyprogram_t*);


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_VM_H__ */
