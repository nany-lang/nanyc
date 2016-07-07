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
		if (unlikely(0 != frame->partiallyResolved.count(clid)))
		{
			// auto& resolveList = frame->resolvePerCLID[clid];
			return complainMultipleOverloads(operands.lvid);
		}

		auto& cdef = cdeftable.classdef(clid);
		if (cdef.isBuiltinOrVoid())
			return true;

		auto* atomptr = cdeftable.findClassdefAtom(cdef);
		if (unlikely(atomptr == nullptr))
		{
			ice() << "invalid pseudo type 'any' for %" << operands.lvid << " (in " << clid << ')';
			return false;
		}
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

					// remove generic type parameters if any
					pushedparams.gentypes.clear();

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
				if (atom.isProperty())
				{
					// for setter, everything will be entirely resolved at the next
					// assignment (or operators like +=)
				}
				else
				{
					error() << "pointer-to-function are not implemented yet";
					return false;
				}
			}
			default:
				break;
		}

		// no generic type parameters should remain at this point
		if (unlikely(not pushedparams.gentypes.empty()))
			ice() << "invalid pushed generic type parameters, ensureResolve '" << atom.caption() << '\'';

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
