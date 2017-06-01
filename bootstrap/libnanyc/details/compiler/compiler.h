#pragma once
#include <nanyc/program.h>
#include "details/reporting/message.h"

namespace ny {
namespace compiler {

struct Compiler final {
	Compiler(const nycompile_opts_t&);
	nyprogram_t* compile();

	const nycompile_opts_t& opts;
	Logs::Message messages{Logs::Level::none};
};

nyprogram_t* compile(nycompile_opts_t&);

} // namespace compiler
} // namespace ny
