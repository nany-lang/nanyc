#pragma once
#include "atom-map.h"


namespace Nany
{


	inline Atom* AtomMap::createNamespace(Atom& parent, const AnyString& name)
	{
		assert(not name.empty());
		Atom* nmspc = parent.findNamespaceAtom(name);
		return (nmspc != nullptr)
			? nmspc : createNewAtom(Atom::Type::namespacedef, parent, name);
	}


	inline Atom* AtomMap::createFuncdef(Atom& parent, const AnyString& name)
	{
		assert(not name.empty());
		return createNewAtom(Atom::Type::funcdef, parent, name);
	}


	inline Atom* AtomMap::createClassdef(Atom& parent, const AnyString& name)
	{
		assert(not name.empty());
		return createNewAtom(Atom::Type::classdef, parent, name);
	}


	inline Atom* AtomMap::createTypealias(Atom& parent, const AnyString& name)
	{
		assert(not name.empty());
		return createNewAtom(Atom::Type::typealias, parent, name);
	}


	inline Atom* AtomMap::createUnit(Atom& parent, const AnyString& name)
	{
		return createNewAtom(Atom::Type::unit, parent, name);
	}


	inline const IR::Sequence& AtomMap::sequence(uint32_t atomid, uint32_t index) const
	{
		assert(atomid < m_byIndex.size());
		return m_byIndex[atomid]->instantiation(index).sequence();
	}


	inline const IR::Sequence* AtomMap::sequenceIfExists(uint32_t atomid, uint32_t index) const
	{
		return (atomid < m_byIndex.size())
			? m_byIndex[atomid]->instantiation(index).sequenceIfExists() : nullptr;
	}


	inline const Atom* AtomMap::findAtom(uint32_t atomid) const
	{
		return atomid < m_byIndex.size() ? m_byIndex[atomid] : nullptr;
	}


	inline Atom* AtomMap::findAtom(uint32_t atomid)
	{
		return atomid < m_byIndex.size() ? m_byIndex[atomid] : nullptr;
	}


} // namespace Nany
