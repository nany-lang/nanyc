#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::follow>& operands)
	{
		// in 'signature only' mode (resolving defined parameter types), all
		// types must be gathered
		if (not operands.symlink or signatureOnly)
		{
			auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
			auto& spare = cdeftable.substitute(operands.follower);
			spare.import(cdef);
			spare.qualifiers.merge(cdef.qualifiers);
			spare.instance = true;
		}
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
