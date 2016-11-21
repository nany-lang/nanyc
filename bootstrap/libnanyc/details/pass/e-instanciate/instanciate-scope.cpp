#include "instanciate.h"
#include "instanciate-debug.h"
#include "libnanyc-traces.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::scope>& /*operands*/)
	{
		if (frame != nullptr)
			++(frame->scope);

		if (canGenerateCode())
			ir::emit::scopeBegin(out);
	}


	void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::end>& /*operands*/)
	{
		if (frame != nullptr)
		{
			releaseScopedVariables(frame->scope, /*forget*/ true);

			// the scope might be zero if the opcode 'end' comes from a class or a func
			if (frame->scope > 0)
			{
				if (canGenerateCode())
					ir::emit::scopeEnd(out);
				--frame->scope;
			}
			else
			{
				if (frame->previous != nullptr)
				{
					if (Config::Traces::classdefTable)
						debugPrintClassdefs(*frame, cdeftable);

					popFrame();

					++layerDepthLimit;
					if (canGenerateCode())
						ir::emit::scopeEnd(out);
				}
				else
					currentSequence.invalidateCursor(*cursor); // end-of-code
			}
		}
		else
		{
			assert(false and "no frame for blueprint end - should not happen");
			currentSequence.invalidateCursor(*cursor);
		}
	}







} // namespace Instanciate
} // namespace Pass
} // namespace ny
