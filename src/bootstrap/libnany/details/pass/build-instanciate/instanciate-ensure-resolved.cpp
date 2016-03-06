#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	inline bool SequenceBuilder::ensureResolve(const IR::ISA::Operand<IR::ISA::Op::ensureresolved>& operands)
	{
		assert(frame != nullptr);
		if (not frame->verify(operands.lvid))
			return false;

		CLID clid{frame->atomid, operands.lvid};
		if (unlikely(not frame->resolvePerCLID[clid].empty()))
		{
			// auto& resolveList = frame->resolvePerCLID[clid];
			return complainMultipleOverloads(operands.lvid);
		}

		auto& cdef = cdeftable.classdef(clid);
		if (cdef.isBuiltinOrVoid())
			return true;

		auto* atomptr = cdeftable.findClassdefAtom(cdef);
		if (unlikely(atomptr == nullptr))
			return (ICE() << "invalid 'any' type for " << clid);
		auto& atom = *atomptr;

		switch (atom.type)
		{
			case Atom::Type::classdef:
			{
				// instanciating the type itself, to resolve member variables
				if (not atom.classinfo.isInstanciated)
				{
					Atom* newAtomRef = instanciateAtomClass(atom);
					if (unlikely(nullptr == newAtomRef))
						return false;

					// The target atom may have changed for generic classes
					if (atom.atomid != newAtomRef->atomid)
					{
						auto& spare = cdeftable.substitute(operands.lvid);
						spare.mutateToAtom(newAtomRef);
					}
				}
				break;
			}
			case Atom::Type::funcdef:
			{
				error() << "pointer-to-function are not implemented yet";
				return false;
			}
			default:
				break;
		}
		return true;
	}


	void SequenceBuilder::visit(const IR::ISA::Operand<IR::ISA::Op::ensureresolved>& operands)
	{
		bool ok = ensureResolve(operands);
		if (unlikely(not ok))
			frame->invalidate(operands.lvid);

		pushedparams.clear();
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
