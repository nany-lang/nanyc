#include <nanyc/library.h>
#include "libnanyc-version.h"
#include <string.h>


#define LIBNANYC_VERSION_UINT32_T \
	(LIBNANYC_VERSION_MAJOR * 1000000 + LIBNANYC_VERSION_MINOR * 10000 + LIBNANYC_VERSION_PATCH)


uint32_t libnanyc_version() {
	return LIBNANYC_VERSION_UINT32_T;
}

uint32_t libnanyc_version_ex(uint32_t* major, uint32_t* minor, uint32_t* patch) {
	if (major)
		*major = LIBNANYC_VERSION_MAJOR;
	if (minor)
		*minor = LIBNANYC_VERSION_MINOR;
	if (patch)
		*patch = LIBNANYC_VERSION_PATCH;
	return LIBNANYC_VERSION_UINT32_T;
}

const char* libnanyc_version_to_cstr() {
	return LIBNANYC_VERSION_STR;
}

const char* libnanyc_version_to_cstr_ex(uint32_t* length) {
	if (length)
		*length = (uint32_t) strlen(LIBNANYC_VERSION_STR);
	return LIBNANYC_VERSION_STR;
}

const char* libnanyc_version_release() {
	size_t length = strlen(LIBNANYC_VERSION_PRERELEASE);
	return (length != 0) ? LIBNANYC_VERSION_PRERELEASE : NULL;
}

const char* libnanyc_version_commit_sha() {
	return LIBNANYC_VERSION_METADATA;
}

const char* libnanyc_website_url() {
	return LIBNANYC_WEBSITE;
}
