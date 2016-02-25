#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::ensureresolved>& operands)
	{
		assert(frame != nullptr);
		if (not frame->verify(operands.lvid))
			return;

		CLID clid{frame->atomid, operands.lvid};
		if (unlikely(not frame->resolvePerCLID[clid].empty()))
		{
			complainMultipleOverloads(operands.lvid);
			return frame->invalidate(operands.lvid);
		}

		auto& cdef = cdeftable.classdef(clid);
		if (cdef.isBuiltinOrVoid())
			return;

		auto* atom = cdeftable.findClassdefAtom(cdef);
		if (unlikely(atom == nullptr))
		{
			ICE() << "invalid 'any' type for " << clid;
			return frame->invalidate(operands.lvid);
		}

		switch (atom->type)
		{
			case Atom::Type::classdef:
			{
				// TODO
				break;
			}
			case Atom::Type::funcdef:
			{
				error() << "pointer-to-function are not implemented yet";
				return frame->invalidate(operands.lvid);
				break;
			}
			default:
				break;
		}
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
