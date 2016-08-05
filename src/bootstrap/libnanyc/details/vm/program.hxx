#pragma once
#include "program.h"



namespace Nany
{

	inline VM::Program& ref(nyprogram_t* const ptr)
	{
		assert(ptr != nullptr);
		return *(reinterpret_cast<Nany::VM::Program*>(ptr));
	}

	inline const VM::Program& ref(const nyprogram_t* const ptr)
	{
		assert(ptr != nullptr);
		return *(reinterpret_cast<const Nany::VM::Program*>(ptr));
	}

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


	inline Program::Program(const nyprogram_cf_t& cf, nybuild_t* build)
		: cf(cf)
		, build(build)
		, map(ref(build).cdeftable.atoms)
	{
		ref(build).addRef(); // nany_build_ref()
	}


	inline Program::~Program()
	{
		auto& b = ref(build); // nany_build_unref(&build);
		if (b.release())
			b.destroy();
	}


	inline void Program::printStderr(const AnyString& msg)
	{
		cf.console.write_stderr(cf.console.internal, msg.c_str(), msg.size());
	}




} // namespace VM
} // namespace Nany
