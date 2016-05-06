#include <yuni/yuni.h>
#include <yuni/core/system/console/console.h>
#include "nany/console.h"
#include <iostream>

using namespace Yuni;


union internal_t
{
	void* pointer;
	uint8_t colors[4];
};

static void nany_console_stdcout(void*, const char* text, size_t length)
{
	assert(!length or text != nullptr);
	try
	{
		std::cout.write(text, static_cast<std::streamsize>(length));
	}
	catch (...) {}
}

static void nany_console_stderr(void*, const char* text, size_t length)
{
	assert(!length or text != nullptr);
	try
	{
		std::cerr.write(text, static_cast<std::streamsize>(length));
	}
	catch (...) {}
}

static void nany_console_flush(void*, nyconsole_output_t out)
{
	try
	{
		switch (out)
		{
			case nycout: std::cout << std::flush; break;
			case nycerr: std::cerr << std::flush; break;
		}
		std::cout << std::flush;
	}
	catch (...) {}
}


static void nany_console_set_color(void* internal, nyconsole_output_t out, nycolor_t color)
{
	assert(internal != nullptr);
	assert((uint32_t) out == nycout or (uint32_t) out == nycerr);

	try
	{
		internal_t flags;
		flags.pointer = internal;

		if (flags.colors[out] != 0)
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
	catch (...) {}
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
		static_assert(sizeof(internal.colors) <= sizeof(internal.pointer), "size mismatch");

		internal.colors[0]      = 0;
		internal.colors[nycout] = System::Console::IsStdoutTTY() ? 1 : 0;
		internal.colors[nycerr] = System::Console::IsStderrTTY() ? 1 : 0;
		internal.colors[3]      = 1; // to always have a non-null value to ease debugging
		cf->internal = internal.pointer;
		cf->release  = nullptr;
	}
}
