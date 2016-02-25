#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::tryUnrefObject(uint32_t lvid)
	{
		if (not frame->verify(lvid))
			return;

		auto& cdef  = cdeftable.classdefFollowClassMember(CLID{frame->atomid, lvid});
		if (not canBeAcquired(cdef)) // do nothing if builtin
			return;

		if (unlikely(frame->lvids[lvid].synthetic))
		{
			error() << "cannot unref a synthetic object";
			return;
		}

		auto* atom = cdeftable.findClassdefAtom(cdef);
		if (unlikely(nullptr == atom))
		{
			ICE() << "invalid atom for 'unref' opcode";
			return frame->invalidate(lvid);
		}

		if (0 == atom->classinfo.dtor.atomid)
		{
			if (unlikely(not instanciateAtomClassDestructor(*atom, lvid)))
				return frame->invalidate(lvid);
			assert(atom->classinfo.dtor.atomid != 0);
		}

		if (canGenerateCode())
			out.emitUnref(lvid, atom->classinfo.dtor.atomid, atom->classinfo.dtor.instanceid);
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::ref>& operands)
	{
		if (not frame->verify(operands.lvid))
			return;

		if (unlikely(frame->lvids[operands.lvid].synthetic))
		{
			error() << "cannot acquire a synthetic object";
			return;
		}

		if (canGenerateCode())
		{
			if (canBeAcquired(operands.lvid))
				out.emitRef(operands.lvid); // manual var acquisition
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
