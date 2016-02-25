#include "instanciate.h"
#include "libnany-traces.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::scope>& /*operands*/)
	{
		if (frame != nullptr)
			++(frame->scope);

		if (canGenerateCode())
			out.emitScope();
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::end>& /*operands*/)
	{
		if (frame != nullptr)
		{
			releaseScopedVariables(frame->scope, /*forget*/ true);

			// the scope might be zero if the opcode 'end' comes from a class or a func
			if (frame->scope > 0)
			{
				if (canGenerateCode())
					out.emitEnd();
				--frame->scope;
			}
			else
			{
				if (frame->previous != nullptr)
				{
					if (Config::Traces::printClassdefTable)
						printClassdefTable(report.subgroup(), *frame);

					// destroy the frame
					auto* previous = frame->previous;
					frame->~AtomStackFrame();
					context.memory.release(&context, frame, sizeof(AtomStackFrame));
					frame = previous;

					++layerDepthLimit;
					if (canGenerateCode())
						out.emitEnd();
				}
				else
					*cursor += currentSequence.opcodeCount(); // end of the code
			}
		}
		else
		{
			assert(false and "should not happen");
			*cursor += currentSequence.opcodeCount();
		}
	}







} // namespace Instanciate
} // namespace Pass
} // namespace Nany
