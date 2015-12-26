#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::stacksize>& operands)
	{
		if (atomStack.empty())
			return;

		auto& frame = atomStack.back();
		if (cdeftable.substituteAtomID() == frame.atomid)
			frame.resizeRegisterCount(operands.add, cdeftable);

		if (canGenerateCode())
			lastOpcodeStacksizeOffset = out.emitStackSizeIncrease(operands.add);
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
