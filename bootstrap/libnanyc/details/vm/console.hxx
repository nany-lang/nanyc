#pragma once
#include "console.h"


namespace ny {
namespace vm {
namespace console {


inline void cout(const nyprogram_cf_t& cf, const AnyString& string) noexcept {
	cf.console.write_stdout(cf.console.internal, string.c_str(), string.size());
}


inline void cerr(const nyprogram_cf_t& cf, const AnyString& string) noexcept {
	cf.console.write_stderr(cf.console.internal, string.c_str(), string.size());
}


inline void cout(const Context& context, const AnyString& string) noexcept {
	cout(context.cf, string);
}


inline void cerr(const Context& context, const AnyString& string) noexcept {
	cerr(context.cf, string);
}


inline void color(const nyprogram_cf_t& cf, nyconsole_output_t output, nycolor_t cl) noexcept {
	cf.console.set_color(cf.console.internal, output, cl);
}


inline void color(const Context& context, nyconsole_output_t output, nycolor_t cl) noexcept {
	color(context.cf, output, cl);
}


} // namespace console
} // namespace vm
} // namespace ny
