/*
** Nany - https://nany.io
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __LIBNANY_NANY_C_TYPES_H__
#define __LIBNANY_NANY_C_TYPES_H__
#include <string.h>
#include <stdint.h>


#if defined(_WIN32) || defined(__CYGWIN__)
#	ifdef __GNUC__
#		define LIBNANY_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#		define LIBNANY_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#	else
#		define LIBNANY_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#		define LIBNANY_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#	endif
#else
#	define LIBNANY_VISIBILITY_EXPORT       __attribute__((visibility("default")))
#	define LIBNANY_VISIBILITY_IMPORT       __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBNANY_DLL_EXPORT)
# define LIBNANY_DLL_EXPORT
#endif


#if defined(__clang__) || defined(__GNUC__)
# define LIBNANY_ATTR_ALLOCSIZE(A)    __attribute__((alloc_size(A)))
# define LIBNANY_ATTR_ALLOCSIZE2(A,B) __attribute__((alloc_size(A, B)))
#else
# define LIBNANY_ATTR_ALLOCSIZE(A)
# define LIBNANY_ATTR_ALLOCSIZE2(A, B)
#endif



/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBNANY_DLL_EXPORT)
#	define NY_EXPORT LIBNANY_VISIBILITY_EXPORT
#else
#	define NY_EXPORT LIBNANY_VISIBILITY_IMPORT
#endif








#ifdef __cplusplus
extern "C" {
#endif





/*! nybool_t */
typedef enum nybool_t {nyfalse, nytrue} nybool_t;


/*! Task status */
enum nytask_status_t
{
	/*! The task has failed */
	nys_failed = 0,
	/*! The task has succeeded */
	nys_succeeded,
	/*! The task is still running */
	nys_running,
	/*! The task is waiting for being launched */
	nys_idle,
	/*! The task has been canceled */
	nys_canceled
};
typedef enum nytask_status_t  nytask_status_t;


/*!
** \brief Nany builtin types
*/
typedef enum /* nytype_t */
{
	/*! No type */
	nyt_void = 0,
	/*! Custom user type */
	nyt_any,
	/*! Raw pointer (arch dependent) */
	nyt_pointer,
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
	nyt_f64,

	/*! The total number of intrinsic types */
	nyt_count

} nytype_t;






/*!
** \brief Identifiers' visibility
**
** \internal All values are strictly ordered
*/
typedef enum /* nyvisibility_t */
{
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
	nyv_published,
	/*! dummy */
	nyv_max

} nyvisibility_t;



/*! Intrnsic attributes */
typedef uint32_t nybind_flag_t;

/*! Flags for intrinsic attributes */
enum
{
	/*! Default settings */
	nybind_default = 0
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




#ifdef __cplusplus
}
#endif

#endif /* __LIBNANY_NANY_C_TYPES_H__ */
