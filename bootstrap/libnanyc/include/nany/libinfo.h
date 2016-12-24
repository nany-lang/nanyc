/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_LIBINFO_H__
#define __LIBNANYC_LIBINFO_H__
#include <nany/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*! \name Library Information */
/*@{*/
/*!
** \brief Print information about nany for bug reporting
*/
NY_EXPORT void nylib_print_info_for_bugreport();

/*!
** \brief Export information about nany for bug reporting
**
** \param[out] length Length of the returned c-string (can be null)
** \return A C-String, which must be released by `free (3)`. NULL if an error occured
*/
NY_EXPORT char* nylib_get_info_for_bugreport(uint32_t* length);

/*! nany's website */
NY_EXPORT const char* nylib_website_url();

/*!
** \brief Get the version of libnany
**
** \param[out] major Major version (eX: 2.4.1 -> 2) (can be null)
** \param[out] minor Minor version (eX: 2.4.1 -> 4) (can be null)
** \param[out] patch Patch (eX: 2.4.1 -> 1) (can be null)
** \return The full version within a single integer (ex: 2.4.1 -> 204001)
*/
NY_EXPORT uint32_t nylib_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch);

/*! Get the full version of nany (string, ex: 2.4.1-beta+2e738ae) */
NY_EXPORT const char* nylib_version();
/*! Get the version metadata (ex: '2e738ae', null if empty) */
NY_EXPORT const char* nylib_version_metadata();
/*! Get the pre-release version (ex: 'beta', null if empty) */
NY_EXPORT const char* nylib_version_prerelease();

/*!
** \brief Check if the version is compatible with the library
**
** This function can be used to avoid unwanted behaviors when
** a program is able to use several versions of libnany
** \code
** if (!nylib_check_compatible_version(0, 2))
**     fprintf(stderr, "incompatible version\n");
** \endcode
** \return 0 when succeeded, != 0 if the version is incompatible
*/
NY_EXPORT int nylib_check_compatible_version(uint32_t major, uint32_t minor);
/*@}*/


#ifdef __cplusplus
}
#endif

#endif  /*__LIBNANYC_LIBINFO_H__ */
