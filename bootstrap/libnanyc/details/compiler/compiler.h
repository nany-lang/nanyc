#pragma once
#include <nanyc/program.h>
#include "details/reporting/message.h"
#include <memory>
#include <cassert>

namespace ny {
namespace compiler {

struct Source final {
	yuni::String content;
	yuni::String filename;
};

struct Compiler final {
	Compiler(const nycompile_opts_t&);
	nyprogram_t* compile();

	const nycompile_opts_t& opts;
	Logs::Message messages{Logs::Level::none};
	struct final {
		uint32_t count = 0;
		std::unique_ptr<Source[]> items;
		Source& operator [] (uint32_t i) { assert(i < count); return items[i]; }
	}
	sources;
};

nyprogram_t* compile(nycompile_opts_t&);

} // namespace compiler
} // namespace ny
