#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::push>& operands)
	{
		if (0 == operands.name)
		{
			pushedparams.func.indexed.emplace_back(operands.lvid, currentLine, currentOffset);
		}
		else
		{
			const auto& name = currentSequence.stringrefs[operands.name];
			pushedparams.func.named.emplace_back(name, operands.lvid, currentLine, currentOffset);
		}
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::tpush>& operands)
	{
		if (0 == operands.name)
		{
			pushedparams.gentypes.indexed.emplace_back(operands.lvid, currentLine, currentOffset);
		}
		else
		{
			const auto& name = currentSequence.stringrefs[operands.name];
			pushedparams.gentypes.named.emplace_back(name, operands.lvid, currentLine, currentOffset);
		}
	}







} // namespace Instanciate
} // namespace Pass
} // namespace Nany
