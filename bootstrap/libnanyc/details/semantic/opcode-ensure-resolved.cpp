#include "semantic-analysis.h"
#include "deprecated-error.h"

using namespace Yuni;

namespace ny::semantic {

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::ensureresolved>& operands) {
	bool ok = [&]() -> bool {
		assert(frame != nullptr);
		if (not frame->verify(operands.lvid))
			return false;

		CLID clid{frame->atomid, operands.lvid};
		if (unlikely(0 != frame->partiallyResolved.count(clid))) {
			// auto& resolveList = frame->resolvePerCLID[clid];
			return complain::multipleOverloads(operands.lvid);
		}

		auto& cdef = cdeftable.classdef(clid);
		if (cdef.isBuiltinOrVoid())
			return true;

		auto* atomptr = cdeftable.findClassdefAtom(cdef);
		if (unlikely(atomptr == nullptr)) {
			ice() << "invalid pseudo type 'any' for %" << operands.lvid << " (in " << clid << ')';
			return false;
		}

		auto& atom = *atomptr;
		switch (atom.type) {
			case Atom::Type::classdef: {
				bool requiresInstanciation =
					// instanciating the type itself, to resolve member variables
					(not atom.classinfo.isInstanciated)
					// in 'signature mode' only, the types are needed but no instanciation
					and (not signatureOnly);
				if (requiresInstanciation) {
					Atom* newAtomRef = instanciateAtomClass(atom);
					if (unlikely(nullptr == newAtomRef))
						return false;
					// The target atom may have changed for generic classes
					if (atom.atomid != newAtomRef->atomid) {
						auto& spare = cdeftable.substitute(operands.lvid);
						spare.mutateToAtom(newAtomRef);
					}
				}
				// remove generic type parameters if any (cleanup to avoid invalid state)
				pushedparams.gentypes.clear();
				break;
			}
			case Atom::Type::funcdef: {
				if (atom.isProperty()) {
					// for setter, everything will be entirely resolved at the next
					// assignment (or operators like +=)
					break;
				}
				return (error() << "pointer-to-function are not implemented yet");
			}
			// [[fallthu]]
			default: {
				// no generic type parameters should remain at this point
				if (unlikely(not pushedparams.gentypes.empty()))
					ice() << "invalid pushed generic type parameters, ensureResolve '" << atom.caption() << '\'';
				break;
			}
		}
		return true;
	}();
	if (unlikely(not ok))
		frame->invalidate(operands.lvid);
	pushedparams.clear();
}

} // ny::semantic
