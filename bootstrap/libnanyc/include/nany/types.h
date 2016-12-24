/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANYC_TYPES_H__
#define __LIBNANYC_TYPES_H__
#include <stdint.h>


#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef __GNUC__
#    define LIBNANYC_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#    define LIBNANYC_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#  else
#    define LIBNANYC_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#    define LIBNANYC_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#  endif
#else
#  define LIBNANYC_VISIBILITY_EXPORT     __attribute__((visibility("default")))
#  define LIBNANYC_VISIBILITY_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBNANYC_DLL_EXPORT)
#  define LIBNANYC_DLL_EXPORT
#endif

/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBNANYC_DLL_EXPORT)
#   define NY_EXPORT LIBNANYC_VISIBILITY_EXPORT
#else
#   define NY_EXPORT LIBNANYC_VISIBILITY_IMPORT
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*! \name Types */
/*@{*/
#ifndef LIBNANYC_NYBOOL_T
#define LIBNANYC_NYBOOL_T
/*! Boolean type */
typedef enum nybool_t {nyfalse = 0, nytrue} nybool_t;
#endif


#ifndef LIBNANYC_NYANYSTR_T
#define LIBNANYC_NYANYSTR_T
typedef struct {
	uint32_t size;
	const char* c_str;
}
nyanystr_t;
#endif


/*! Nany Language builtin types */
typedef enum { /* nytype_t */
	/*! No type */
	nyt_void = 0,
	/*! Custom user type */
	nyt_any,
	/*! Raw pointer (arch dependent) */
	nyt_ptr,
	/*! Boolean (nytrue/nyfalse) */
	nyt_bool,
	/*! Unsigned 8  bits integer */
	nyt_u8,
	/*! Unsigned 16 bits integer */
	nyt_u16,
	/*! Unsigned 32 bits integer */
	nyt_u32,
	/*! Unsigned 64 bits integer */
	nyt_u64,
	/*! Signed 8  bits integer */
	nyt_i8,
	/*! Signed 16 bits integer */
	nyt_i16,
	/*! Signed 32 bits integer */
	nyt_i32,
	/*! Signed 64 bits integer */
	nyt_i64,
	/*! Floating-point number 32 bits */
	nyt_f32,
	/*! Floating-point number 64 bits */
	nyt_f64
}
nytype_t;

enum {
	/*! The total number of intrinsic types */
	nyt_count = nyt_f64 + 1,
};


/*!
** \brief Identifiers' visibility
**
** \internal All values are strictly ordered
*/
typedef enum { /* nyvisibility_t */
	/*! no valid visibility */
	nyv_undefined,
	/*! default: public or internal, according the context */
	nyv_default,
	/*! private: accessible only by the class */
	nyv_private,
	/*! protected: accessible only by the class and all derived classes */
	nyv_protected,
	/*! internal: accessible only from the correspondig target */
	nyv_internal,
	/*! public: accessible by everyone */
	nyv_public,
	/*! published: same as public, but accessible from an IDE */
	nyv_published
}
nyvisibility_t;

enum {
	/*! The total number of visibility types */
	nyv_count = nyv_published + 1,
};


/*! Opaque Thread Object */
typedef struct nythread_t nythread_t;
/*! Opaque Project Object */
typedef struct nyproject_t nyproject_t;
/*! Opaque Target Object */
typedef struct nytarget_t nytarget_t;
/*! Opaque structure to a source */
typedef struct nysource_t nysource_t;
/*! Build */
typedef struct nybuild_t nybuild_t;
/*! VM Program */
typedef struct nyprogram_t nyprogram_t;
/*! VM Thread Context */
typedef struct nythread_t nytctx_t;
/*@}*/


#ifdef __cplusplus
}
#endif

#endif  /*__LIBNANYC_TYPES_H__ */
