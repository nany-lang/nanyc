#include <nany/console.h>
#include <yuni/yuni.h>
#include <yuni/core/system/console/console.h>
#include <iostream>
#include <string.h>

using namespace Yuni;


union internal_t {
	void* pointer;
	uint8_t colors[4];
};


static void nyconsole_stdcout(void*, const char* text, size_t length) {
	assert(!length or text != nullptr);
	try {
		std::cout.write(text, static_cast<std::streamsize>(length));
	}
	catch (...) {}
}


static void nyconsole_stderr(void*, const char* text, size_t length) {
	assert(!length or text != nullptr);
	try {
		std::cerr.write(text, static_cast<std::streamsize>(length));
	}
	catch (...) {}
}


static void nyconsole_flush(void*, nyconsole_output_t out) {
	try {
		switch (out) {
			case nycout:
				std::cout << std::flush;
				break;
			case nycerr:
				std::cerr << std::flush;
				break;
		}
		std::cout << std::flush;
	}
	catch (...) {}
}


constexpr static const System::Console::Color color2yunicolor[nyc_count] = {
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


static void nyconsole_set_color(void* internal, nyconsole_output_t out, nycolor_t color) {
	assert(internal != nullptr);
	assert((uint32_t) out == nycout or (uint32_t) out == nycerr);
	internal_t flags;
	flags.pointer = internal;
	if (flags.colors[out] != 0) {
		// which output
		auto& o = (out == nycout) ? std::cout : std::cerr;
		if (color == nyc_none)
			System::Console::ResetTextColor(o);
		else
			System::Console::SetTextColor(o, color2yunicolor[color]);
	}
}


static nybool_t nyconsole_has_color(void* internal, nyconsole_output_t out) {
	assert(internal != nullptr);
	assert((uint32_t) out == nycout or (uint32_t) out == nycerr);
	internal_t flags;
	flags.pointer = internal;
	return (flags.colors[out] != 0) ? nytrue : nyfalse;
}


extern "C" void nyconsole_cf_set_stdcout(nyconsole_t* cf) {
	if (cf) {
		cf->write_stdout = &nyconsole_stdcout;
		cf->write_stderr = &nyconsole_stderr;
		cf->set_color    = &nyconsole_set_color;
		cf->has_color    = &nyconsole_has_color;
		cf->flush        = &nyconsole_flush;
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


extern "C" void nyconsole_cf_copy(nyconsole_t* out, const nyconsole_t* const src) {
	if (out)
		memcpy(out, src, sizeof(nyconsole_t));
}
