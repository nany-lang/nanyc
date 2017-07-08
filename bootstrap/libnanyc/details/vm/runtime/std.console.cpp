#include "runtime.h"
#include "details/intrinsic/catalog.h"
#include <yuni/core/string.h>
#include <yuni/core/system/environment.h>

using namespace Yuni;


static void _nanyc_console_out(nyoldvm_t* vm, const char* text, uint32_t size) {
	assert(text and size);
	vm->console->write_stdout(vm->console->internal, text, size);
}


static void _nanyc_console_err(nyoldvm_t* vm, const char* text, uint32_t size) {
	assert(text and size);
	vm->console->write_stderr(vm->console->internal, text, size);
}


static void _nanyc_console_out_flush(nyoldvm_t* vm) {
	vm->console->flush(vm->console->internal, nycout);
}


static void _nanyc_console_err_flush(nyoldvm_t* vm) {
	vm->console->flush(vm->console->internal, nycerr);
}


static bool _nanyc_console_out_has_colors(nyoldvm_t* vm) {
	return nyfalse != vm->console->has_color(vm->console->internal, nycout);
}


static bool _nanyc_console_err_has_colors(nyoldvm_t* vm) {
	return nyfalse != vm->console->has_color(vm->console->internal, nycerr);
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
