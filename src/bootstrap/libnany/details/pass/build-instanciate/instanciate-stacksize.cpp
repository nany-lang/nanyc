#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::stacksize>& operands)
	{
		if (frame)
		{
			if (cdeftable.substituteAtomID() == frame->atomid)
				frame->resizeRegisterCount(operands.add, cdeftable);

			if (canGenerateCode())
				frame->offsetOpcodeStacksize = out.emitStackSizeIncrease(operands.add);
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
