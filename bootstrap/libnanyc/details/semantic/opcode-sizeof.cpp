#include "semantic-analysis.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::classdefsizeof>& operands) {
	if (canGenerateCode()) {
		if (not frame->verify(operands.type))
			return frame->invalidate(operands.lvid);
		auto& cdef = cdeftable.classdefFollowClassMember(CLID{frame->atomid, operands.type});
		if (not cdef.isBuiltinOrVoid()) {
			auto* atom = cdeftable.findClassdefAtom(cdef);
			if (unlikely(nullptr == atom)) {
				ice() << "invalid atom for sizeof operator";
				return;
			}
			ir::emit::type::objectSizeof(out, operands.lvid, atom->atomid);
		}
		else {
			uint64_t size = nytype_sizeof(cdef.kind);
			ir::emit::constantu64(out, operands.lvid, size);
		}
	}
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
