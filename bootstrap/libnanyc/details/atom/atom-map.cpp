#include "atom-map.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"
#include "details/errors/complain.h"

using namespace Yuni;


namespace ny {

namespace {


struct MissingBuiltin final: std::exception {
	MissingBuiltin(AnyString name) : name{name} {}
	const char* what() const noexcept override {return nullptr;}
	AnyString name;
};


struct MultipleDefinition final: std::exception {
	MultipleDefinition(AnyString name) : name{name} {}
	const char* what() const noexcept override {return nullptr;}
	AnyString name;
};


auto createDummyAtom() {
	auto atom = yuni::make_ref<Atom>("", Atom::Type::classdef);
	atom->builtinMapping = nyt_void;
	return atom;
}


auto findBuiltinAtom(Atom& root, nytype_t kind, const AnyString& name) {
	Atom* atom = nullptr;
	switch (root.findClassAtom(atom, name)) {
		case 1: {
			assert(atom != nullptr);
			atom->builtinMapping = kind;
			return Ref<Atom>{atom};
		}
		case 0:  throw MissingBuiltin{name};
		default: throw MultipleDefinition{name};
	}
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
		core.object[nyt_bool] = findBuiltinAtom(root, nyt_bool, "bool");
		core.object[nyt_i8]   = findBuiltinAtom(root, nyt_i8,   "i8");
		core.object[nyt_i16]  = findBuiltinAtom(root, nyt_i16,  "i16");
		core.object[nyt_i32]  = findBuiltinAtom(root, nyt_i32,  "i32");
		core.object[nyt_i64]  = findBuiltinAtom(root, nyt_i64,  "i64");
		core.object[nyt_u8]   = findBuiltinAtom(root, nyt_u8,   "u8");
		core.object[nyt_u16]  = findBuiltinAtom(root, nyt_u16,  "u16");
		core.object[nyt_u32]  = findBuiltinAtom(root, nyt_u32,  "u32");
		core.object[nyt_u64]  = findBuiltinAtom(root, nyt_u64,  "u64");
		core.object[nyt_f32]  = findBuiltinAtom(root, nyt_f32,  "f32");
		core.object[nyt_f64]  = findBuiltinAtom(root, nyt_f64,  "f64");
		core.object[nyt_ptr]  = findBuiltinAtom(root, nyt_ptr,  "pointer");
		return true;
	}
	catch (const MissingBuiltin& e) {
		ice() << "failed to find builtin '" << e.name << '\'';
	}
	catch (const MultipleDefinition& e) {
		ice() << "multiple definition for type '" << e.name << '\'';
	}
	catch (const std::exception& e) {
		ny::complain::exception(e);
	}
	core.object[nyt_bool] = nullptr;
	return false;
}


} // namespace ny
