#include <nany/nany.h>
#include <yuni/core/bit/bit.h>
#include "details/type/builtin.h"
#include "details/fwd.h"
#include <unordered_map>
#include <map>

using namespace Yuni;





static const std::unordered_map<AnyString, nytype_t> translationStringToDef
{
	{ "__bool",    nyt_bool    },
	{ "__pointer", nyt_pointer },
	{ "__u8",      nyt_u8      },
	{ "__u16",     nyt_u16     },
	{ "__u32",     nyt_u32     },
	{ "__u64",     nyt_u64     },
	{ "__i8",      nyt_i8      },
	{ "__i16",     nyt_i16     },
	{ "__i32",     nyt_i32     },
	{ "__i64",     nyt_i64     },
	{ "__f32",     nyt_f32     },
	{ "__f64",     nyt_f64     },
};


extern "C" nytype_t  nany_cstring_to_type_n(const char* const text, size_t length)
{
	if (likely(length > 3 and length < 128 and text and text[0] == '_' and text[1] == '_'))
	{
		auto it = translationStringToDef.find(AnyString{text, (uint32_t)length});
		return (it != translationStringToDef.cend()) ? it->second : nyt_void;
	}
	return nyt_void;
}


extern "C" const char* nany_type_to_cstring(nytype_t type)
{
	switch (type)
	{
		case nyt_bool:    return "__bool";
		case nyt_pointer: return "__pointer";
		case nyt_u64:     return "__u64";
		case nyt_u32:     return "__u32";
		case nyt_u16:     return "__u16";
		case nyt_u8:      return "__u8";
		case nyt_i64:     return "__i64";
		case nyt_i32:     return "__i32";
		case nyt_i16:     return "__i16";
		case nyt_i8:      return "__i8";
		case nyt_f32:     return "__f32";
		case nyt_f64:     return "__f64";
		case nyt_void:    return "void";
		case nyt_any:     return "any";
		case nyt_count:   return "??";
	}
	assert(false and "unknown nany type");
	return "<unknown>";
}
