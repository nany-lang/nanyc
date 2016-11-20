#include "instanciate.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::stacksize>& operands)
	{
		if (frame)
		{
			// the new stack size
			uint32_t stacksize = operands.add + frame->atom.opcodes.stackSizeExtra;

			//if (cdeftable.substituteAtomID() == frame->atomid)
			frame->resizeRegisterCount(stacksize, cdeftable);

			if (canGenerateCode())
				frame->offsetOpcodeStacksize = out->emitStackSizeIncrease(stacksize);
		}
	}




} // namespace Instanciate
} // namespace Pass
} // namespace ny
