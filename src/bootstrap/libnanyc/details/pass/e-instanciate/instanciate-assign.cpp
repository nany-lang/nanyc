#include "instanciate.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::assign>& operands)
	{
		assert(frame != nullptr);
		frame->lvids(operands.lhs).synthetic = false;

		if (not frame->verify(operands.rhs))
			return frame->invalidate(operands.lhs);

		auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lhs});
		if (cdef.isAny())
		{
			// type propagation
			auto& cdefsrc = cdeftable.classdef(CLID{frame->atomid, operands.rhs});
			auto& spare = cdeftable.substitute(cdef.clid.lvid());
			spare.import(cdefsrc);
		}

		bool canDisposeLHS = (operands.disposelhs != 0);
		bool r = instanciateAssignment(*frame, operands.lhs, operands.rhs, canDisposeLHS);
		if (unlikely(not r))
			frame->invalidate(operands.lhs);
	}




} // namespace Instanciate
} // namespace Pass
} // namespace ny
