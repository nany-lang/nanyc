/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_NANY_EXT_C_H__
#define __LIBNANYC_NANY_EXT_C_H__
#include "nany.h"




#ifdef __cplusplus
extern "C" {
#endif


/*! Extension manager for programs using nany as plugins */
typedef struct nyextman_t nyextman_t;

/*! Extension manager for programs using nany as plugins */
typedef struct nyextmonitor_t nyextmonitor_t;

/*! Object */
typedef struct nyobj_t nyobj_t;

/*! Callback for iterating over extensions */
typedef int (*nyextman_each_t)(nyextman_t*, nyext_t*);


/*! Extension State */
enum nyext_state_t
{
	/*! No state */
	nyest_none       = 0,
	/*! Ready to run and waiting for requests */
	nyest_idle       = (1 << 0),
	/*! Currently running */
	nyest_running    = (1 << 1),
	/*! Stopping the extension */
	nyest_stopping   = (1 << 2),
	/*! Compilation in progress */
	nyest_building   = (1 << 3),
	/*! Loading (reading manifest) */
	nyest_loading    = (1 << 4),
	/*! Error (build failed or runtime failure) */
	nyest_error      = (1 << 5),
	/*! Listed but nothing loaded */
	nyest_disabled   = (1 << 6),
};




typedef struct nyextman_cf_t
{
	/*! Maximum number of threads (0: automatic) */
	uint32_t max_threads;

	/*! Warning reporting */
	void (*on_warning)(nyextman_t*, nyext_t*, const char* msg, uint32_t len);
	/*! Error reporting */
	void (*on_error)(nyextman_t*, nyext_t*, const char* msg, uint32_t len);

	void (*on_started)(nyextman_t*);
	void (*on_stop)(nyextman_t*);
}
nyextman_cf_t;



NY_EXPORT void nany_extman_cf_init(nyextman_cf_t*);




NY_EXPORT nyextman_t* nany_extman_create(const nyextman_cf_t*, void* userdata);

NY_EXPORT void nany_extman_ref(nyextman_t*);

NY_EXPORT void nany_extman_unref(nyextman_t*);


NY_EXPORT uint64_t nany_ext_load_from_folder(nyextman_t*, const char* folder);

NY_EXPORT uint64_t nany_ext_load_from_file(nyextman_t*, const char* file);

NY_EXPORT uint64_t nany_ext_load_from_script(nyextman_t*, const char* content);

NY_EXPORT void nany_ext_reload_all(nyextman_t*);

NY_EXPORT void nany_ext_unload_all(nyextman_t*);

NY_EXPORT void nany_ext_remove_all(nyextman_t*);

NY_EXPORT nytask_t* nany_ext_from_id(nyextman_t*, uint64_t id);

NY_EXPORT nybool_t nany_ext_reload(nyext_t*);

NY_EXPORT nybool_t nany_ext_unload(nyext_t*);


NY_EXPORT uint32_t nany_ext_each(nyextman_t*, uint32_t filter, nyextman_each_t);

NY_EXPORT void nany_ext_get_description(nyext_t*, nyext_description_t*);





typedef struct nyextmonitor_cf_t
{
	/*! An extension entered into a new state */
	void (*on_ext_state)(nyextman_t*, nyext_t*, nyext_state_t);
	/*! An extension has been registered */
	void (*on_ext_register)(nyextman_t*, nyext_t*, nyext_state_t, nybool_t added);
	/*! An extension has been removed */
	void (*on_ext_removed)(nyextman_t*, nyext_t*);
	/*! Callback called when starting the monitoring to prealloc data if helpful */
	void (*on_reserve)(uint32_t count);
	/*! Callback called every 'tick' ms to update statistics */
	void (*on_tick)(nyextman_t*, nyext_t*, size_t bytes, uint32_t threads, uint32_t tasks, uint32_t fds);

	/*! The monitoring is starting */
	void (*on_start)(nyextman_t*);
	/*! The monitor has been stopped */
	void (*on_stop)(nyextman_t*);

	/*! Auto refresh every X ms */
	uint32_t tickms;
}
nyextmonitor_cf_t;





NY_EXPORT nyobj_t* nany_extman_interactive(nyextman_t*, const char** args, const nyobj_t*);

NY_EXPORT nytask_t* nany_extman_interactive_async(nyextman_t*, const char** args, const nyobj_t*);



NY_EXPORT nybool_t nany_ext_verify_manifest(const char* file);


#ifdef __cplusplus
}
#endif

#endif /*__LIBNANYC_NANY_EXT_C_H__*/
