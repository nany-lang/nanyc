#include "instanciate.h"
#include "instanciate-error.h"

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{

	void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::allocate>& operands)
	{
		if (not frame->verify(operands.atomid))
			return frame->invalidate(operands.lvid);

		// find the type of the object to allocate
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, operands.atomid});
		Atom* atom = (not cdef.isBuiltinOrVoid()) ? cdeftable.findClassdefAtom(cdef) : nullptr;
		if (unlikely(atom == nullptr))
			return (void) complain::canNotAllocateClassNullAtom(cdef, operands.lvid);

		if (unlikely(not atom->isClass()))
			return (void) complain::classRequired();

		if (canGenerateCode()) // checking for real object only when they exist
		{
			if (unlikely(not atom->classinfo.isInstanciated and canGenerateCode()))
				return (void) complain::classNotInstanciated(*atom);
		}

		// propagate the object type
		{
			auto& spare = cdeftable.substitute(operands.lvid);
			spare.qualifiers.ref = true;
			spare.mutateToAtom(atom);
		}

		// remember that the value stored into the register comes from a memory allocation
		// (can be used to avoid spurious object copies for example)
		frame->lvids(operands.lvid).origin.memalloc = true;
		// not a synthetic object
		frame->lvids(operands.lvid).synthetic = false;

		if (canGenerateCode())
		{
			// trick: when generating the opcode, a register has already been allocated
			// for storing the size of the object
			out->emitSizeof(operands.lvid - 1, atom->atomid);
			ir::emit::memory::allocate(out, operands.lvid, operands.lvid - 1);
			acquireObject(operands.lvid);
		}
	}




} // namespace Instanciate
} // namespace Pass
} // namespace ny
