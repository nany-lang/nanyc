#include "semantic-analysis.h"
#include "details/intrinsic/catalog.h"
#include "deprecated-error.h"
#include "intrinsics.h"

using namespace Yuni;

namespace ny::semantic {

namespace {

bool translateIntrinsic(Analyzer& seq, const ir::isa::Operand<ir::isa::Op::intrinsic>& operands) {
	AnyString name = seq.currentSequence.stringrefs[operands.intrinsic];
	if (unlikely(name.empty()))
		return (error() << "invalid empty intrinsic name");
	switch (intrinsic::langOrNanycSpecifics(seq, name, operands.lvid, false)) {
		case Tribool::Value::yes:
			return true;
		case Tribool::Value::indeterminate:
			break; // not found
		case Tribool::Value::no:
			return false; // an error has occured
	}
	// trying user-defined intrinsic
	auto* intrinsic = seq.intrinsics.find(name);
	if (!intrinsic)
		return complain::unknownIntrinsic(name);
	if (unlikely(not seq.checkForIntrinsicParamCount(name, intrinsic->paramcount)))
		return false;
	auto& frame = *seq.frame;
	uint32_t count = static_cast<uint32_t>(seq.pushedparams.func.indexed.size());
	frame.lvids(operands.lvid).synthetic = false;
	// reset the returned type
	seq.cdeftable.substitute(operands.lvid).kind = intrinsic->rettype;
	// intrinsic parameters
	bool success = true;
	auto& out = seq.out;
	for (uint32_t i = 0; i != count; ++i) {
		auto& element = seq.pushedparams.func.indexed[i];
		if (not frame.verify(element.lvid)) // silently ignore error
			return false;
		auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{frame.atomid, element.lvid});
		if (unlikely(not cdef.isBuiltin())) {
			success = false;
			seq.complainIntrinsicParameter(name, i, cdef, "a builtin type");
		}
		else {
			if (seq.canGenerateCode())
				ir::emit::push(out, element.lvid);
		}
	}
	if (seq.canGenerateCode())
		ir::emit::intrinsic(out, operands.lvid, nullptr, intrinsic->id);
	return success;
}

} // namespace

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::intrinsic>& operands) {
	assert(frame != nullptr);
	if (unlikely(not translateIntrinsic(*this, operands))) {
		frame->invalidate(operands.lvid);
		success = false;
	}
	// always remove pushed parameters, whatever the result
	pushedparams.clear();
}

} // ny::semantic
