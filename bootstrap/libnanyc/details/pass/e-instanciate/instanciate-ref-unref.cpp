#include "instanciate.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


void SequenceBuilder::tryUnrefObject(uint32_t lvid) {
	if (not frame->verify(lvid))
		return;
	if (unlikely(frame->lvids(lvid).synthetic)) {
		auto err = (error() << "cannot unref a synthetic object");
		if (debugmode)
			err << " {" << frame->atomid << ',' << lvid << '}';
		return;
	}
	auto& cdef  = cdeftable.classdefFollowClassMember(CLID{frame->atomid, lvid});
	if (not canBeAcquired(cdef)) // do nothing if builtin
		return;
	auto* atom = cdeftable.findClassdefAtom(cdef);
	if (unlikely(nullptr == atom)) {
		ice() << "invalid atom for 'unref' opcode";
		return frame->invalidate(lvid);
	}
	if (0 == atom->classinfo.dtor.atomid) {
		if (unlikely(not instanciateAtomClassDestructor(*atom, lvid)))
			return frame->invalidate(lvid);
		assert(atom->classinfo.dtor.atomid != 0);
	}
	if (canGenerateCode())
		ir::emit::unref(out, lvid, atom->classinfo.dtor.atomid, atom->classinfo.dtor.instanceid);
}


void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::ref>& operands) {
	if (not frame->verify(operands.lvid))
		return;
	if (unlikely(frame->lvids(operands.lvid).synthetic)) {
		error() << "cannot acquire a synthetic object";
		return;
	}
	if (canGenerateCode()) {
		if (canBeAcquired(operands.lvid))
			ir::emit::ref(out, operands.lvid); // manual var acquisition
	}
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
