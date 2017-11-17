#include "semantic-analysis.h"
#include "deprecated-error.h"

using namespace Yuni;

namespace ny::semantic {

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::typeisobject>& operands) {
	assert(frame != nullptr);
	bool ok = [&]() -> bool {
		if (not frame->verify(operands.lvid))
			return false;
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, operands.lvid});
		if (likely(not cdef.isBuiltinOrVoid())) {
			auto* atom = cdeftable.findClassdefAtom(cdef);
			if (likely(nullptr != atom)) {
				if (unlikely(atom->isClass() or atom->isFunction())) {
					if (canGenerateCode()) { // checking for real object only when they exist
						if (unlikely(atom->isClass() and not atom->classinfo.isInstanciated))
							return complain::classNotInstanciated(*atom);
					}
					return true;
				}
			}
		}
		return complain::classOrFuncExpected(cdef);
	}();
	if (unlikely(not ok))
		frame->invalidate(operands.lvid);
}

} // ny::semantic
