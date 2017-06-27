#include "details/atom/ctype.h"
#include <unordered_map>


namespace ny {

static const std::unordered_map<AnyString, CType> translationStringToDef {
	{ "__bool",    CType::t_bool },
	{ "__pointer", CType::t_ptr  },
	{ "__u8",      CType::t_u8   },
	{ "__u16",     CType::t_u16  },
	{ "__u32",     CType::t_u32  },
	{ "__u64",     CType::t_u64  },
	{ "__i8",      CType::t_i8   },
	{ "__i16",     CType::t_i16  },
	{ "__i32",     CType::t_i32  },
	{ "__i64",     CType::t_i64  },
	{ "__f32",     CType::t_f32  },
	{ "__f64",     CType::t_f64  },
};

static constexpr const char* const type2cstring[ctypeCount] = {
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

static constexpr const uint32_t type2size[] = {
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

CType toCType(const AnyString& text) {
	auto it = translationStringToDef.find(text);
	return (it != translationStringToDef.cend()) ? it->second : CType::t_void;
}

AnyString toString(CType type) {
	return type2cstring[static_cast<uint32_t>(type)];
}

uint32_t ctypeSizeof(CType type) {
	return type2size[static_cast<uint32_t>(type)];
}

} // namespace ny
