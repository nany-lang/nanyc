#pragma once
#include "vm.h"



namespace Nany
{
namespace VM
{


	inline nyprogram_t* Program::self()
	{
		return reinterpret_cast<nyprogram_t*>(this);
	}


	inline const nyprogram_t* Program::self() const
	{
		return reinterpret_cast<const nyprogram_t*>(this);
	}


	inline Program::Program(Build& build, const AtomMap& map)
		: build(build)
		, map(map)
	{
		memset(&cf, 0x0, sizeof(cf));
		cf.allocator = build.cf.allocator;
		cf.console   = build.cf.console;
		cf.program   = self();
	}


	inline Program::Program(Program& inherit)
		: build(inherit.build)
		, map(inherit.map)
	{
		memset(&cf, 0x0, sizeof(cf));
		cf.allocator = build.cf.allocator;
		cf.console   = build.cf.console;
		cf.program   = self();
	}


	inline void Program::printStderr(const AnyString& msg)
	{
		cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
	}




} // namespace VM
} // namespace Nany
