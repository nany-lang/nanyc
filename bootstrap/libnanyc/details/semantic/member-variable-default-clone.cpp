#include "semantic-analysis.h"
#include "member-variable.h"
#include "ref-unref.h"

using namespace Yuni;

namespace ny::semantic {

void produceMemberVarDefaultClone(Analyzer& analyzer) {
	assert(analyzer.canGenerateCode());
	assert(analyzer.frame != nullptr);
	auto& frame = *analyzer.frame;
	assert(frame.offsetOpcodeStacksize != (uint32_t) - 1);
	// special location: in a constructor - initializing all variables with their def value
	// note: do not keep a reference on 'out->at...', since the internal buffer might be reized
	auto& parentAtom = *(frame.atom.parent);
	// do not warn about rhs (unused). The parameter is used, but not via its name
	// reminder: 1-based, 1: returntype, 2: self, 3: first parameter rhs
	frame.lvids(3).warning.unused = false;
	// looking for all members to clone
	std::vector<std::reference_wrapper<Atom>> atomvars;
	Atom* userDefinedClone = nullptr;
	parentAtom.eachChild([&](Atom & subatom) -> bool {
		if (subatom.isMemberVariable()) {
			atomvars.emplace_back(std::ref(subatom));
		}
		else
		{
			if (subatom.isCloneCtor())
				userDefinedClone = &subatom;
		}
		return true; // next
	});
	if (atomvars.empty() and userDefinedClone == nullptr)
		return;
	auto& cdeftable = analyzer.cdeftable;
	auto& out = analyzer.out;
	// create new local variables for performing the cline
	uint32_t more = (uint32_t)atomvars.size() * 2 + (userDefinedClone ? 1 : 0);
	uint32_t lvid = analyzer.createLocalVariables(more);
	for (auto& subatomref : atomvars) {
		auto& subatom = subatomref.get();
		auto& cdef    = cdeftable.classdef(subatom.returnType.clid); // type of the var member
		ir::emit::trace(out, [&]() {
			return String{"\nCLONE for '"} << subatom.name() << '\'';
		});
		switch (cdef.kind) {
			case CType::t_any: {
				uint32_t rhsptr = lvid++; // rhs value, from the object being cloned
				uint32_t lhsptr = lvid++; // the target local value
				auto& origin  = frame.lvids(rhsptr).origin.varMember;
				origin.self   = 2;
				origin.atomid = subatom.atomid;
				origin.field  = subatom.varinfo.effectiveFieldIndex;
				auto& cdeflhs = cdeftable.substitute(lhsptr);
				cdeflhs.import(cdef);
				cdeflhs.qualifiers = cdef.qualifiers; // qualifiers must be preserved
				auto& cdefrhs = cdeftable.substitute(rhsptr);
				cdefrhs.import(cdef);
				cdefrhs.qualifiers = cdef.qualifiers;
				// fetching the rhs value, from the object being copied
				ir::emit::fieldget(out, rhsptr, /*rhs*/ 3, subatom.varinfo.effectiveFieldIndex);
				// perform a deep copy to the local variable
				analyzer.instanciateAssignment(frame, lhsptr, rhsptr, false);
				// .. copied to the member
				ir::emit::fieldset(out, lhsptr, /*self*/ 2, subatom.varinfo.effectiveFieldIndex);
				// prevent the cloned object from being released at the end of the scope
				assert(canBeAcquired(analyzer, lhsptr));
				frame.lvids(lhsptr).autorelease = false;
				break;
			}
			case CType::t_void: {
				ice() << "unexpected pseudo type 'void' for " << cdef.clid;
				break;
			}
			default: {
				// rhs value, from the object being clone
				ir::emit::fieldget(out, lvid, /*rhs*/  3, subatom.varinfo.effectiveFieldIndex);
				// .. copied directly into the local member
				ir::emit::fieldset(out, lvid, /*self*/ 2, subatom.varinfo.effectiveFieldIndex);
				++lvid;
			}
		}
	}
	// call to user-defined operator dispose
	if (userDefinedClone) {
		ir::emit::trace(out, "\nuser's defined clone method");
		ir::emit::push(out, 2); // self
		ir::emit::push(out, 3); // rhs
		ir::emit::call(out, lvid, userDefinedClone->atomid, 0);
		++lvid;
	}
}

} // ny::semantic
