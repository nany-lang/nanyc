#include "atom-map.h"

using namespace Yuni;




namespace Nany
{

	AtomMap::AtomMap(StringRefs& stringrefs)
		: root(AnyString(), Atom::Type::namespacedef) // global namespace
		, stringrefs(stringrefs)
	{
		// since `pAtomGrpID` will start from 1
		pByIndex.push_back(nullptr);
	}


	Atom* AtomMap::createNewAtom(Atom::Type type, Atom& root, const AnyString& name)
	{
		auto* newnode   = new Atom(root, stringrefs.refstr(name), type);
		newnode->atomid = ++pAtomGrpID;
		pByIndex.emplace_back(newnode);
		return newnode;
	}


	const IR::Program* AtomMap::fetchProgram(uint32_t atomid, uint32_t instanceid) const
	{
		if (atomid < pByIndex.size())
			return pByIndex[atomid]->fetchInstance(instanceid);
		return nullptr;
	}


	AnyString AtomMap::fetchProgramCaption(uint32_t atomid, uint32_t instanceid) const
	{
		if (atomid < pByIndex.size())
			return pByIndex[atomid]->fetchInstanceCaption(instanceid);
		return AnyString{};
	}




} // namespace Nany
