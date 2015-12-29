#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/context.h"
#include <limits.h>

using namespace Yuni;




extern "C" nybool_t
nany_add_intrinsic(nycontext_t* ctx, const char* name, uint32_t flags, void* callback, nytype_t ret, ...)
{
	try
	{
		if (ctx and ctx->build.internal)
		{
			auto& context = *(reinterpret_cast<Nany::Context*>(ctx->build.internal));

			AnyString intrname{name};
			va_list argp;
			va_start(argp, ret);
			bool success = context.intrinsics.add(intrname, flags, callback, ret, argp);
			va_end(argp);
			return (success) ? nytrue : nyfalse;
		}
	}
	catch (...) {}
	return nyfalse;
}


extern "C" nybool_t
nany_add_intrinsic_n(nycontext_t* ctx, const char* name, size_t length, uint32_t flags, void* callback, nytype_t ret, ...)
{
	try
	{
		if (ctx and ctx->build.internal)
		{
			auto& context = *(reinterpret_cast<Nany::Context*>(ctx->build.internal));

			AnyString intrname{name, length};
			va_list argp;
			va_start(argp, ret);
			bool success = context.intrinsics.add(intrname, flags, callback, ret, argp);
			va_end(argp);
			return (success) ? nytrue : nyfalse;
		}
	}
	catch (...) {}
	return nyfalse;
}
