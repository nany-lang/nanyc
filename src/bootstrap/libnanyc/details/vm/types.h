#pragma once
#include <yuni/yuni.h>
#include "../fwd.h"
#include <exception>


namespace ny
{
namespace vm
{

	union DataRegister
	{
		uint64_t u64;
		int64_t i64;
		double f64;
	};


	struct CodeAbort final : public std::exception
	{
		virtual const char* what() const throw() { return ""; }
	};


} // namespace vm
} // namespace ny
