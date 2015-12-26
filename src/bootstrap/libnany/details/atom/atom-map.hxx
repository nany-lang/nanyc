#pragma once
#include "atom-map.h"




namespace Nany
{

	inline Atom* AtomMap::createNamespace(Atom& root, const AnyString& name)
	{
		assert(not name.empty());
		return createNewAtom(Atom::Type::namespacedef, root, name);
	}

	inline Atom* AtomMap::createFuncdef(Atom& root, const AnyString& name)
	{
		assert(not name.empty());
		return createNewAtom(Atom::Type::funcdef, root, name);
	}

	inline Atom* AtomMap::createClassdef(Atom& root, const AnyString& name)
	{
		assert(not name.empty());
		return createNewAtom(Atom::Type::classdef, root, name);
	}

	inline Atom* AtomMap::createVardef(Atom& root, const AnyString& name)
	{
		assert(not name.empty());
		auto* atom = createNewAtom(Atom::Type::vardef, root, name);
		auto fieldindex = root.classinfo.nextFieldIndex++;
		atom->varinfo.fieldindex = fieldindex;
		atom->varinfo.effectiveFieldIndex = fieldindex;
		return atom;
	}


	inline const IR::Program& AtomMap::program(uint32_t atomid, uint32_t instanceid) const
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
