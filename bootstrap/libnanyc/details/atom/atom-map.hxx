#pragma once
#include "atom-map.h"


namespace ny {


inline Atom& AtomMap::createNamespace(Atom& parent, const AnyString& name) {
	assert(not name.empty());
	Atom* nmspc = parent.findNamespaceAtom(name);
	return (nmspc != nullptr)
		   ? *nmspc : createNewAtom(Atom::Type::namespacedef, parent, name);
}


inline Atom& AtomMap::createFuncdef(Atom& parent, const AnyString& name) {
	assert(not name.empty());
	return createNewAtom(Atom::Type::funcdef, parent, name);
}


inline Atom& AtomMap::createClassdef(Atom& parent, const AnyString& name) {
	assert(not name.empty());
	return createNewAtom(Atom::Type::classdef, parent, name);
}


inline Atom& AtomMap::createTypealias(Atom& parent, const AnyString& name) {
	assert(not name.empty());
	return createNewAtom(Atom::Type::typealias, parent, name);
}


inline Atom& AtomMap::createUnit(Atom& parent, const AnyString& name) {
	return createNewAtom(Atom::Type::unit, parent, name);
}


inline const ir::Sequence& AtomMap::ircode(uint32_t atomid, uint32_t index) const {
	assert(atomid < m_byIndex.size());
	return m_byIndex[atomid]->instances[index].ircode();
}


inline const ir::Sequence* AtomMap::ircodeIfExists(uint32_t atomid, uint32_t index) const {
	return (atomid < m_byIndex.size())
		? m_byIndex[atomid]->instances[index].ircodeIfExists() : nullptr;
}


inline yuni::Ref<Atom> AtomMap::findAtom(uint32_t atomid) const {
	return atomid < m_byIndex.size() ? m_byIndex[atomid] : nullptr;
}


inline yuni::Ref<Atom> AtomMap::findAtom(uint32_t atomid) {
	return atomid < m_byIndex.size() ? m_byIndex[atomid] : nullptr;
}


} // namespace ny
