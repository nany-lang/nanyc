/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_REPORT_H__
#define __LIBNANYC_REPORT_H__

#include <nanyc/types.h>
#include <nanyc/console.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! Report */
typedef struct nyreport_t nyreport_t;

typedef enum nyerrlevel_t {
	nyel_ice,
	nyel_error,
	nyel_warning,
	nyel_success,
	nyel_info,
	nyel_hint,
	nyel_suggest,
	nyel_trace,
	nyel_none,
}
nyerrlevel_t;

typedef struct nyreport_entry_origin_t {
	nyanystr_t filename;
	uint32_t line;
	uint32_t column;
}
nyreport_entry_origin_t;

typedef struct nyreport_entry_t {
	nyerrlevel_t errlevel;
	nyanystr_t highlight; /**/
	nyanystr_t text;
	nyreport_entry_origin_t origin;
	uint32_t item_count;
	uint32_t depth;
	const struct nyreport_entry_t* parent;
}
nyreport_entry_t;

typedef struct nyreport_visitor_opts_t {
	void* userdata;
	nyflow_t (*on_entry)(void* userdata, const nyreport_entry_t* entry);
	nyflow_t (*on_entry_leave)(void* userdata, const nyreport_entry_t* entry);
	void* (*on_start)(void* userdata, const nyreport_t*);
	nybool_t (*on_end)(void* userdata, const nyreport_t*, nybool_t result);
}
nyreport_visitor_opts_t;


/*!
** \brief Visit recursively each report entry
*/
NY_EXPORT nybool_t nyreport_visit(const nyreport_visitor_opts_t*, const nyreport_t*);

/*!
** \brief Print the report using a give nconsole
*/
NY_EXPORT nybool_t nyreport_print(const nyreport_t*, nyconsole_t*);

/*!
** \brief Print the report on stdout
*/
NY_EXPORT nybool_t nyreport_print_stdout(const nyreport_t*);


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_REPORT_H__ */
