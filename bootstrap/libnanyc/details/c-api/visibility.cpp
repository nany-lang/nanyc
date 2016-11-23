#include "nany/nany.h"
#include <yuni/string.h>

using namespace Yuni;


extern "C" nyvisibility_t  nycstring_to_visibility_n(const char* const text, size_t length) {
	// published  9
	// public     6
	// protected  9
	// private    7
	// internal   8
	AnyString str{text, (length < 10) ? static_cast<uint32_t>(length) : 0};
	str.trimRight();
	switch (str.size()) {
		case 6: {
			if (str == "public")
				return nyv_public;
			break;
		}
		case 7: {
			if (str == "private")
				return nyv_private;
			break;
		}
		case 8: {
			if (str == "internal")
				return nyv_internal;
			break;
		}
		case 9: {
			if (str == "published")
				return nyv_published;
			if (str == "protected")
				return nyv_protected;
		}
	}
	return nyv_undefined;
}


extern "C" const char* nyvisibility_to_cstring(nyvisibility_t visibility) {
	static constexpr const char* const values[(uint32_t) nyv_count] = {
		"undefined",
		"default",
		"private",
		"protected",
		"internal",
		"public",
		"published"
	};
	uint32_t index = (uint32_t) visibility;
	return (index < (uint32_t) nyv_count) ? values[index] : values[index];
}


extern "C" nyvisibility_t nycstring_to_visibility(const char* const text) {
	size_t length = (text ? strlen(text) : 0u);
	return nycstring_to_visibility_n(text, length);
}
