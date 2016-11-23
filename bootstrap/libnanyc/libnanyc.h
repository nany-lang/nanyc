#pragma once
#include <yuni/yuni.h>

#ifdef YUNI_OS_MSVC
#pragma warning(disable: 4251) // class 'type' needs to have dll-interface
#endif

#define likely(X)    YUNI_LIKELY(X)
#define unlikely(X)  YUNI_UNLIKELY(X)
