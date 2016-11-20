#include "nany/nany.h"
#include <yuni/string.h>

using namespace Yuni;





extern "C" nyvisibility_t  nycstring_to_visibility_n(const char* const text, size_t length)
{
	try
	{
		AnyString s{text, (uint32_t)length};
		s.trim();
		if (YUNI_UNLIKELY(s.empty()))
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
	}
	catch (...) {}
	return nyv_undefined;
}


extern "C" const char* nyvisibility_to_cstring(nyvisibility_t visibility)
{
	static constexpr const char* const values[(uint32_t) nyv_count] =
	{
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


extern "C" nyvisibility_t nycstring_to_visibility(const char* const text)
{
	size_t length = (text ? strlen(text) : 0u);
	return nycstring_to_visibility_n(text, length);
}
