#include "atom-map.h"
#include "details/reporting/report.h"

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


	namespace // anonymous
	{
		static inline bool findCoreObject(Atom::Ptr& out, const AnyString& name, Logs::Report& report, Atom& root)
		{
			Atom* atom;
			switch (root.findClassAtom(atom, name))
			{
				case 1:
				{
					out = atom;
					return true;
				}
				case 0:
				{
					report.error() << "failed to find ' class" << name << "'";
					break;
				}
				default:
				{
					report.error() << "multiple definition for 'class" << name << "'";
				}
			}
			return false;
		}

	} // anonymous namespace


	bool AtomMap::fetchAndIndexCoreObjects(Logs::Report& report)
	{
		bool success = true;
		if (core.object[nyt_bool] == nullptr)
		{
			success &= findCoreObject(core.object[nyt_bool], "bool", report, root);

			if (unlikely(not success))
				core.object[nyt_bool] = nullptr;
		}
		return success;
	}



} // namespace Nany
