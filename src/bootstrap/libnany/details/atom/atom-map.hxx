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

	inline Atom* AtomMap::createVardef(Atom& parent, const AnyString& name)
	{
		assert(not name.empty());
		auto* atom = createNewAtom(Atom::Type::vardef, parent, name);
		auto fieldindex = parent.classinfo.nextFieldIndex++;
		atom->varinfo.fieldindex = fieldindex;
		atom->varinfo.effectiveFieldIndex = fieldindex;
		return atom;
	}


	inline const IR::Sequence& AtomMap::sequence(uint32_t atomid, uint32_t instanceid) const
	{
		assert(atomid < pByIndex.size());
		return pByIndex[atomid]->instance(instanceid);
	}


	inline const Atom* AtomMap::findAtom(uint32_t atomid) const
	{
		return atomid < pByIndex.size() ? pByIndex[atomid] : nullptr;
	}


	inline Atom* AtomMap::findAtom(uint32_t atomid)
	{
		return atomid < pByIndex.size() ? pByIndex[atomid] : nullptr;
	}



} // namespace Nany
