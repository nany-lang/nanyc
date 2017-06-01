#pragma once
#include <nanyc/program.h>

namespace ny {
namespace compiler {

struct Compiler final {
	Compiler(const nycompile_opts_t&);
	nyprogram_t* compile();

	const nycompile_opts_t& opts;
};

nyprogram_t* compile(nycompile_opts_t&);

} // namespace compiler
} // namespace ny
