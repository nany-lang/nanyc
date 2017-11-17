#include "semantic-analysis.h"

using namespace Yuni;

namespace ny {
namespace semantic {

namespace {

void funcOrClassOrType(Analyzer& seq, const ir::isa::Operand<ir::isa::Op::blueprint>& operands) {
	seq.pushedparams.clear();
	seq.bodystart.memberVarsInit = false;
	seq.bodystart.memberVarsRelease = false;
	seq.bodystart.memberVarsClone = false;
	bool bug = (seq.layerDepthLimit == 0); // TODO: determine why this value can be zero with this opcode
	if (not bug)
		--seq.layerDepthLimit;
	uint32_t atomid = operands.atomid;
	uint32_t lvid   = operands.lvid;
	// retrieving the atomid - the atomid may be different from the one requested
	// (class with generic types parameters, anonymous classes...)
	auto atomptr = seq.cdeftable.atoms().findAtom(atomid);
	if (unlikely(!atomptr)) {
		seq.complainOperand(ir::Instruction::fromOpcode(operands), "invalid atom");
		return;
	}
	auto& atom = *atomptr;
	assert(atom.isFunction() or atom.isClass() or atom.isTypeAlias());
	auto& currentSequence = seq.currentSequence;
	// 2 cases can be encountered:
	// - a normal class definition: 'operands.lvid' will be equal to 0
	//   since there is no lvid to update (linked to nothing)
	// - an anonymous class (in the middle of a function for example):
	//   'operands.lvid' will be different from 0. However, when the class
	//   will be instanciated, we will be called again, but without any current 'frame'
	if (lvid == 0 or seq.frame == nullptr) {
		seq.pushNewFrame(atom);
		seq.frame->offsetOpcodeBlueprint = currentSequence.offsetOf(**seq.cursor);
		auto kind = static_cast<ir::isa::Blueprint>(operands.kind);
		if (kind == ir::isa::Blueprint::funcdef) {
			// some special actions must be triggered according the operator name
			AnyString atomname = currentSequence.stringrefs[operands.name];
			if (atomname.first() == '^') { // operator spotted
				assert(atomname.size() >= 2);
				switch (atomname[1]) {
					case 'd':
						if (atomname == "^default-new")
							seq.bodystart.memberVarsInit = true; // same as '^new'
						else if (atomname == "^dispose")
							seq.bodystart.memberVarsRelease = true;
						break;
					case 'n':
						if (atomname == "^new")
							seq.bodystart.memberVarsInit = true; // same as '^default-new'
						break;
					case 'o':
						if (atomname == "^obj-clone")
							seq.bodystart.memberVarsClone = true;
						break;
				}
			}
		}
	}
	else {
		currentSequence.moveCursorFromBlueprintToEnd(*seq.cursor);
		if (bug)
			*seq.cursor = &currentSequence.at(currentSequence.offsetOf(**seq.cursor) + 1);
		if (lvid != 0 and atom.isClass()) {
			// anonymous class
			// The flag Atom::Flags::captureVariables should already be set via 'mapping'
			assert(atom.flags(Atom::Flags::captureVariables));
			Atom* resAtom = seq.instanciateAtomClass(atom);
			if (unlikely(!resAtom)) {
				if (seq.frame)
					seq.frame->invalidate(lvid);
				return;
			}
			seq.cdeftable.substitute(lvid).mutateToAtom(resAtom);
		}
	}
}

void unit(Analyzer& seq, const ir::isa::Operand<ir::isa::Op::blueprint>& operands) {
	assert(seq.frame != nullptr);
	seq.pushedparams.clear();
	seq.bodystart.memberVarsInit = false;
	seq.bodystart.memberVarsRelease = false;
	seq.bodystart.memberVarsClone = false;
	uint32_t atomid = operands.atomid;
	auto atom = seq.cdeftable.atoms().findAtom(atomid);
	if (unlikely(!atom)) {
		seq.complainOperand(ir::Instruction::fromOpcode(operands), "invalid unit atom");
		return;
	}
	seq.pushNewFrame(*atom);
	seq.frame->offsetOpcodeBlueprint = seq.currentSequence.offsetOf(**seq.cursor);
}

void parameter(Analyzer& seq, uint32_t lvid, bool isvar, uint32_t nameindex) {
	assert(seq.frame != nullptr);
	auto& cdef = seq.cdeftable.substitute(lvid);
	cdef.qualifiers.ref = false;
	cdef.instance = isvar;
	auto& details = seq.frame->lvids(lvid);
	details.synthetic = not isvar;
	details.warning.unused = isvar;
	const auto& name = seq.currentSequence.stringrefs[nameindex];
	seq.declareNamedVariable(name, lvid, false);
}

void asSelf(Analyzer& seq, const ir::isa::Operand<ir::isa::Op::blueprint>& operands) {
	// -- with automatic variable assignment for operator new
	// example: operator new (self varname) {}
	assert(seq.frame != nullptr);
	auto& frame = *seq.frame;
	if (unlikely(not frame.atom.isClassMember())) {
		error() << "automatic variable assignment is only allowed in class operator 'new'";
		return;
	}
	if (!frame.selfParameters.get())
		frame.selfParameters = std::make_unique<decltype(frame.selfParameters)::element_type>();
	uint32_t sid  = operands.name;
	uint32_t lvid = operands.lvid;
	AnyString varname = seq.currentSequence.stringrefs[sid];
	(*frame.selfParameters)[varname].first = lvid;
}

void vardef(Analyzer& seq, uint32_t lvid, uint32_t sid) {
	if (seq.frame != nullptr) {
		if (seq.frame->atom.isClass()) {
			const AnyString& varname = seq.currentSequence.stringrefs[sid];
			Atom* atom = nullptr;
			if (1 != seq.frame->atom.findVarAtom(atom, varname)) {
				ice() << "unknown variable member '" << varname << "'";
				return;
			}
			atom->returnType.clid.reclass(seq.frame->atomid, lvid);
		}
	}
	seq.pushedparams.clear();
}

} // namespace

void Analyzer::visit(const ir::isa::Operand<ir::isa::Op::blueprint>& operands) {
	auto kind = static_cast<ir::isa::Blueprint>(operands.kind);
	switch (kind) {
		case ir::isa::Blueprint::param: {
			parameter(*this, operands.lvid, true, operands.name);
			break;
		}
		case ir::isa::Blueprint::gentypeparam: {
			parameter(*this, operands.lvid, false, operands.name);
			break;
		}
		case ir::isa::Blueprint::paramself: {
			asSelf(*this, operands);
			break;
		}
		case ir::isa::Blueprint::funcdef:
		case ir::isa::Blueprint::classdef:
		case ir::isa::Blueprint::typealias: {
			funcOrClassOrType(*this, operands);
			break;
		}
		case ir::isa::Blueprint::vardef: {
			vardef(*this, operands.lvid, operands.name);
			break;
		}
		case ir::isa::Blueprint::namespacedef: {
			break; // see mapping instead
		}
		case ir::isa::Blueprint::unit: {
			unit(*this, operands);
			break;
		}
	}
}

} // namespace semantic
} // namespace ny
