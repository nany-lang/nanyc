#include "atom-map.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"

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


	const IR::Sequence* AtomMap::fetchSequence(uint32_t atomid, uint32_t instanceid) const
	{
		if (atomid < pByIndex.size())
			return pByIndex[atomid]->fetchInstance(instanceid);
		return nullptr;
	}


	AnyString AtomMap::fetchSequenceCaption(uint32_t atomid, uint32_t instanceid) const
	{
		if (atomid < pByIndex.size())
			return pByIndex[atomid]->fetchInstanceCaption(instanceid);
		return AnyString{};
	}




	namespace // anonymous
	{
		static inline bool findCoreObject(Atom::Ptr& out, nytype_t kind, const AnyString& name, Atom& root)
		{
			if (Config::importNSL)
			{
				Atom* atom;
				switch (root.findClassAtom(atom, name))
				{
					case 1:
					{
						out = atom;
						atom->builtinMapping = kind;
						return true;
					}
					case 0:
					{
						error() << "failed to find builtin 'class " << name << "' from nsl";
						break;
					}
					default:
					{
						error() << "multiple definition for 'class" << name << "'";
					}
				}
				return false;
			}
			else
			{
				out = Atom::createDummy();
				out->builtinMapping = nyt_void;
				return true;
			}
		}

	} // anonymous namespace


	bool AtomMap::fetchAndIndexCoreObjects()
	{
		bool success = true;

		if (core.object[nyt_bool] == nullptr) // quick & arbitrary check
		{
			success &= findCoreObject(core.object[nyt_bool], nyt_bool, "bool", root);

			success &= findCoreObject(core.object[nyt_i8],  nyt_i8,  "i8",  root);
			success &= findCoreObject(core.object[nyt_i16], nyt_i16, "i16", root);
			success &= findCoreObject(core.object[nyt_i32], nyt_i32, "i32", root);
			success &= findCoreObject(core.object[nyt_i64], nyt_i64, "i64", root);

			success &= findCoreObject(core.object[nyt_u8],  nyt_u8,  "u8",  root);
			success &= findCoreObject(core.object[nyt_u16], nyt_u16, "u16", root);
			success &= findCoreObject(core.object[nyt_u32], nyt_u32, "u32", root);
			success &= findCoreObject(core.object[nyt_u64], nyt_u64, "u64", root);

			success &= findCoreObject(core.object[nyt_f32], nyt_f32, "f32", root);
			success &= findCoreObject(core.object[nyt_f64], nyt_f64, "f64", root);

			success &= findCoreObject(core.object[nyt_ptr], nyt_ptr, "pointer", root);

			if (unlikely(not success))
				core.object[nyt_bool] = nullptr;
		}
		return success;
	}



} // namespace Nany
