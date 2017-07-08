#pragma once
#include "libnanyc.h"
#include <exception>


namespace ny {
namespace vm {

union Register {
	uint64_t u64;
	int64_t i64;
	double f64;
};


struct CodeAbort final : public std::exception {
	virtual const char* what() const throw() {
		return "";
	}
};


} // namespace vm
} // namespace ny
