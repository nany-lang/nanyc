#include "nany/nany.h"
#include "libnany-version.h"
#include <assert.h>

// NOTE
//
// This file is intentionnaly a C file to abort the compilation if nany.h
// does not use ansi C



static inline const char* strOrNull(const char* const text)
{
	return (text && *text != '\0') ? text : NULL;
}


const char* libnany_website_url()
{
	#ifdef YUNI_OS_LINUX
	_Static_assert(LIBNANY_WEBSITE != NULL, "invalid null url");
	#endif
	return LIBNANY_WEBSITE;
}

const char* libnany_version()
{
	#ifdef YUNI_OS_LINUX
	_Static_assert(LIBNANY_VERSION_STR != NULL, "invalid null version");
	#endif
	return LIBNANY_VERSION_STR;
}

const char* libnany_version_metadata()
{
	return strOrNull(LIBNANY_VERSION_METADATA);
}

const char* libnany_version_prerelease()
{
	return strOrNull(LIBNANY_VERSION_PRERELEASE);
}


uint32_t libnany_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch)
{
	if (major)
		*major = LIBNANY_VERSION_MAJOR;
	if (minor)
		*minor = LIBNANY_VERSION_MINOR;
	if (patch)
		*patch = LIBNANY_VERSION_PATCH;

	return LIBNANY_VERSION_MAJOR * 100000u + LIBNANY_VERSION_MINOR * 1000u + LIBNANY_VERSION_PATCH;
}


int libnany_check_compatible_version(uint32_t major, uint32_t minor)
{
	/* currently, no real incompatibilities except for version comparison */
	return ((LIBNANY_VERSION_MAJOR >= major)
		|| (LIBNANY_VERSION_MAJOR == major && minor <= LIBNANY_VERSION_MINOR)
		) ? 0 : 1;
}
