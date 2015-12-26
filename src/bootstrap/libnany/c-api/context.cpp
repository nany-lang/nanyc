#include <yuni/yuni.h>
#include "nany/nany.h"
#include "details/context/context.h"
#include <limits.h>

using namespace Yuni;




extern "C" nycontext_t* nany_create(const nycontext_t* inherit)
{
	if (inherit and inherit->build.on_context_create_query)
	{
		if (YUNI_UNLIKELY(nyfalse == inherit->build.on_context_create_query(inherit)))
			return nullptr;
	}

	nycontext_t* ctx = (nycontext_t*) malloc(sizeof(nycontext_t));
	if (YUNI_LIKELY(ctx))
	{
		nybool_t success = nany_initialize(ctx, inherit);
		if (YUNI_UNLIKELY(success == nyfalse))
		{
			free(ctx);
			ctx = nullptr;
		}
	}
	return ctx;
}


extern "C" nybool_t nany_initialize(nycontext_t* ctx, const nycontext_t* inherit)
{
	if (YUNI_UNLIKELY(!ctx))
		return nyfalse;
	try
	{
		if (inherit)
		{
			memcpy(ctx, inherit, sizeof(nycontext_t));

			if (ctx->io.chroot_path_size != 0)
			{
				size_t len = ctx->io.chroot_path_size;
				char* cstr = (char*) malloc(sizeof(char) * len + 1);
				memcpy(cstr, ctx->io.chroot_path, sizeof(char) * len);
				cstr[len] = '\0';
				ctx->io.chroot_path = cstr;
			}
			else
				ctx->io.chroot_path = nullptr; // just in case

			// re-acquire the queueservice (if any)
			if (ctx->queueservice)
				reinterpret_cast<Job::QueueService*>(ctx->queueservice)->addRef();

			auto& rhs = *(reinterpret_cast<Nany::Context*>(inherit->build.internal));
			ctx->build.internal = new Nany::Context(*ctx, rhs);
		}
		else
		{
			memset(ctx, 0x0, sizeof(nycontext_t));
			// default implementation
			ctx->memory.allocate   = nanysdbx_mem_alloc;
			ctx->memory.reallocate = nanysdbx_mem_realloc;
			ctx->memory.release    = nanysdbx_mem_free;
			ctx->memory.limits_mem_size = 0;
			ctx->memory.on_not_enough_memory = nanysdbx_not_enough_memory;

			ctx->io.chroot_path = nullptr;
			ctx->io.chroot_path_size = 0;
			ctx->io.open  = nanysdbx_io_open;
			ctx->io.close = nanysdbx_io_close;
			ctx->io.read  = nanysdbx_io_read;
			ctx->io.write = nanysdbx_io_write;

			ctx->console.write_stdout = nanysdbx_write_stdout;
			ctx->console.write_stderr = nanysdbx_write_stderr;
			ctx->console.flush_stdout = nanysdbx_flush_stdout;
			ctx->console.flush_stderr = nanysdbx_flush_stderr;

			// build
			ctx->build.internal = new Nany::Context(*ctx);
			ctx->build.on_err_file_access = nanysdbx_build_on_err_file_access;
		}


		if (ctx->build.on_context_create)
			ctx->build.on_context_create(ctx, inherit);

		return nytrue;
	}
	catch (...)
	{}
	return nyfalse;
}


extern "C" void nany_uninitialize(nycontext_t* context)
{
	if (context)
	{
		if (context->build.on_context_destroy)
			context->build.on_context_destroy(context);

		try
		{
			delete (reinterpret_cast<Nany::Context*>(context->build.internal));
		}
		catch (...){}

		if (context->queueservice)
			nany_queueservice_unref(&(context->queueservice));

		#ifndef NDEBUG
		// to avoid bad reuse (see MALLOC_OPTIONS on UNIXs)
		memset(context, 0xEF, sizeof(nycontext_t));
		#endif
	}
}


extern "C" void nany_dispose(nycontext_t** context)
{
	if (context and *context)
	{
		nany_uninitialize(*context);
		free(*context);
		*context = nullptr;
	}
}


extern "C" void nany_lock(const nycontext_t* ctx)
{
	assert(ctx != nullptr);
	(reinterpret_cast<Nany::Context*>(ctx->build.internal))->mutex.lock();
}


extern "C" void nany_unlock(const nycontext_t* ctx)
{
	assert(ctx != nullptr);
	(reinterpret_cast<Nany::Context*>(ctx->build.internal))->mutex.unlock();
}


extern "C" nybool_t nany_trylock(const nycontext_t* ctx)
{
	assert(ctx != nullptr);
	bool success = (reinterpret_cast<Nany::Context*>(ctx->build.internal))->mutex.trylock();
	return (success) ? nytrue : nyfalse;
}


extern "C" nybool_t nany_build(nycontext_t* ctx, nyreport_t** report)
{
	if (YUNI_UNLIKELY(!ctx))
		return nyfalse;

	if (!(*report))
		*report = nany_report_create();
	nany_report_add_compiler_headerinfo(*report);

	auto& context = *(reinterpret_cast<Nany::Context*>(ctx->build.internal));
	auto* message = reinterpret_cast<Nany::Logs::Message*>(*report);
	bool  success = context.build(message);
	return success ? nytrue : nyfalse;
}


extern "C" int nany_run_main(nycontext_t* ctx, int argc, const char** argv)
{
	if (YUNI_UNLIKELY(!ctx))
		return INT_MIN;
	if (YUNI_UNLIKELY(not (argc > 0) or argv == nullptr))
		return INT_MIN;
	if (YUNI_UNLIKELY(!ctx->build.internal))
		return INT_MIN;

	bool success = false;
	auto& context = *(reinterpret_cast<Nany::Context*>(ctx->build.internal));

	int exitstatus = context.run(success);
	return (success) ? static_cast<int>(exitstatus) : INT_MIN;
}
