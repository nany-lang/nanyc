#include "nany/nany.h"
#include <yuni/core/system/console/console.h>
#include <yuni/string.h>

using namespace Yuni;







extern "C" nyvisibility_t  nany_cstring_to_visibility_n(const char* text, size_t length)
{
	AnyString s{text, (uint32_t)length};
	s.trim();

	if (s.empty())
		return nyv_default;

	switch (s[0])
	{
		case 'p':
		{
			if (s.equalsInsensitive("published"))
				return nyv_published;
			if (s.equalsInsensitive("public"))
				return nyv_public;
			if (s.equalsInsensitive("protected"))
				return nyv_protected;
			if (s.equalsInsensitive("private"))
				return nyv_private;
			break;
		}
		case 'i':
		{
			if (s.equalsInsensitive("internal"))
				return nyv_internal;
			break;
		}
	}
	return nyv_undefined;
}


extern "C" const char* nany_visibility_to_cstring(nyvisibility_t visibility)
{
	static constexpr const char* const values[(uint) nyv_max] =
	{
		"undefined",
		"default",
		"private",
		"protected",
		"internal",
		"public",
		"published"
	};
	uint index = (uint) visibility;
	return (index < (uint) nyv_max) ? values[index] : values[index];
}
