#pragma once
#include "vm.h"




namespace Nany
{
namespace VM
{

	inline nytctx_t* ThreadContext::self()
	{
		return reinterpret_cast<nytctx_t*>(this);
	}

	inline const nytctx_t* ThreadContext::self() const
	{
		return reinterpret_cast<const nytctx_t*>(this);
	}


	inline ThreadContext::ThreadContext(Program& program, const AnyString& name)
		: program(program)
		, cf(program.cf)
		, name(name)
	{}


	inline ThreadContext::ThreadContext(ThreadContext& rhs)
		: program(rhs.program)
		, cf(rhs.cf)
	{}


	inline void ThreadContext::cerr(const AnyString& msg)
	{
		cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
	}

	inline void ThreadContext::cerrColor(nycolor_t color)
	{
		cf.console.set_color(cf.console.internal, nycerr, color);
	}


	inline void ThreadContext::cerrException(const AnyString& msg)
	{
		cerr("\n\n");
		cerrColor(nyc_red);
		cerr("exception: ");
		cerrColor(nyc_white);
		cerr(msg);
		cerrColor(nyc_none);
		cerr("\n");
	}


} // namespace VM
} // namespace Nany
