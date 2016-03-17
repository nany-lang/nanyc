#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/context.h"
#include <limits.h>
#include "details/utils/memory-allocator.h"

using namespace Yuni;




extern "C" void nanysdbx_not_enough_memory(nycontext_t* ctx, nybool_t limit_reached)
{
	if (ctx)
	{
		ShortString128 msg;
		msg = "error: not enough memory";
		if (limit_reached == nytrue)
			msg << ", sandbox limit reached (" << ctx->memory.limit_mem_size << " bytes)\n";
		else
			msg << " from the system\n";

		ctx->console.write_stderr(ctx, msg.c_str(), msg.sizeInBytes());
		ctx->console.flush_stderr(ctx);
	}
}






static void* nymemalloc_default_allocate(nycontext_t* ctx, size_t size)
{
	assert(0 != size);
	assert(ctx != nullptr);
	void* p = ::malloc(size);
	if (YUNI_LIKELY(!p) and ctx->memory.on_not_enough_memory)
		ctx->memory.on_not_enough_memory(ctx, nyfalse);
	return p;
}


static void* nymemalloc_default_reallocate(nycontext_t* ctx, void* ptr, size_t, size_t newsize)
{
	assert(0 != newsize);
	assert(ctx != nullptr);
	void* p = ::realloc(ptr, newsize);
	if (YUNI_LIKELY(!p) and ctx->memory.on_not_enough_memory)
		ctx->memory.on_not_enough_memory(ctx, nyfalse);
	return p;
}


static void nymemalloc_default_release(nycontext_t*, void* ptr, size_t)
{
	::free(ptr);
}


extern "C" void nany_memalloc_init_default(nycontext_memory_t* mem)
{
	if (YUNI_LIKELY(mem))
	{
		mem->allocate   = &nymemalloc_default_allocate;
		mem->reallocate = &nymemalloc_default_reallocate;
		mem->release    = &nymemalloc_default_release;
		mem->limit_mem_size = (size_t) -1; // just to have a value set
		mem->on_not_enough_memory = &nanysdbx_not_enough_memory;
	}
}







static void* nymemalloc_withlimit_allocate(nycontext_t* ctx, size_t size)
{
	assert(0 != size);
	assert(ctx != nullptr);
	// TODO not thread safe
	if (YUNI_LIKELY((ctx->reserved.mem0 += size) < ctx->memory.limit_mem_size))
	{
		void* p = ::malloc(size);
		if (YUNI_LIKELY(p))
			return p;

		if (ctx->memory.on_not_enough_memory)
			ctx->memory.on_not_enough_memory(ctx, nyfalse);
	}
	else
	{
		if (ctx->memory.on_not_enough_memory)
			ctx->memory.on_not_enough_memory(ctx, nytrue);
	}
	ctx->reserved.mem0 -= size;
	return nullptr;
}


static void* nymemalloc_withlimit_reallocate(nycontext_t* ctx, void* ptr, size_t oldsize, size_t newsize)
{
	assert(ctx != nullptr);
	// in this implementation, the total allocated is based on the fact that
	// it costs nothing to reallocate a chunk of memory smaller than the previous one
	// A safer implementation would probably consider that the cost is always based on
	// the old size + the new one

	if (newsize > oldsize)
	{
		// TODO not thread safe
		if (YUNI_LIKELY((ctx->reserved.mem0 += (newsize - oldsize)) < ctx->memory.limit_mem_size))
		{
			void* p = ::realloc(ptr, newsize);
			if (YUNI_LIKELY(p))
				return p;

			ctx->reserved.mem0 -= newsize - oldsize;
		}
		else
		{
			if (ctx->memory.on_not_enough_memory)
				ctx->memory.on_not_enough_memory(ctx, nytrue);
			return nullptr;
		}
	}
	else
	{
		void* p = ::realloc(ptr, newsize);
		if (YUNI_LIKELY(p))
		{
			ctx->reserved.mem0 -= oldsize - newsize;
			return p;
		}
	}

	if (ctx->memory.on_not_enough_memory)
		ctx->memory.on_not_enough_memory(ctx, nyfalse);
	return nullptr;
}


static void nymemalloc_withlimit_release(nycontext_t* ctx, void* ptr, size_t size)
{
	assert(ctx != nullptr);
	if (YUNI_LIKELY(ptr))
	{
		free(ptr);
		// TODO not thread safe
		ctx->reserved.mem0 -= size;
	}
}


extern "C" void nany_memalloc_init_with_limit(nycontext_memory_t* mem, size_t limit)
{
	if (YUNI_LIKELY(mem))
	{
		mem->allocate   = &nymemalloc_withlimit_allocate;
		mem->reallocate = &nymemalloc_withlimit_reallocate;
		mem->release    = &nymemalloc_withlimit_release;
		mem->limit_mem_size = limit;
		mem->on_not_enough_memory = &nanysdbx_not_enough_memory;
	}
}
