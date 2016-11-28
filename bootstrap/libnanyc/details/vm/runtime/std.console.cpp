#include "runtime.h"
#include "details/intrinsic/intrinsic-table.h"
#include <yuni/core/string.h>
#include <yuni/core/system/environment.h>

using namespace Yuni;


static void _nanyc_console_out(nyvm_t* vm, const char* text, uint32_t size) {
	assert(text and size);
	vm->console->write_stdout(vm->console->internal, text, size);
}


static void _nanyc_console_err(nyvm_t* vm, const char* text, uint32_t size) {
	assert(text and size);
	vm->console->write_stderr(vm->console->internal, text, size);
}


static void _nanyc_console_out_flush(nyvm_t* vm) {
	vm->console->flush(vm->console->internal, nycout);
}


static void _nanyc_console_err_flush(nyvm_t* vm) {
	vm->console->flush(vm->console->internal, nycerr);
}


static bool _nanyc_console_out_has_colors(nyvm_t* vm) {
	return nyfalse != vm->console->has_color(vm->console->internal, nycout);
}


static bool _nanyc_console_err_has_colors(nyvm_t* vm) {
	return nyfalse != vm->console->has_color(vm->console->internal, nycerr);
}


namespace ny {
namespace nsl {
namespace import {


void console(IntrinsicTable& intrinsics) {
	intrinsics.add("__nanyc_console_out",  _nanyc_console_out);
	intrinsics.add("__nanyc_console_err",  _nanyc_console_err);
	intrinsics.add("__nanyc_console_out_flush",  _nanyc_console_out_flush);
	intrinsics.add("__nanyc_console_err_flush",  _nanyc_console_err_flush);
	intrinsics.add("__nanyc_console_out_has_colors",  _nanyc_console_out_has_colors);
	intrinsics.add("__nanyc_console_err_has_colors",  _nanyc_console_err_has_colors);
}


} // namespace import
} // namespace nsl
} // namespace ny
