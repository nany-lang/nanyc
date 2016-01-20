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
		auto& frame = atomStack.back();
		if (not frame.verify(lvid))
			return;

		auto& cdef  = cdeftable.classdefFollowClassMember(CLID{frame.atomid, lvid});
		if (not canBeAcquired(cdef)) // do nothing if builtin
			return;


		auto* atom = cdeftable.findClassdefAtom(cdef);
		if (unlikely(nullptr == atom))
		{
			ICE() << "invalid atom for 'unref' opcode";
			return;
		}

		if (0 == atom->classinfo.dtor.atomid)
		{
			if (unlikely(not instanciateAtomClassDestructor(*atom, lvid)))
				return;
			assert(atom->classinfo.dtor.atomid != 0);
		}

		if (canGenerateCode())
			out.emitUnref(lvid, atom->classinfo.dtor.atomid, atom->classinfo.dtor.instanceid);
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::ref>& operands)
	{
		if (not atomStack.back().verify(operands.lvid))
			return;

		if (canGenerateCode())
		{
			if (canBeAcquired(operands.lvid))
				out.emitRef(operands.lvid); // manual var acquisition
		}
	}






} // namespace Instanciate
} // namespace Pass
} // namespace Nany
