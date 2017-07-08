/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_NANYC_H__
#define __LIBNANYC_NANYC_H__

#include <nanyc/vm.h>

#ifdef __cplusplus
extern "C" {
#endif

NY_EXPORT int nymain(const nyvm_opts_t*, const char* filename, size_t flen, uint32_t argc, const char** argv);

NY_EXPORT int nymain_ex(const nyvm_opts_t*, const nyanystr_t* files, uint32_t, uint32_t argc, const char** argv);

#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_NANYC_H__ */
