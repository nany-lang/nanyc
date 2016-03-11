#ifndef __LIBNANY_NANY_C_HXX__
# define __LIBNANY_NANY_C_HXX__

#include "nany.h"
#include <assert.h>



#ifdef __cplusplus
extern "C"
{
#endif


inline size_t nany_type_sizeof(nytype_t type)
{
	switch (type)
	{
		case nyt_pointer: return sizeof(size_t);
		case nyt_bool:    return sizeof(uint8_t);
		case nyt_u8:      return sizeof(uint8_t);
		case nyt_u16:     return sizeof(uint16_t);
		case nyt_u32:     return sizeof(uint32_t);
		case nyt_u64:     return sizeof(uint64_t);
		case nyt_i8:      return sizeof(int8_t);
		case nyt_i16:     return sizeof(int16_t);
		case nyt_i32:     return sizeof(int32_t);
		case nyt_i64:     return sizeof(int64_t);
		case nyt_f32:     return sizeof(float);
		case nyt_f64:     return sizeof(double);
		case nyt_void:
		case nyt_any:
		case nyt_count: break;
	}
	return 0;
}


inline nybool_t nany_source_add_from_file(nycontext_t* ctx, const char* const filename)
{
	size_t length = (filename ? strlen(filename) : 0u);
	return nany_source_add_from_file_n(ctx, filename, length);
}


inline nybool_t nany_source_add(nycontext_t* ctx, const char* const text)
{
	size_t length = (text ? strlen(text) : 0u);
	return nany_source_add_n(ctx, text, length);
}


inline nybool_t  nany_try_parse_file(const char* const filename)
{
	size_t length = (filename ? strlen(filename) : 0u);
	return nany_try_parse_file_n(filename, length);
}


inline nyvisibility_t  nany_cstring_to_visibility(const char* text)
{
	size_t length = (text ? (uint32_t) strlen(text) : 0u);
	return nany_cstring_to_visibility_n(text, length);
}


inline nytype_t nany_cstring_to_type(const char* text)
{
	size_t length = (text ? (uint32_t) strlen(text) : 0u);
	return nany_cstring_to_type_n(text, length);
}


inline nybool_t nany_print_ast_from_file(const char* filename, int fd, nybool_t unixcolors)
{
	size_t length = (filename ? strlen(filename) : 0u);
	return nany_print_ast_from_file_n(filename, length, fd, unixcolors);
}


inline nybool_t nany_print_ast_from_memory(const char* content, int fd, nybool_t unixcolors)
{
	size_t length = (content ? strlen(content) : 0u);
	return nany_print_ast_from_memory_n(content, length, fd, unixcolors);
}




#ifdef __cplusplus
}
#endif

#endif /* __LIBNANY_NANY_C_HXX__ */
