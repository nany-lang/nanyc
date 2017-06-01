#include <nanyc/program.h>
#include "details/program/program.h"
#include "details/compiler/compile.h"
#include "libnanyc.h"
#include <cstring>


namespace {

nyprogram_t* compile_from_source(nycompile_opts_t* opts, nysource_opts_t& source) {
	nycompile_opts_t newopts;
	if (opts)
		newopts = *opts;
	else
		memset(&newopts, 0x0, sizeof(nycompile_opts_t));
	newopts.sources.items = &source;
	newopts.sources.count = 1;
	return ny::compiler::compile(newopts);
}

} // namespace

nyprogram_t* nyprogram_compile(nycompile_opts_t* opts) {
	if (likely(opts))
		return ny::compiler::compile(*opts);
	return nullptr;
}

nyprogram_t* nyprogram_compile_from_file(nycompile_opts_t* opts, const char* filename, size_t len) {
	if (unlikely(!filename or len == 0))
		return nullptr;
	nysource_opts_t source;
	memset(&source, 0x0, sizeof(nysource_opts_t));
	source.filename.c_str = filename;
	source.filename.len = len;
	return compile_from_source(opts, source);
}

nyprogram_t* nyprogram_compile_from_content(nycompile_opts_t* opts, const char* content, size_t len) {
	if (unlikely(!content or len == 0))
		return nullptr;
	nysource_opts_t source;
	memset(&source, 0x0, sizeof(nysource_opts_t));
	source.content.c_str = content;
	source.content.len = len;
	return compile_from_source(opts, source);
}

void nyprogram_free(nyprogram_t* program) {
	delete ny::Program::pointer(program);
}
