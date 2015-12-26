#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::assign>& operands)
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

		instanciateAssignment(atomStack.back(), operands.lhs, operands.rhs, operands.disposelhs);
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
