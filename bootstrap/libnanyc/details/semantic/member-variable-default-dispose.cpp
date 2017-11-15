#include "semantic-analysis.h"
#include "member-variable.h"

using namespace Yuni;

namespace ny {
namespace semantic {

namespace {

void complainInvalidAtom(Atom& subatom, const Classdef& cdef) {
	auto e = ice();
	e << "invalid atom from " << cdef.clid << " for disposing member variable '";
	subatom.retrieveFullname(e.data().message);
	e << '\'';
}

bool categorizeAtomChildren(Atom& subatom, std::vector<std::reference_wrapper<Atom>>& atomvars, Atom*& userDefinedDispose) {
	if (subatom.isMemberVariable()) {
		atomvars.emplace_back(std::ref(subatom));
	}
	else
	{
		if (subatom.isDtor())
			userDefinedDispose = &subatom;
	}
	return true; // next
}

} // namespace

void produceMemberVarDefaultDispose(Analyzer& analyzer) {
	assert(analyzer.canGenerateCode());
	assert(analyzer.frame != nullptr);
	auto& frame = *analyzer.frame;
	assert(frame.offsetOpcodeStacksize != (uint32_t) - 1);
	// special location: in a constructor - initializing all variables with their def value
	// note: do not keep a reference on 'out->at...', since the internal buffer might be reized
	auto& parentAtom = *(frame.atom.parent);
	std::vector<std::reference_wrapper<Atom>> atomvars;
	Atom* userDefinedDispose = nullptr;
	parentAtom.eachChild([&](Atom & subatom) -> bool {
		return categorizeAtomChildren(subatom, atomvars, userDefinedDispose);
	});
	if (atomvars.empty() and userDefinedDispose == nullptr)
		return;
	// resize
	uint32_t more = (uint32_t)atomvars.size() + (userDefinedDispose ? 1 : 0);
	uint32_t lvid = analyzer.createLocalVariables(more);
	auto& cdeftable = analyzer.cdeftable;
	auto& out = analyzer.out;
	// call to user-defined operator dispose
	if (userDefinedDispose) {
		ir::emit::trace(out, "calling user destructor");
		ir::emit::push(out, 2); // self
		ir::emit::call(out, lvid, userDefinedDispose->atomid, 0);
		cdeftable.substitute(lvid).mutateToVoid();
		++lvid;
	}
	bool commentAdded = false;
	// ... then release all variables !
	for (auto& subatomref : atomvars) {
		auto& subatom = subatomref.get();
		auto& cdef    = cdeftable.classdef(subatom.returnType.clid);
		// nothing to do for pod types
		if (cdef.isBuiltinOrVoid())
			continue;
		ir::emit::trace(out, (not commentAdded), "releasing variable members");
		commentAdded = true;
		// current lvig, previously allocated to release variable members
		uint32_t reglvid = lvid++;
		// type propagation
		cdeftable.substitute(reglvid).import(cdef);
		// ir::emit::trace(out, [&](){ return String("dispose for ") << subatom.name;});
		// read the pointer
		ir::emit::fieldget(out, reglvid, /*self*/ 2, subatom.varinfo.effectiveFieldIndex);
		auto& origin  = frame.lvids(reglvid).origin.varMember;
		origin.self   = 2;
		origin.atomid = subatom.atomid;
		origin.field  = subatom.varinfo.effectiveFieldIndex;
		auto* typeAtom = cdeftable.findClassdefAtom(cdef);
		if (unlikely(nullptr == typeAtom)) {
			complainInvalidAtom(subatom, cdef);
			continue;
		}
		if (0 == typeAtom->classinfo.dtor.atomid) {
			if (not analyzer.instanciateAtomClassDestructor(*typeAtom, reglvid))
				continue;
		}
		auto& classinfo = typeAtom->classinfo;
		ir::emit::unref(out, reglvid, classinfo.dtor.atomid);
	}
}

} // namespace semantic
} // namespace ny
