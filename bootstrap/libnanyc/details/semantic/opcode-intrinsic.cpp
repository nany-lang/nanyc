#include "semantic-analysis.h"
#include "details/intrinsic/catalog.h"
#include "deprecated-error.h"

using namespace Yuni;


namespace ny {
namespace semantic {


namespace {


bool verifyParameters(Analyzer& analyzer, const AnyString& name) {
	auto& pushedparams = analyzer.pushedparams;
	// named parameters are not accepted
	if (unlikely(not pushedparams.func.named.empty()))
		return analyzer.complainIntrinsicWithNamedParameters(name);
	// generic type parameters are not accepted
	if (unlikely(not pushedparams.gentypes.indexed.empty() or not pushedparams.gentypes.named.empty()))
		return analyzer.complainIntrinsicWithGenTypeParameters(name);
	// checking if one parameter was already flag as 'error'
	auto& frame = *analyzer.frame;
	uint32_t count = static_cast<uint32_t>(pushedparams.func.indexed.size());
	for (uint32_t i = 0u; i != count; ++i) {
		if (unlikely(not frame.verify(pushedparams.func.indexed[i].lvid)))
			return false;
	}
	return true;
}


bool translateIntrinsic(Analyzer& seq, const ir::isa::Operand<ir::isa::Op::intrinsic>& operands) {
	AnyString name = seq.currentSequence.stringrefs[operands.intrinsic];
	if (unlikely(name.empty()))
		return (error() << "invalid empty intrinsic name");
	if (unlikely(not verifyParameters(seq, name)))
		return false;
	// official nanyc lang intrinsics
	switch (seq.instanciateBuiltinIntrinsic(name, operands.lvid, false)) {
		case Tribool::Value::yes:
			return true;
		case Tribool::Value::indeterminate:
			break; // not found
		case Tribool::Value::no:
			return false; // an error has occured
	}
	// trying user-defined intrinsic
	auto* intrinsic = seq.intrinsics.find(name);
	if (!intrinsic) {
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


} // namespace semantic
} // namespace ny
