#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::assign>& operands)
	{
		auto& frame = atomStack.back();

		if (not frame.verify(operands.rhs))
			return frame.invalidate(operands.lhs);

		auto& cdef  = cdeftable.classdef(CLID{frame.atomid, operands.lhs});
		if (cdef.isAny())
		{
			// type propagation
			auto& cdefsrc = cdeftable.classdef(CLID{frame.atomid, operands.rhs});
			auto& spare = cdeftable.substitute(cdef.clid.lvid());
			spare.import(cdefsrc);
		}

		bool canDisposeLHS = (operands.disposelhs != 0);
		instanciateAssignment(atomStack.back(), operands.lhs, operands.rhs, canDisposeLHS);
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
