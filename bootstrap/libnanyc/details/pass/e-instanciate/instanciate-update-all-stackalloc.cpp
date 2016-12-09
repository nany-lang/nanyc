#include "instanciate.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


namespace {


struct PostProcessStackAllocWalker final {
	PostProcessStackAllocWalker(ClassdefTableView& table, uint32_t atomid)
		: table(table)
		, atomid(atomid) {
	}

	void visit(ir::isa::Operand<ir::isa::Op::stackalloc>& opc) {
		if (debugmode) {
			if (not table.hasClassdef(CLID{atomid, opc.lvid})) {
				ice() << "failed to get classdef " << CLID{atomid, opc.lvid};
				return;
			}
		}
		auto& cdef = table.classdef(CLID{atomid, opc.lvid});
		if (not cdef.isBuiltinOrVoid()) {
			auto* atom = table.findClassdefAtom(cdef);
			if (atom != nullptr) {
				assert(opc.atomid == 0 or opc.atomid == (uint32_t) - 1 or opc.atomid == atom->atomid);
				opc.type = static_cast<uint32_t>(nyt_ptr);
				opc.atomid = atom->atomid;
			}
		}
		else
			opc.type = static_cast<uint32_t>(cdef.kind);
	}

	template<ir::isa::Op O> void visit(const ir::isa::Operand<O>&) {}

	ClassdefTableView& table;
	uint32_t atomid;
	ir::Instruction** cursor = nullptr;
};


} // anonymous namespace


void updateTypesInAllStackallocOp(ir::Sequence& out, ClassdefTableView& table, uint32_t atomid) {
	PostProcessStackAllocWalker walker{table, atomid};
	out.each(walker);
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
