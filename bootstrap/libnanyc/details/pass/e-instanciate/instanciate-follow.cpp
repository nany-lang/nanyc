#include "instanciate.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::follow>& operands) {
	// in 'signature only' mode (resolving defined parameter types), all
	// types must be gathered
	if (not operands.symlink or signatureOnly) {
		auto& cdef  = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
		auto& spare = cdeftable.substitute(operands.follower);
		spare.import(cdef);
		spare.qualifiers.merge(cdef.qualifiers);
		spare.instance = true;
	}
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
