#include <yuni/yuni.h>
#include <yuni/core/system/console/console.h>
#include "nany/console.h"
#include <iostream>

using namespace Yuni;


union internal_t
{
	void* pointer;
	nybool_t colors[3];
};

static void nany_console_stdcout(void*, const char* text, size_t length)
{
	assert(!length or text != nullptr);
	std::cout.write(text, static_cast<std::streamsize>(length));
}

static void nany_console_stderr(void*, const char* text, size_t length)
{
	assert(!length or text != nullptr);
	std::cerr.write(text, static_cast<std::streamsize>(length));
}

static void nany_console_flush(void*, nyconsole_output_t out)
{
	switch (out)
	{
		case nycout: std::cout << std::flush; break;
		case nycerr: std::cerr << std::flush; break;
	}
	std::cout << std::flush;
}


static void nany_console_set_color(void* internal, nyconsole_output_t out, nycolor_t color)
{
	internal_t flags;
	flags.pointer = internal;

	if (flags.colors[out] != nyfalse)
	{
		// which output
		auto& o = (out == nycout) ? std::cout : std::cerr;

		if (color == nyc_none)
		{
			System::Console::ResetTextColor(o);
		}
		else
		{
			constexpr static const System::Console::Color mapping[] =
			{
				System::Console::none, // none

				System::Console::black,
				System::Console::red,
				System::Console::green,
				System::Console::yellow,
				System::Console::blue,
				System::Console::purple,
				System::Console::gray,
				System::Console::white,
				System::Console::lightblue,
			};
			System::Console::SetTextColor(o, mapping[color]);
		}
	}
}


extern "C" void nany_console_cf_set_stdcout(nyconsole_t* cf)
{
	if (cf)
	{
		cf->write_stdout = &nany_console_stdcout;
		cf->write_stderr = &nany_console_stderr;
		cf->set_color    = &nany_console_set_color;
		cf->flush        = &nany_console_flush;

		internal_t internal;
		internal.colors[0]      = nyfalse;
		internal.colors[nycout] = System::Console::IsStdoutTTY() ? nytrue : nyfalse;
		internal.colors[nycerr] = System::Console::IsStderrTTY() ? nytrue : nyfalse;
		cf->internal = internal.pointer;

		cf->release  = nullptr;
	}
}
