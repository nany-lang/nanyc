#include "details/context/build.h"
#include "instanciate.h"
#include "details/reporting/report.h"
#include "details/reporting/message.h"
#include "details/utils/origin.h"
#include "libnanyc-traces.h"
#include "instanciate-atom.h"
#include "instanciate-debug.h"
#include "func-overload-match.h"
#include "details/ir/emit.h"
#include <memory>

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


Logs::Report emitReportEntry(void* self, Logs::Level);
void retriveReportMetadata(void* self, Logs::Level, const AST::Node*, Yuni::String&, uint32_t&, uint32_t&);


SequenceBuilder::SequenceBuilder(Logs::Report report, ClassdefTableView& cdeftable, Build& build,
								 ir::Sequence* out, ir::Sequence& sequence, SequenceBuilder* parent)
	: cdeftable(cdeftable)
	, out(out)
	, currentSequence(sequence)
	, build(build)
	, intrinsics(build.intrinsics)
	, overloadMatch(this)
	, parent(parent)
	, localErrorHandler(this, &emitReportEntry)
	, localMetadataHandler(this, &retriveReportMetadata)
	, report(report) {
	// reduce memory (re)allocations
	multipleResults.reserve(8); // arbitrary value
	pushedparams.func.indexed.reserve(16);
	pushedparams.func.named.reserve(16);
	pushedparams.gentypes.indexed.reserve(8);
	pushedparams.gentypes.named.reserve(8);
}


SequenceBuilder::~SequenceBuilder() {
	if (Config::Traces::allTypeDefinitions) {
		for (auto* f = frame; f != nullptr; f = f->previous)
			debugPrintClassdefs(*f, cdeftable);
	}
	auto* frm = frame;
	while (frm) {
		auto* previous = frm->previous;
		build.deallocate(frm);
		frm = previous;
	}
}


void SequenceBuilder::releaseScopedVariables(int scope, bool forget) {
	if (unlikely(!frame))
		return;
	if (not forget and (not canGenerateCode()))
		return;
	// unref in the reverse order
	auto i = frame->localVariablesCount();
	if (canGenerateCode()) {
		while (i-- != 0) {
			auto& clcvr = frame->lvids(i);
			if (not (clcvr.scope >= scope))
				continue;
			if (clcvr.autorelease) {
				//if (not clcvr.userDefinedName.empty())
				//  ir::emit::trace(out, [&](){return String{"unref var "} << clcvr.userDefinedName << " -> %" << i;});
				tryUnrefObject(i);
			}
			// forget this variable!
			if (forget) {
				if (not clcvr.userDefinedName.empty() and clcvr.warning.unused) {
					if (unlikely(not clcvr.hasBeenUsed) and (clcvr.userDefinedName != "self"))
						complainUnusedVariable(*frame, i);
				}
				clcvr.userDefinedName.clear();
				clcvr.scope = -1;
			}
		}
	}
	else {
		assert(forget == true);
		while (i-- != 0) { // just invalidate everything
			auto& clcvr = frame->lvids(i);
			if (clcvr.scope >= scope) {
				clcvr.userDefinedName.clear();
				clcvr.scope = -1;
			}
		}
	}
}


