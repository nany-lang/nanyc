#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::self>& operands)
	{
		// we can have at least 2 patterns:
		//
		//  * the most frequent, called from a method contained within a class
		//  * from the class itself, most likely a variable
		Atom* parent = &atomStack.back().atom;
		do
		{
			if (parent->isClass())
			{
				auto& cdef = cdeftable.substitute(operands.self);
				cdef.mutateToAtom(parent);
				cdef.qualifiers.ref = true;
				return;
			}
			parent = parent->parent;
		}
		while (parent != nullptr);

		// not found
		error() << "invalid param atom for 'self' resolution";
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
