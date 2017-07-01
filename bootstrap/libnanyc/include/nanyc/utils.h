/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_UTILS_H__
#define __LIBNANYC_UTILS_H__

#include <nanyc/types.h>
#include <nanyc/console.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** \brief Print the ast of a source file
*/
NY_EXPORT nybool_t nyast_print_file(nyconsole_t*, const char* filename, size_t len);

/*!
** \brief Print the ast of some content in memory
*/
NY_EXPORT nybool_t nyast_print_content(nyconsole_t*, const char* content, size_t len);

/*!
** \brief Try to simply parse a file
*/
NY_EXPORT nybool_t nyparse_check_file(const char*, size_t);

/*!
** \brief Try to simply parse some content in memory
*/
NY_EXPORT nybool_t nyparse_check_content(const char*, size_t);


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_UTILS_H__ */
