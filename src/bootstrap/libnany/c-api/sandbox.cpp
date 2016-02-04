#include "nany/nany.h"
#include <yuni/core/string.h>
#include <yuni/io/file.h>
#ifndef YUNI_OS_MSVC
#include <unistd.h>
#endif
#include <iostream>

#ifdef YUNI_OS_WINDOWS
#include <io.h>
#endif

using namespace Yuni;




#ifdef YUNI_OS_WINDOWS
static inline int fsync(int fd)
{
	// thanks sqlite !
	HANDLE h = (HANDLE) _get_osfhandle (fd);
	if (INVALID_HANDLE_VALUE == h)
	{
		errno = EBADF;
		return -1;
	}

	if (FALSE == FlushFileBuffers(h))
	{
		// Translate some Windows errors into rough approximations of Unix
		// errors.  MSDN is useless as usual - in this case it doesn't
		// document the full range of errors.
		DWORD err = GetLastError();
		switch (err)
		{
			case ERROR_ACCESS_DENIED:
			{
				// For a read-only handle, fsync should succeed, even though we have
				// no way to sync the access-time changes
				return 0;
			}
				// eg. Trying to fsync a tty.
			case ERROR_INVALID_HANDLE:
			{
				errno = EINVAL;
				break;
			}
			default:
			{
				errno = EIO;
			}
		}
		return -1;
	}
	return 0;
}
#endif


extern "C" void nanysdbx_not_enough_memory(nycontext_t* ctx)
{
	constexpr const char* const msg = "error: not enough memory\n";
	auto len = strlen(msg);
	ctx->console.write_stderr(ctx, msg, len);
	ctx->console.flush_stderr(ctx);
}


extern "C" void nanysdbx_write_stdout(nycontext_t*, const char* text, size_t length)
{
	if (YUNI_LIKELY(text != nullptr and length))
		std::cout.write(text, static_cast<std::streamsize>(length));
}


extern "C" void nanysdbx_write_stderr(nycontext_t*, const char* text, size_t length)
{
	if (YUNI_LIKELY(text != nullptr and length))
		std::cerr.write(text, static_cast<std::streamsize>(length));
}


extern "C" void nanysdbx_flush_stdout(nycontext_t*)
{
	#ifndef YUNI_OS_WINDOWS
	::fsync(STDOUT_FILENO);
	#endif
}


extern "C" void nanysdbx_flush_stderr(nycontext_t*)
{
	#ifndef YUNI_OS_WINDOWS
	::fsync(STDERR_FILENO);
	#endif
}


extern "C" void* nanysdbx_mem_alloc(nycontext_t* ctx, size_t size)
{
	assert(0 != size);
	if (YUNI_LIKELY((ctx->reserved.mem0 += size) < ctx->memory.limit_mem_size))
	{
		void* p = ::malloc(size);
		if (YUNI_LIKELY(p))
			return p;
	}

	ctx->reserved.mem0 -= size;
	if (ctx->memory.on_not_enough_memory)
		ctx->memory.on_not_enough_memory(ctx);
	return nullptr;
}


extern "C" void* nanysdbx_mem_realloc(nycontext_t* ctx, void* ptr, size_t oldsize, size_t newsize)
{
	if (newsize > oldsize)
	{
		if (YUNI_LIKELY((ctx->reserved.mem0 += newsize - oldsize) < ctx->memory.limit_mem_size))
		{
			void* p = ::realloc(ptr, newsize);
			if (YUNI_LIKELY(p))
				return p;

			ctx->reserved.mem0 -= newsize - oldsize;
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
		ctx->memory.on_not_enough_memory(ctx);
	return nullptr;
}


extern "C" void nanysdbx_mem_free(nycontext_t* ctx, void* ptr, size_t size)
{
	if (ptr)
	{
		free(ptr);
		ctx->reserved.mem0 -= size;
	}
}


extern "C" nyfd_t nanysdbx_io_open(nycontext_t*, const char* path, size_t length, uint32_t)
{
	if (YUNI_UNLIKELY(length > 1024 * 1024))
		return nullptr;
	AnyString filename{path, (uint32_t)length};
	if (not filename.empty())
	{
		String canonfile;
	}
	return nullptr;
}


extern "C" void nanysdbx_io_close(nycontext_t*, nyfd_t fd)
{
	if (fd) {}
}


extern "C" size_t nanysdbx_io_read(nycontext_t*, nyfd_t, uint8_t*, size_t)
{
	return 0;
}


extern "C" size_t nanysdbx_io_write(nycontext_t*, nyfd_t, uint8_t*, size_t)
{
	return 0;
}


extern "C" void nanysdbx_build_on_err_file_access(nycontext_t* ctx, const char* filename, size_t length)
{
	String msg;
	msg << "error: file not found: " << AnyString{filename, (uint32_t) length};
	ctx->console.write_stderr(ctx, msg.c_str(), msg.size());
	ctx->console.flush_stderr(ctx);
}
