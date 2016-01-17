#pragma once
#include <yuni/yuni.h>
#include "../fwd.h"
#include <exception>



namespace Nany
{
namespace VM
{

	union DataRegister
	{
		uint64_t u64;
		int64_t i64;
		double f64;
	};


	struct CodeException: public std::exception
	{
		virtual const char* what() const throw() { return ""; }
	};







} // namespace VM
} // namespace Nany
