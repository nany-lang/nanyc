#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::allocate>& operands)
	{
		if (not frame->verify(operands.atomid))
			return;

		// find the type of the object to allocate
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, operands.atomid});
		Atom* atom = (not cdef.isBuiltinOrVoid()) ? cdeftable.findClassdefAtom(cdef) : nullptr;
		if (unlikely(atom == nullptr))
		{
			auto err = (error() << "cannot instanciate object of type '");
			cdef.print(err.data().message, cdeftable, false);
			err << "'";
			frame->invalidate(operands.lvid);
			return;
		}

		// propagate the object type
		{
			auto& spare = cdeftable.substitute(operands.lvid);
			spare.qualifiers.ref = true;
			spare.mutateToAtom(atom);
		}

		// remember that the value stored into the register comes from a memory allocation
		// (can be used to avoid spurious object copies for example)
		frame->lvids[operands.lvid].origin.memalloc = true;
		// not a synthetic object
		frame->lvids[operands.lvid].synthetic = false;

		if (canGenerateCode())
		{
			// trick: when generating the opcode, a register has already been allocated
			// for storing the size of the object
			out.emitSizeof(operands.lvid - 1, atom->atomid);
			out.emitMemalloc(operands.lvid, operands.lvid - 1);
			acquireObject(operands.lvid);
		}
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
