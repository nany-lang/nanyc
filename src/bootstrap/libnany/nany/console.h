/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANY_NANY_CONSOLE_C_H__
#define __LIBNANY_NANY_CONSOLE_C_H__
#include "../nany/types.h"




#ifdef __cplusplus
extern "C" {
#endif


typedef enum nyconsole_output_t
{
	/*! Print to the cout */
	nycout = 1,
	/*! Print to the cerr */
	nycerr = 2,
}
nyconsole_output_t;



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




/*! \name Console Management */
/*@{*/
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
	void (*release)(void**);
}
nyconsole_t;

/*! Initialize a project configuration */
NY_EXPORT void nany_console_cf_set_stdcout(nyconsole_t*);

/*!
** \nbrief Copy Cf
*/
NY_EXPORT void nany_console_cf_copy(nyconsole_t* out, const nyconsole_t* const src);
/*@}*/









#ifdef __cplusplus
}
#endif

#endif /* __LIBNANY_NANY_CONSOLE_C_H__ */
