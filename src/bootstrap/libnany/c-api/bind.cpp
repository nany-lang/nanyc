#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/context.h"
#include <limits.h>

using namespace Yuni;




extern "C" nybool_t
nany_bind(nycontext_t* ctx, const char* name, uint32_t flags, void* callback, nytype_t ret, ...)
{
	try
	{
		AnyString intrname{name};
		if (intrname.size() < 128 and not intrname.empty())
		{
			if (ctx and ctx->internal)
			{
				auto& context = *(reinterpret_cast<Nany::Context*>(ctx->internal));
				va_list argp;
				va_start(argp, ret);
				bool success = context.intrinsics.add(intrname, flags, callback, ret, argp);
				va_end(argp);
				return (success) ? nytrue : nyfalse;
			}
		}
	}
	catch (...) {}
	return nyfalse;
}


extern "C" nybool_t
nany_bind_n(nycontext_t* ctx, const char* name, size_t length, uint32_t flags, void* callback, nytype_t ret, ...)
{
	try
	{
		if (ctx and ctx->internal and length < 128 and length > 0)
		{
			auto& context = *(reinterpret_cast<Nany::Context*>(ctx->internal));

			AnyString intrname{name, static_cast<uint32_t>(length)};
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
