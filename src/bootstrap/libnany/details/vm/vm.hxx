#pragma once
#include "vm.h"



namespace Nany
{
namespace VM
{

	inline Program::Program(nycontext_t& context, const IR::Sequence* sequence, const AtomMap& map)
		: context(context)
		, sequence(sequence)
		, map(map)
	{}


	inline Program::Program(Program& inherit)
		: context(inherit.context)
		, sequence(inherit.sequence)
		, map(inherit.map)
	{}





} // namespace VM
} // namespace Nany
