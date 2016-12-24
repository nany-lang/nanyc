/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_TYPES_H__
#define __LIBNANYC_TYPES_H__
#include <stdint.h>


#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef __GNUC__
#    define LIBNANYC_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#    define LIBNANYC_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#  else
#    define LIBNANYC_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#    define LIBNANYC_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#  endif
#else
#  define LIBNANYC_VISIBILITY_EXPORT     __attribute__((visibility("default")))
#  define LIBNANYC_VISIBILITY_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBNANYC_DLL_EXPORT)
#  define LIBNANYC_DLL_EXPORT
#endif

/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBNANYC_DLL_EXPORT)
#   define NY_EXPORT LIBNANYC_VISIBILITY_EXPORT
#else
#   define NY_EXPORT LIBNANYC_VISIBILITY_IMPORT
#endif


#endif  /*__LIBNANYC_TYPES_H__ */
