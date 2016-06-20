#include <yuni/yuni.h>
#include <yuni/core/string/string.h>
#include <nany/nany.h>
#include <unordered_map>

using namespace Yuni;





static const std::unordered_map<AnyString, nytype_t> translationStringToDef
{
	{ "__bool",    nyt_bool },
	{ "__pointer", nyt_ptr  },
	{ "__u8",      nyt_u8   },
	{ "__u16",     nyt_u16  },
	{ "__u32",     nyt_u32  },
	{ "__u64",     nyt_u64  },
	{ "__i8",      nyt_i8   },
	{ "__i16",     nyt_i16  },
	{ "__i32",     nyt_i32  },
	{ "__i64",     nyt_i64  },
	{ "__f32",     nyt_f32  },
	{ "__f64",     nyt_f64  },
};

extern "C" nytype_t nany_cstring_to_type_n(const char* const text, size_t length)
{
	if (length > 3 and length < 16 and text)
	{
		auto it = translationStringToDef.find(AnyString{text, static_cast<uint32_t>(length)});
		return (it != translationStringToDef.cend()) ? it->second : nyt_void;
	}
	return nyt_void;
}

extern "C" nytype_t nany_cstring_to_type(const char* const text)
{
	size_t length = (text ? strlen(text) : 0u);
	if (length > 3 and length < 16)
	{
		auto it = translationStringToDef.find(AnyString{text, static_cast<uint32_t>(length)});
		return (it != translationStringToDef.cend()) ? it->second : nyt_void;
	}
	return nyt_void;
}




static constexpr const char* const type2cstring[nyt_count] =
{
	"void",
	"any",
	"__pointer",
	"__bool",
	"__u8",
	"__u16",
	"__u32",
	"__u64",
	"__i8",
	"__i16",
	"__i32",
	"__i64",
	"__f32",
	"__f64"
};
extern "C" const char* nany_type_to_cstring(nytype_t type)
{
	return (static_cast<uint32_t>(type) < nyt_count)
		? type2cstring[static_cast<uint32_t>(type)]
		: "<unknown>";
}


static constexpr const uint32_t type2size[] =
{
	0,
	0,
	(uint32_t) sizeof(void*),
	(uint32_t) sizeof(uint8_t),

	(uint32_t) sizeof(uint8_t),
	(uint32_t) sizeof(uint16_t),
	(uint32_t) sizeof(uint32_t),
	(uint32_t) sizeof(uint64_t),
	(uint32_t) sizeof(int8_t),
	(uint32_t) sizeof(int16_t),
	(uint32_t) sizeof(int32_t),
	(uint32_t) sizeof(int64_t),

	(uint32_t) sizeof(float),
	(uint32_t) sizeof(double),
};
extern "C" uint32_t nany_type_sizeof(nytype_t type)
{
	return (static_cast<uint32_t>(type) < nyt_count)
		? type2size[static_cast<uint32_t>(type)]
		: 0;
}
