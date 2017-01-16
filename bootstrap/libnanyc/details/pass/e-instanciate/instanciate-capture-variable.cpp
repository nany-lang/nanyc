#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "instanciate.h"
#include "libnanyc-traces.h"

using namespace Yuni;


namespace ny {
namespace Pass {
namespace Instanciate {


void SequenceBuilder::captureVariables(Atom& atom) {
	// Try to capture variables from a list of potentiel candidates created by the mapping
	// get the ownership of the container to release it
	assert(atom.candidatesForCapture != nullptr);
	auto candidatesPtr = std::move(atom.candidatesForCapture);
	auto& originalCandidates = *candidatesPtr;
	if (unlikely(originalCandidates.empty()))
		return;
	// The list of variables that can be really captured
	struct CapturedVar final {
		AnyString name;
		CLID clid;
	};
	std::unique_ptr<CapturedVar[]> candidates;
	//
	// -- FILTERING
	// Keeping only local variables from the candidate list
	//
	uint32_t count = 0;
	{
		ShortString128 newVarname;
		for (auto& varname : originalCandidates) {
			uint32_t lvid = frame->findLocalVariable(varname);
			if (lvid != 0) { // local variable
				if (!candidates)
					candidates = std::make_unique<CapturedVar[]>(originalCandidates.size());
				auto& element = candidates[count++];
				newVarname.clear() << "^trap^" << varname;
				// the new name must be stored somewhere
				element.name = currentSequence.stringrefs.refstr(newVarname);
				element.clid.reclass(frame->atomid, lvid);
			}
		}
	}
	if (count == 0) // nothing to capture
		return;
	assert(!!candidates);
	assert(atom.opcodes.ircode and "invalid empty IR sequence");
	if (unlikely(atom.opcodes.ircode == nullptr))
		return;
	auto& ircode = *atom.opcodes.ircode;
	auto& table = cdeftable.originalTable();
	// Base offset for the new lvid (new captured variables in the class)
	uint32_t startLvid = atom.localVariablesCount + 1; // 1-based
	auto offset = atom.opcodes.offset;
	#ifndef NDEBUG
	auto& blueprint = ircode.at<ir::isa::Op::blueprint>(offset);
	assert(blueprint.opcode == (uint32_t) ir::isa::Op::blueprint);
	#endif
	// blueprint size
	++offset;
	#ifndef NDEBUG
	auto& blueprintsize = ircode.at<ir::isa::Op::pragma>(offset);
	assert(blueprintsize.opcode == (uint32_t) ir::isa::Op::pragma);
	#endif
	// Updating the opcode stacksize
	++offset;
	auto& stacksize = ircode.at<ir::isa::Op::stacksize>(offset);
	assert(stacksize.opcode == static_cast<uint32_t>(ir::isa::Op::stacksize));
	assert(stacksize.add + 1 == startLvid);
	if (unlikely(stacksize.opcode != static_cast<uint32_t>(ir::isa::Op::stacksize)))
		return (void)(ice() << "capturing variable: stacksize opcode expected");
	// new stack size
	stacksize.add += count;
	atom.localVariablesCount += count;
	table.bulkAppend(atom.atomid, startLvid, count);
	for (uint32_t i = 0; i != count; ++i) {
		auto& var = candidates[i];
		// new atom for the new variable member
		auto& newVarAtom = table.atoms.createVardef(atom, var.name);
		table.registerAtom(newVarAtom);
		auto& cdef = table.rawclassdef(CLID{atom.atomid, startLvid + i});
		auto& src  = table.classdef(var.clid);
		// the new captured variable is obviously a 'ref' and shares
		// the same type than the original one
		auto* varSrcAtom = table.findClassdefAtom(src);
		if (unlikely(varSrcAtom == nullptr))
			ice() << "invalid atom for captured variable '" << var.name << "'";
		cdef.mutateToAtom(varSrcAtom);
		cdef.qualifiers.ref = true;
		newVarAtom.returnType.clid = cdef.clid;
		assert(var.clid.atomid() == frame->atomid);
		if (var.clid.atomid() == frame->atomid) {
			// mark the captured variables as used to avoid spurious warnings
			frame->lvids(var.clid.lvid()).warning.unused = false;
			// reset the subtitute
			auto& spare = cdeftable.substitute(var.clid.lvid());
			spare.mutateToAtom(varSrcAtom);
			spare.qualifiers.ref = true;
		}
	}
	// When instanciating this method, automatically push captured variables
	atom.flags += Atom::Flags::pushCapturedVariables;
	// adding parameters to all constructors
	atom.eachChild([&](Atom & child) -> bool {
		if (child.isCtor()) {
			// appending parameters to ctor
			for (uint32_t i = 0; i != count; ++i) {
				auto& var = candidates[i];
				CLID clid{atom.atomid, startLvid + i};
				bool success = child.parameters.append(clid, var.name);
				if (unlikely(not success)) {
					error() << "too many parameters for appending captured variable in "
							<< frame->atom.caption();
				}
			}
			// need extra size for the next instanciation
			child.opcodes.stackSizeExtra = count;
		}
		return true; // continue iteration
	});
}


bool SequenceBuilder::pushCapturedVarsAsParameters(const Atom& atomclass) {
	atomclass.eachChild([&](Atom & child) -> bool {
		if (child.isCapturedVariable()) {
			AnyString varname{child.name(), /*offset*/ (uint32_t)::strlen("^trap^")};
			if (unlikely(varname.empty())) {
				ice() << "invalid empty captured variable name";
				return true;
			}
			uint32_t varlvid = frame->findLocalVariable(varname);
			if (unlikely(0 == varlvid)) {
				error() << "failed to find captured variable '" << varname << "'";
				return true;
			}
			if (not frame->verify(varlvid))
				return true;
			// named parameter for captured variable
			CLID clid{frame->atomid, varlvid};
			overloadMatch.input.params.named.emplace_back(std::make_pair(child.name(), clid));
			// checking that the definition is still accurate
			assert(cdeftable.findClassdefAtom(cdeftable.classdef(clid)) != nullptr);
		}
		return true;
	});
	return true;
}


bool SequenceBuilder::identifyCapturedVar(const ir::isa::Operand<ir::isa::Op::identify>& operands,
		const AnyString& name) {
	AnyString captureName;
	{
		ShortString128 newname;
		newname << "^trap^" << name;
		captureName = currentSequence.stringrefs.refstr(newname);
	}
	return identify(operands, captureName, false);
}


} // namespace Instanciate
} // namespace Pass
} // namespace ny
