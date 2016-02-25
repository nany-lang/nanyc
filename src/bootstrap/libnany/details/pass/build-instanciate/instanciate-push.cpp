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
		bool verified = (frame->verify(operands.lvid));

		// always push the parameter to have a consistent output
		if (0 == operands.name)
		{
			pushedparams.func.indexed.emplace_back(operands.lvid, currentLine, currentOffset);

			if (verified and unlikely(frame->lvids[operands.lvid].synthetic))
			{
				error() << "cannot push synthetic object for parameter "
					<< pushedparams.func.indexed.size();
			}
		}
		else
		{
			const auto& name = currentSequence.stringrefs[operands.name];
			pushedparams.func.named.emplace_back(name, operands.lvid, currentLine, currentOffset);

			if (verified and unlikely(frame->lvids[operands.lvid].synthetic))
				error() << "cannot push synthetic object for parameter '" << name << "'";
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
