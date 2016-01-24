#pragma once
#include "vm.h"



namespace Nany
{
namespace VM
{

	inline Program::Program(nycontext_t& context, const AtomMap& map)
		: context(context)
		, map(map)
	{}


	inline Program::Program(Program& inherit)
		: context(inherit.context)
		, map(inherit.map)
	{}





} // namespace VM
} // namespace Nany
