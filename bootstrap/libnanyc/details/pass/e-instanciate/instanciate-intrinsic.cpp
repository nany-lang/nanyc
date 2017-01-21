#include "instanciate.h"
#include "details/intrinsic/catalog.h"
#include "instanciate-error.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


namespace {


bool translateIntrinsic(SequenceBuilder& seq, const ir::isa::Operand<ir::isa::Op::intrinsic>& operands) {
	AnyString name = seq.currentSequence.stringrefs[operands.intrinsic];
	if (unlikely(name.empty()))
		return (error() << "invalid empty intrinsic name");
	// named parameters are not accepted
	if (unlikely(not seq.pushedparams.func.named.empty()))
		return seq.complainIntrinsicWithNamedParameters(name);
	// generic type parameters are not accepted
	if (unlikely(not seq.pushedparams.gentypes.indexed.empty() or not seq.pushedparams.gentypes.named.empty()))
		return seq.complainIntrinsicWithGenTypeParameters(name);
	// checking if one parameter was already flag as 'error'
	auto& frame = *seq.frame;
	for (uint32_t i = 0u; i != seq.pushedparams.func.indexed.size(); ++i) {
		if (unlikely(not frame.verify(seq.pushedparams.func.indexed[i].lvid)))
			return false;
	}
	// trying user-defined intrinsic
	auto* intrinsic = seq.intrinsics.find(name);
	// if not found, this could be a compiler intrinsic
	if (!intrinsic) {
		switch (seq.instanciateBuiltinIntrinsic(name, operands.lvid, false)) {
			case Tribool::Value::indeterminate:
				break; // not found
			case Tribool::Value::yes:
				return true;
			case Tribool::Value::no:
				return false; // an error has occured
		}
		// intrinsic not found, trying discover mode
		if (seq.build.cf.on_binding_discovery) {
			auto retry = seq.build.cf.on_binding_discovery(seq.build.self(), name.c_str(), name.size());
			if (retry == nytrue)
				intrinsic = seq.intrinsics.find(name);
		}
		if (unlikely(!intrinsic))
			return complain::unknownIntrinsic(name);
	}
	if (unlikely(not seq.checkForIntrinsicParamCount(name, intrinsic->paramcount)))
		return false;
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


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::intrinsic>& operands) {
	assert(frame != nullptr);
	if (unlikely(not translateIntrinsic(*this, operands))) {
		frame->invalidate(operands.lvid);
		success = false;
	}
	// always remove pushed parameters, whatever the result
	pushedparams.clear();
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
