/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_LIBRARY_H__
#define __LIBNANYC_LIBRARY_H__

#include <nanyc/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
** \brief Get the version of the library
** \return XXYYZZZZ where XX is the major version, YY the minor one and ZZZZ the patch number
*/
NY_EXPORT uint32_t libnanyc_version();

/*!
** \brief Get each component of the version of the library
** \return XXYYZZZZ where XX is the major version, YY the minor one and ZZZZ the patch number
*/
NY_EXPORT uint32_t libnanyc_version_ex(uint32_t* major, uint32_t* minor, uint32_t* patch);

/*!
** \brief Get the full version of the library in a string format
** \return non-null c-string (ex: "1.2.3+beta-55fc0eb9")
*/
NY_EXPORT const char* libnanyc_version_to_cstr();

/*!
** \brief Get the full version of the library in a string format (with length)
** \param[out] length Length in bytes [optional]
** \return non-null c-string (ex: "1.2.3+beta-55fc0eb9")
*/
NY_EXPORT const char* libnanyc_version_to_cstr_ex(uint32_t* length);

/*!
** \brief Get the release type of the library
** \return non-null c-string ("beta", "alpha" or NULL for normal releases)
*/
NY_EXPORT const char* libnanyc_version_release();

/*!
** \brief Get the commit id of the library
** \return non-null c-string (ex: "55fc0eb9")
*/
NY_EXPORT const char* libnanyc_version_commit_sha();

/*!
** \brief Get the url where the documentation of the library can be found
** \return non-null c-string (ex: "https://nany.io")
*/
NY_EXPORT const char* libnanyc_website_url();


#ifdef __cplusplus
}
#endif

#endif /* __LIBNANYC_LIBRARY_H__ */
