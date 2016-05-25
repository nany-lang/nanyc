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
			// the new stack size
			uint32_t stacksize = operands.add + frame->atom.opcodes.stackSizeExtra;

			if (cdeftable.substituteAtomID() == frame->atomid)
				frame->resizeRegisterCount(stacksize, cdeftable);

			if (canGenerateCode())
				frame->offsetOpcodeStacksize = out.emitStackSizeIncrease(stacksize);
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
