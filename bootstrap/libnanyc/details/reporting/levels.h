#pragma once
#include <cstdint>




namespace ny
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


	inline bool isError(Level level)
	{
		switch (level)
		{
			default:
				break;
			case Level::error:
			case Level::ICE:
				return true;
		}
		return false;
	}



} // namespace Logs
} // namespace ny
