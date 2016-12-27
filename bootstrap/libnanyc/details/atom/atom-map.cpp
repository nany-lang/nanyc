#include "atom-map.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"

using namespace Yuni;


namespace ny {

namespace {


auto createDummyAtom() {
	auto atom = yuni::make_ref<Atom>("", Atom::Type::classdef);
	atom->builtinMapping = nyt_void;
	return atom;
}


auto findCoreObject(nytype_t kind, const AnyString& name, Atom& root) {
	Atom* atom = nullptr;
	switch (root.findClassAtom(atom, name)) {
		case 1: {
			assert(atom != nullptr);
			atom->builtinMapping = kind;
			return Ref<Atom>{atom};
		}
		case 0: {
			error() << "failed to find builtin 'class " << name << "' from nsl";
			break;
		}
		default:
			error() << "multiple definition for 'class" << name << "'";
	}
	throw std::runtime_error("not found");
}


} // anonymous namespace


AtomMap::AtomMap(StringRefs& stringrefs)
	: root(AnyString(), Atom::Type::namespacedef) // global namespace
	, stringrefs(stringrefs) {
	// since `m_atomGrpID` will start from 1
	m_byIndex.emplace_back(nullptr);
}


Atom& AtomMap::createNewAtom(Atom::Type type, Atom& parent, const AnyString& name) {
	auto newnode = yuni::make_ref<Atom>(parent, stringrefs.refstr(name), type);
	newnode->atomid = ++m_atomGrpID;
	m_byIndex.emplace_back(newnode);
	return *newnode;
}


Atom& AtomMap::createVardef(Atom& parent, const AnyString& name) {
	assert(not name.empty());
	auto& atom = createNewAtom(Atom::Type::vardef, parent, name);
	auto fieldindex = parent.classinfo.nextFieldIndex++;
	atom.varinfo.fieldindex = fieldindex;
	atom.varinfo.effectiveFieldIndex = fieldindex;
	return atom;
}


AnyString AtomMap::symbolname(uint32_t atomid, uint32_t index) const {
	if (atomid < m_byIndex.size())
		return m_byIndex[atomid]->instances[index].symbolname();
	return AnyString{};
}


bool AtomMap::fetchAndIndexCoreObjects() {
	if (unlikely(!!core.object[nyt_bool]))
		return true;
	if (not config::importNSL) {
		for (auto& object: core.object)
			object = createDummyAtom();
		return true;
	}
	try {
		core.object[nyt_bool] = findCoreObject(nyt_bool, "bool", root);
		core.object[nyt_i8]   = findCoreObject(nyt_i8,   "i8",  root);
		core.object[nyt_i16]  = findCoreObject(nyt_i16,  "i16", root);
		core.object[nyt_i32]  = findCoreObject(nyt_i32,  "i32", root);
		core.object[nyt_i64]  = findCoreObject(nyt_i64,  "i64", root);
		core.object[nyt_u8]   = findCoreObject(nyt_u8,   "u8",  root);
		core.object[nyt_u16]  = findCoreObject(nyt_u16,  "u16", root);
		core.object[nyt_u32]  = findCoreObject(nyt_u32,  "u32", root);
		core.object[nyt_u64]  = findCoreObject(nyt_u64,  "u64", root);
		core.object[nyt_f32]  = findCoreObject(nyt_f32,  "f32", root);
		core.object[nyt_f64]  = findCoreObject(nyt_f64,  "f64", root);
		core.object[nyt_ptr]  = findCoreObject(nyt_ptr,  "pointer", root);
		return true;
	}
	catch (const std::exception&) {
		// invalidate
		core.object[nyt_bool] = nullptr;
	}
	return false;
}


} // namespace ny
