#include "nany/nany.h"
#include "libnanyc-version.h"
#include <assert.h>

// NOTE
//
// This file is intentionnaly a C file to abort the compilation if nany.h
// does not use ansi C


static inline const char* strOrNull(const char* const text) {
	return (text && *text != '\0') ? text : NULL;
}


const char* nylib_website_url() {
	#ifdef YUNI_OS_LINUX
	_Static_assert(LIBNANYC_WEBSITE != NULL, "invalid null url");
	#endif
	return LIBNANYC_WEBSITE;
}


const char* nylib_version() {
	#ifdef YUNI_OS_LINUX
	_Static_assert(LIBNANYC_VERSION_STR != NULL, "invalid null version");
	#endif
	return LIBNANYC_VERSION_STR;
}


const char* nylib_version_metadata() {
	return strOrNull(LIBNANYC_VERSION_METADATA);
}


const char* nylib_version_prerelease() {
	return strOrNull(LIBNANYC_VERSION_PRERELEASE);
}


uint32_t nylib_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch) {
	if (major)
		*major = LIBNANYC_VERSION_MAJOR;
	if (minor)
		*minor = LIBNANYC_VERSION_MINOR;
	if (patch)
		*patch = LIBNANYC_VERSION_PATCH;
	return LIBNANYC_VERSION_MAJOR * 100000u + LIBNANYC_VERSION_MINOR * 1000u + LIBNANYC_VERSION_PATCH;
}


int nylib_check_compatible_version(uint32_t major, uint32_t minor) {
	/* currently, no real incompatibilities except for version comparison */
	return ((LIBNANYC_VERSION_MAJOR >= major)
		|| (LIBNANYC_VERSION_MAJOR == major && minor <= LIBNANYC_VERSION_MINOR))
		? 0 : 1;
}
