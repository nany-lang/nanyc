#include "runtime.h"
#include "details/intrinsic/catalog.h"
#include <yuni/core/string.h>
#include <yuni/core/system/environment.h>

using namespace Yuni;


static void _nanyc_console_out(nyvmthread_t* vm, const char* text, uint32_t size) {
	assert(text and size);
	vm->cout.write(&vm->cout, text, size);
}


static void _nanyc_console_err(nyvmthread_t* vm, const char* text, uint32_t size) {
	assert(text and size);
	vm->cerr.write(&vm->cerr, text, size);
}


static void _nanyc_console_out_flush(nyvmthread_t* vm) {
	vm->cout.flush(&vm->cout);
}


static void _nanyc_console_err_flush(nyvmthread_t* vm) {
	vm->cerr.flush(&vm->cerr);
}


static bool _nanyc_console_out_has_colors(nyvmthread_t*) {
	return nyfalse;
}


static bool _nanyc_console_err_has_colors(nyvmthread_t*) {
	return nyfalse;
}


namespace ny {
namespace nsl {
namespace import {


void console(ny::intrinsic::Catalog& intrinsics) {
	intrinsics.emplace("__nanyc_console_out",  _nanyc_console_out);
	intrinsics.emplace("__nanyc_console_err",  _nanyc_console_err);
	intrinsics.emplace("__nanyc_console_out_flush",  _nanyc_console_out_flush);
	intrinsics.emplace("__nanyc_console_err_flush",  _nanyc_console_err_flush);
	intrinsics.emplace("__nanyc_console_out_has_colors",  _nanyc_console_out_has_colors);
	intrinsics.emplace("__nanyc_console_err_has_colors",  _nanyc_console_err_has_colors);
}


} // namespace import
} // namespace nsl
} // namespace ny