uint32_t SequenceBuilder::createLocalVariables(uint32_t count) {
	assert(count > 0);
	assert(canGenerateCode());
	assert(frame != nullptr);
	assert(frame->offsetOpcodeStacksize != (uint32_t) - 1);
	auto& operands = out->at<ir::ISA::Op::stacksize>(frame->offsetOpcodeStacksize);
	uint32_t startOffset = operands.add;
	int scope = frame->scope;
	operands.add += count;
	frame->resizeRegisterCount(operands.add, cdeftable);
	assert(startOffset + count <= frame->localVariablesCount());
	for (uint32_t i = 0; i != count; ++i) {
		uint32_t lvid = startOffset + i;
		cdeftable.substitute(lvid).mutateToAny();
		auto& details = frame->lvids(lvid);
		details.scope = scope;
		details.synthetic = false;
		details.offsetDeclOut = out->opcodeCount();
		ir::emit::alloc(out, startOffset + i);
	}
	return startOffset;
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::namealias>& operands) {
	const auto& name = currentSequence.stringrefs[operands.name];
	declareNamedVariable(name, operands.lvid);
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::debugfile>& operands) {
	currentFilename = currentSequence.stringrefs[operands.filename].c_str();
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::debugpos>& operands) {
	currentLine   = operands.line;
	currentOffset = operands.offset;
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::unref>& operands) {
	tryUnrefObject(operands.lvid);
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::nop>&) {
	// duplicate nop as well since they can be used to insert code
	// (for shortcircuit for example)
	if (canGenerateCode())
		ir::emit::nop(out);
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::label>& operands) {
	if (canGenerateCode())
		/*uint32_t lbl =*/ ir::emit::label(out, operands.label);
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::qualifiers>& operands) {
	assert(static_cast<uint32_t>(operands.qualifier) < ir::ISA::TypeQualifierCount);
	bool  onoff = (operands.flag != 0);
	auto& qualifiers = cdeftable.substitute(operands.lvid).qualifiers;
	switch (operands.qualifier) {
		case ir::ISA::TypeQualifier::ref:
			qualifiers.ref = onoff;
			break;
		case ir::ISA::TypeQualifier::constant:
			qualifiers.constant = onoff;
			break;
	}
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::jmp>& opc) {
	if (canGenerateCode())
		out->emit<ir::ISA::Op::jmp>() = opc;
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::jz>& opc) {
	if (canGenerateCode())
		out->emit<ir::ISA::Op::jz>() = opc;
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::jnz>& opc) {
	if (canGenerateCode())
		out->emit<ir::ISA::Op::jnz>() = opc;
}


inline void SequenceBuilder::visit(const ir::ISA::Operand<ir::ISA::Op::comment>& opc) {
	if (debugmode and canGenerateCode())
		ir::emit::trace(out, [&]() {
		return currentSequence.stringrefs[opc.text];
	});
}


template<ir::ISA::Op O>
void SequenceBuilder::visit(const ir::ISA::Operand<O>& operands) {
	complainOperand(ir::Instruction::fromOpcode(operands));
}


bool SequenceBuilder::readAndInstanciate(uint32_t offset) {
	#if LIBNANYC_IR_PRINT_OPCODES != 0
	std::cout << "\n\n -- sequence builder read start from " << (void*) this << "\n";
	#endif
	currentSequence.each(*this, offset);
	#if LIBNANYC_IR_PRINT_OPCODES != 0
	std::cout << " -- END " << (void*) this << std::endl;
	#endif
	return success;
}


void SequenceBuilder::PushedParameters::clear() {
	func.indexed.clear();
	func.named.clear();
	gentypes.indexed.clear();
	gentypes.named.clear();
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny


namespace ny {


bool Build::resolveStrictParameterTypes(Atom& atom) {
	return Pass::Instanciate::resolveStrictParameterTypes(*this, atom);
}


bool Build::instanciate(const AnyString& entrypoint, const nytype_t* args, uint32_t& atomid,
		uint32_t& instanceid) {
	ny::Logs::Report report{*messages.get()};
	if (unlikely(args)) {
		report.error() << "arguments for atom instanciation is not supported yet";
		return false;
	}
	MutexLocker locker{mutex};
	Atom* entrypointAtom = nullptr;
	try {
		cdeftable.atoms.root.eachChild(entrypoint, [&](Atom & child) -> bool {
			if (unlikely(entrypointAtom != nullptr))
				throw std::runtime_error("': multiple entry points found");
			entrypointAtom = &child;
			return true;
		});
		if (unlikely(!entrypointAtom))
			throw std::runtime_error("()': function not found");
		if (unlikely(not entrypointAtom->isFunction() or entrypointAtom->isClassMember()))
			throw std::runtime_error("': the atom is not a function");
	}
	catch (const std::exception& e) {
		report.error() << "failed to instanciate '" << entrypoint << e.what();
		return false;
	}
	decltype(Pass::Instanciate::FuncOverloadMatch::result.params) params;
	decltype(Pass::Instanciate::FuncOverloadMatch::result.params) tmplparams;
	Logs::Message::Ptr newReport;
	ClassdefTableView cdeftblView{cdeftable};
	Pass::Instanciate::InstanciateData info {
		newReport, *entrypointAtom, cdeftblView, *this, params, tmplparams
	};
	bool instanciated = Pass::Instanciate::instanciateAtom(info);
	report.appendEntry(newReport);
	if (Config::Traces::atomTable)
		cdeftable.atoms.root.printTree(cdeftable);
	if (not instanciated) {
		atomid = (uint32_t) - 1;
		instanceid = (uint32_t) - 1;
		return false;
	}
	atomid = entrypointAtom->atomid;
	instanceid = info.instanceid;
	return true;
}


} // namespace ny
