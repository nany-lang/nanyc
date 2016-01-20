#pragma once
#include "details/ir/sequence.h"
#include "nany/nany.h"
#include "details/atom/atom-map.h"
#include "details/context/context.h"
#include "libnany-config.h"



namespace Nany
{
namespace VM
{


	/*!
	** \brief Execute a sequence within its sandbox
	*/
	int execute(bool& success, nycontext_t&, const IR::Sequence& sequence, const AtomMap&);





} // namespace VM
} // namespace Nany
