#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::scope>& /*operands*/)
	{
		if (likely(not atomStack.empty()))
			++(atomStack.back().scope);

		if (canGenerateCode())
			out.emitScope();
	}


	void ProgramBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::end>& /*operands*/)
	{
		if (likely(not atomStack.empty()))
		{
			auto& frame = atomStack.back();
			releaseScopedVariables(frame.scope, /*forget*/ true);

			// the scope might be zero if the opcode 'end' comes from a class or a func
			if (frame.scope > 0)
			{
				if (canGenerateCode())
					out.emitEnd();
				--frame.scope;
			}
			else
			{
				if (atomStack.size() > 1)
				{
					if (Config::Traces::printClassdefTable)
						printClassdefTable(report.subgroup(), atomStack.back());

					atomStack.pop_back(); // remove a part of the stack
					++layerDepthLimit;
					if (canGenerateCode())
						out.emitEnd();
				}
				else
					*cursor += currentProgram.opcodeCount(); // end of the code
			}
		}
		else
		{
			assert(false and "should not happen");
			*cursor += currentProgram.opcodeCount();
		}
	}







} // namespace Instanciate
} // namespace Pass
} // namespace Nany
