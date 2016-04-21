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


	inline void ThreadContext::printStderr(const AnyString& msg)
	{
		cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
	}



} // namespace VM
} // namespace Nany
