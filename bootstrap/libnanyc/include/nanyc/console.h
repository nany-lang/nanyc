/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_CONSOLE_H__
#define __LIBNANYC_CONSOLE_H__

#include <nanyc/types.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nycolor_t {
	nyc_default,
	nyc_black,
	nyc_red,
	nyc_green,
	nyc_yellow,
	nyc_blue,
	nyc_magenta,
	nyc_cyan,
	nyc_white,
}
nycolor_t;

typedef struct nyconsole_t nyconsole_t;

typedef struct nyconsole_opts_t {
	void* userdata;
	void (*write)(struct nyconsole_t*, const char*, size_t);
	void (*flush)(struct nyconsole_t*);
	void (*set_color)(struct nyconsole_t*, nycolor_t);
	void (*set_bkcolor)(struct nyconsole_t*, nycolor_t);
	void (*on_dispose)(nyconsole_t*);
}
nyconsole_opts_t;

struct nyconsole_t {
	void* userdata;
	void (*write)(nyconsole_t*, const char*, size_t);
	void (*flush)(nyconsole_t*);
	void (*set_color)(nyconsole_t*, nycolor_t);
	void (*set_bkcolor)(nyconsole_t*, nycolor_t);
	void (*on_dispose)(nyconsole_t*);
};

/*!
** \brief Make a new console for writing to stdout
*/
NY_EXPORT nyconsole_t* nyconsole_make_from_stdout();

/*!
** \brief Make a new console for writing to stderr
*/
NY_EXPORT nyconsole_t* nyconsole_make_from_stderr();

/*!
** \brief Make a new console
*/
NY_EXPORT nyconsole_t* nyconsole_make(const nyconsole_opts_t*);

/*!
** \brief Initialize a preallocated console object
*/
NY_EXPORT void nyconsole_init(nyconsole_t*, const nyconsole_opts_t*);

/*!
** \brief Initialize a preallocated console object for writing to stdout
*/
NY_EXPORT void nyconsole_init_from_stdout(nyconsole_t*);

/*!
** \brief Initialize a preallocated console object for writing to stderr
*/
NY_EXPORT void nyconsole_init_from_stderr(nyconsole_t*);

/*!
** \brief Release all resources held by a console
*/
NY_EXPORT void nyconsole_dispose(nyconsole_t*);

/*!
** \brief Write some blob to a console
*/
inline void nywrite(nyconsole_t*, const char* utf8s, size_t length);

/*!
** \brief Flush the content of a console
*/
inline void nyconsole_flush(nyconsole_t*);

/*!
** \brief Set the current foreground color of the console
*/
inline void nyconsole_set_color(nyconsole_t*, nycolor_t);

/*!
** \brief Set the current background color of the console
*/
inline void nyconsole_set_bkcolor(nyconsole_t*, nycolor_t);




/* *** */




inline void nywrite(nyconsole_t* console, const char* utf8s, size_t length) {
	assert(console != NULL && "invalid nyconsole_t pointer");
	if (length != 0) {
		assert(utf8s != NULL && "invalid null string for writing to the console");
		console->write(console, utf8s, length);
	}
}

inline void nyconsole_flush(nyconsole_t* console) {
	assert(console != NULL && "invalid nyconsole_t pointer");
	console->flush(console);
}

inline void nyconsole_set_color(nyconsole_t* console, nycolor_t color) {
	assert(console != NULL && "invalid nyconsole_t pointer");
	console->set_color(console, color);
}

inline void nyconsole_set_bkcolor(nyconsole_t* console, nycolor_t color) {
	assert(console != NULL && "invalid nyconsole_t pointer");
	console->set_bkcolor(console, color);
}

#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_CONSOLE_H__ */
