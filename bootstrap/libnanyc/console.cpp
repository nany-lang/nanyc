#include <nanyc/console.h>
#include "libnanyc.h"
#include <stdio.h>


namespace {

void import_console_opts(nyconsole_t* console, const nyconsole_opts_t* opts) {
	console->userdata = opts->userdata;
	console->write = opts->write;
	console->flush = opts->flush;
	console->set_color = opts->set_color;
	console->set_bkcolor = opts->set_bkcolor;
	console->on_dispose = opts->on_dispose;
}

void nanyc_console_stdout_write(nyconsole_t*, const char* utf8s, size_t length) {
	fwrite(utf8s, length, 1, stdout);
}

void nanyc_console_stdout_flush(nyconsole_t*) {
	fflush(stdout);
}

void nanyc_console_stderr_write(nyconsole_t*, const char* utf8s, size_t length) {
	fwrite(utf8s, length, 1, stderr);
}

void nanyc_console_stderr_flush(nyconsole_t*) {
	fflush(stderr);
}

void nanyc_console_set_color_dummy(nyconsole_t*, nycolor_t) {
}

void nanyc_console_set_bkcolor_dummy(nyconsole_t*, nycolor_t) {
}

} // namespace

void nyconsole_init_from_stdout(nyconsole_t* console) {
	console->userdata = nullptr;
	console->write = &nanyc_console_stdout_write;
	console->flush = &nanyc_console_stdout_flush;
	console->set_color = nanyc_console_set_color_dummy;
	console->set_bkcolor = nanyc_console_set_bkcolor_dummy;
	console->on_dispose = nullptr;
}

void nyconsole_init_from_stderr(nyconsole_t* console) {
	console->userdata = nullptr;
	console->write = &nanyc_console_stderr_write;
	console->flush = &nanyc_console_stderr_flush;
	console->set_color = nanyc_console_set_color_dummy;
	console->set_bkcolor = nanyc_console_set_bkcolor_dummy;
	console->on_dispose = nullptr;
}

void nyconsole_init(nyconsole_t* console, const nyconsole_opts_t* opts) {
	if (unlikely(console))
		return;
	if (unlikely(opts == nullptr))
		return nyconsole_init_from_stdout(console);
	import_console_opts(console, opts);
}

nyconsole_t* nyconsole_make(const nyconsole_opts_t* opts) {
	auto* console = (nyconsole_t*)::malloc(sizeof(nyconsole_t));
	if (console)
		import_console_opts(console, opts);
	return console;
}

nyconsole_t* nyconsole_make_from_stdout() {
	auto* console = (nyconsole_t*)::malloc(sizeof(nyconsole_t));
	if (console)
		nyconsole_init_from_stdout(console);
	return console;
}

nyconsole_t* nyconsole_make_from_stderr() {
	auto* console = (nyconsole_t*)::malloc(sizeof(nyconsole_t));
	if (console)
		nyconsole_init_from_stderr(console);
	return console;
}

void nyconsole_dispose(nyconsole_t* console) {
	if (console) {
		if (console->on_dispose)
			console->on_dispose(console);
		free(console);
	}
}
