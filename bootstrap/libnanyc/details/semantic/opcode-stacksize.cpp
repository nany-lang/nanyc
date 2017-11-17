#include "semantic-analysis.h"

using namespace Yuni;

namespace ny::semantic {

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::stacksize>& operands) {
	if (frame) {
		// the new stack size
		uint32_t stacksize = operands.add + frame->atom.opcodes.stackSizeExtra;
		//if (cdeftable.substituteAtomID() == frame->atomid)
		frame->resizeRegisterCount(stacksize, cdeftable);
		if (canGenerateCode())
			frame->offsetOpcodeStacksize = ir::emit::increaseStacksize(out, stacksize);
	}
}

} // ny::semantic
