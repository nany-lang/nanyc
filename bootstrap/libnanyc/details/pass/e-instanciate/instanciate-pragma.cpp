#include "instanciate.h"
#include "details/ir/emit.h"
#include "instanciate-error.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


namespace {


bool bodyStart(SequenceBuilder& seq) {
	assert(seq.frame != nullptr);
	assert(not seq.signatureOnly);
	assert(seq.codeGenerationLock == 0 and "any good reason to not generate code ?");
	assert(seq.out != nullptr and "no output IR sequence");
	auto& frame = *seq.frame;
	auto& atom = frame.atom;
	if (not atom.isFunction())
		return true;
	bool generateCode = seq.canGenerateCode();
	uint32_t count = atom.parameters.size();
	bool success = true;
	atom.parameters.each([&](uint32_t i, const AnyString& name, const Vardef & vardef) {
		uint32_t lvid  = i + 1 + 1; // 1: return type, 2: first parameter
		frame.lvids(lvid).synthetic = false;
		// Parameters Deep copy (if required)
		auto& cdef = seq.cdeftable.classdef(vardef.clid);
		if (name[0] != '^') {
			// normal input parameter (not captured - does not start with '^')
			// clone it if necessary (only non-ref parameters)
			if (not cdef.qualifiers.ref) {
				ir::emit::trace(seq.out, generateCode, [&]() {
					return String{"----- deep copy parameter "} << i << " aka " << name;
				});
				// a register has already been reserved for cloning parameters
				uint32_t clone = 2 + count + i; // 1: return type, 2: first parameter
				frame.lvids(clone).synthetic = false;
				bool r = seq.instanciateAssignment(frame, clone, lvid, false, false, true);
				if (unlikely(not r))
					frame.invalidate(clone);
				if (seq.canBeAcquired(lvid)) {
					frame.lvids(lvid).autorelease = true;
					frame.lvids(clone).autorelease = false;
				}
				if (generateCode) {
					ir::emit::copy(seq.out, lvid, clone); // register swap
					ir::emit::trace(seq.out, "--\n");
				}
			}
		}
		if (not cdef.isBuiltinOrVoid()) {
			if (unlikely(!seq.cdeftable.findClassdefAtom(cdef))) {
				frame.invalidate(lvid);
				success = complain::parameterTypeHasVanished(seq, i);
			}
		}
	});
	if (atom.isSpecial() and generateCode and success) {
		if (seq.generateClassVarsAutoInit) { // ctor
			seq.generateClassVarsAutoInit = false;
			seq.generateMemberVarDefaultInitialization();
		}
		if (seq.generateClassVarsAutoRelease) { // dtor
			seq.generateClassVarsAutoRelease = false;
			seq.generateMemberVarDefaultDispose();
		}
		if (seq.generateClassVarsAutoClone) { // deep copy (ctor)
			seq.generateClassVarsAutoClone = false;
			seq.generateMemberVarDefaultClone();
		}
	}
	return success;
}


void codegen(SequenceBuilder& seq, bool onoff) {
	auto& refcount = seq.codeGenerationLock;
	if (onoff) {
		if (refcount > 0) // re-enable code generation
			--refcount;
	}
	else
		++refcount;
}


void blueprintSize(SequenceBuilder& seq, uint32_t opcodeCount) {
	if (0 == seq.layerDepthLimit) {
		// ignore the current blueprint
		//*cursor += operands.value.blueprintsize;
		assert(seq.frame != nullptr);
		assert(seq.frame->offsetOpcodeBlueprint != (uint32_t) - 1);
		auto startOffset = seq.frame->offsetOpcodeBlueprint;
		if (unlikely(opcodeCount < 3))
			return (void) (ice() << "invalid blueprint size when instanciating atom");
		// goto the end of the blueprint
		// -2: the final 'end' opcode must be interpreted
		*seq.cursor = &seq.currentSequence.at(startOffset + opcodeCount - 1 - 1);
	}
}


void shortcircuitMutateToBool(SequenceBuilder& seq, const ir::isa::Operand<ir::isa::Op::pragma>& operands) {
	uint32_t lvid = operands.value.shortcircuitMutate.lvid;
	uint32_t source = operands.value.shortcircuitMutate.source;
	seq.frame->lvids(lvid).synthetic = false;
	if (true) {
		auto& instr = *(*seq.cursor - 1);
		assert(instr.opcodes[0] == static_cast<uint32_t>(ir::isa::Op::stackalloc));
		uint32_t sizeoflvid = instr.to<ir::isa::Op::stackalloc>().lvid;
		// sizeof
		auto& atombool = *(seq.cdeftable.atoms().core.object[nyt_bool]);
		ir::emit::type::objectSizeof(seq.out, sizeoflvid, atombool.atomid);
		auto& opc = seq.cdeftable.substitute(lvid);
		opc.mutateToAtom(&atombool);
		opc.qualifiers.ref = true;
		// ALLOC: memory allocation of the new temporary object
		ir::emit::memory::allocate(seq.out, lvid, sizeoflvid);
		ir::emit::ref(seq.out, lvid);
		seq.frame->lvids(lvid).autorelease = true;
		// reset the internal value of the object
		ir::emit::fieldset(seq.out, source, /*self*/lvid, 0); // builtin
	}
	else
		ir::emit::copy(seq.out, lvid, source);
}


} // anonymous namespace


void SequenceBuilder::visit(const ir::isa::Operand<ir::isa::Op::pragma>& operands) {
	switch (operands.pragma) {
		case ir::isa::Pragma::codegen: {
			codegen(*this, operands.value.codegen != 0);
			break;
		}
		case ir::isa::Pragma::bodystart: {
			// In 'signature only' mode, we only care about the
			// parameter user types. Everything after this opcode is unrelevant
			bool r = (!signatureOnly and bodyStart(*this)); // params deep copy, implicit var auto-init...
			if (not r)
				currentSequence.invalidateCursor(*cursor);
			break;
		}
		case ir::isa::Pragma::blueprintsize: {
			blueprintSize(*this, operands.value.blueprintsize);
			break;
		}
		case ir::isa::Pragma::visibility: {
			assert(frame != nullptr);
			break;
		}
		case ir::isa::Pragma::shortcircuitOpNopOffset: {
			shortcircuit.label = operands.value.shortcircuitMetadata.label;
			break;
		}
		case ir::isa::Pragma::shortcircuitMutateToBool: {
			shortcircuitMutateToBool(*this, operands);
			break;
		}
		case ir::isa::Pragma::synthetic: {
			uint32_t lvid = operands.value.synthetic.lvid;
			bool onoff = (operands.value.synthetic.onoff != 0);
			frame->lvids(lvid).synthetic = onoff;
			break;
		}
		case ir::isa::Pragma::suggest:
		case ir::isa::Pragma::builtinalias:
		case ir::isa::Pragma::shortcircuit:
		case ir::isa::Pragma::unknown:
			break;
	}
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
