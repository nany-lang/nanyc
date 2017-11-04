#pragma once
#include <cstdint>

namespace ny {
namespace vm {

struct InvalidLabel final {
	InvalidLabel(uint32_t atomid, uint32_t label): atomid(atomid), label(label) {}
	uint32_t atomid;
	uint32_t label;
};

struct DivideByZero final {
};

struct Assert final {
};

struct UnexpectedOpcode final {
	UnexpectedOpcode(const AnyString& name): name(name) {}
	AnyString name;
};

struct InvalidDtor final {
	InvalidDtor(const Atom&) {}
};

struct ICE final {
	ICE(uint32_t line, const char* msg): line(line), msg(msg) {}
	const char* file = __FILE__;
	uint32_t line;
	const char* msg;
};

struct InvalidCast final {
};

} // vm
} // ny
