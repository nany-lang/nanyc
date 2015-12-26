#pragma once
#include <cstdint>




namespace Nany
{
namespace Logs
{

	enum class Level : std::uint8_t
	{
		ICE,
		error,
		warning,
		hint,
		suggest,
		success,
		info,
		verbose,
		trace,
		none,
	};





} // namespace Logs
} // namespace Nany
