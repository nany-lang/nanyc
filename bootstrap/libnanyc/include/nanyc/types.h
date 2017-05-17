/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_TYPES_H__
#define __LIBNANYC_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! Boolean values */
typedef enum nybool_t {
	nyfalse = 0,
	nytrue
}
nybool_t;


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_TYPES_H__ */
