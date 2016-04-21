#include <yuni/yuni.h>
#include "nany/nany.h"
#include <iostream>




static void nany_console_stdcout(void*, const char* text, size_t length)
{
	assert(!length or text != nullptr);
	std::cout.write(text, length);
}

static void nany_console_stderr(void*, const char* text, size_t length)
{
	assert(!length or text != nullptr);
	std::cerr.write(text, length);
}

static void nany_console_flush_stdcout(void*)
{
	std::cout << std::flush;
}

static void nany_console_flush_stderr(void*)
{
	std::cerr << std::flush;
}


extern "C" void nany_console_cf_set_stdcout(nyconsole_cf_t* cf)
{
	if (cf)
	{
		cf->write_stdout = &nany_console_stdcout;
		cf->write_stderr = &nany_console_stderr;
		cf->flush_stdout = &nany_console_flush_stdcout;
		cf->flush_stderr = &nany_console_flush_stderr;
		cf->internal = nullptr;
		cf->release  = nullptr;
	}
}
