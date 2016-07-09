#include "classdef-table.h"
#include "classdef-table-view.h"
#include <yuni/core/tribool.h>
#include "nany/nany.h"
#include "libnany-traces.h"
#include <cassert>

using namespace Yuni;
#include <iostream>




namespace Nany
{

	ClassdefTable::ClassdefTable()
		: atoms(stringrefs)
	{
		pClassdefs.insert(std::make_pair(CLID{}, new Classdef));
	}


	bool ClassdefTable::makeHardlink(const CLID& source, const CLID& target)
	{
		assert(not source.isVoid() and "invalid source CLID");
		assert(not target.isVoid() and "invalid target CLID");
		assert(source != target and "same target !");

		// looking for the source
		auto it = pClassdefs.find(source);
		if (it != pClassdefs.end())
		{
			// return *(it->second);
			// looking for the target
			auto ittarget = pClassdefs.find(target);
			if (ittarget != pClassdefs.end())
			{
				// the target has been found, replacing the classdef
				ittarget->second = it->second;
				return true;
			}
			//else
			//{
			//	// the target has not been found
			//	pClassdefs.insert(std::make_pair(target, it->second));
			//}
			//return true;
		}
		return false;
	}



	Classdef& ClassdefTable::classdef(const CLID& clid)
	{
		assert(not clid.isVoid() and "invalid clid");

		// pick first substitutes
		if (clid.atomid() == layer.atomid)
		{
			assert(clid.lvid() < layer.count);
			if (layer.flags[clid.lvid()])
				return layer.storage[clid.lvid()];
		}

		auto it = pClassdefs.find(clid);
		if (unlikely(it == pClassdefs.end()))
		{
			assert(false and "classdef not found");
			it = pClassdefs.find(CLID{});
		}
		auto& result = *(it->second);

		if (result.clid.atomid() == layer.atomid) // dealing with hard links
		{
			auto lvid = result.clid.lvid();
			assert(lvid < layer.count);
			if (layer.flags[lvid])
				return layer.storage[lvid];
		}
		return result;
	}


	const Classdef& ClassdefTable::rawclassdef(const CLID& clid) const
	{
		assert(not clid.isVoid() and "invalid clid");
		// TODO use an alternate (and more efficient) container for the special classdefs atomid=0

		auto it = pClassdefs.find(clid);
		if (unlikely(it == pClassdefs.end()))
		{
			assert(false and "failed to find clid");
			it = pClassdefs.find(CLID{});
		}
		return *(it->second);
	}


	Classdef& ClassdefTable::rawclassdef(const CLID& clid)
	{
		assert(not clid.isVoid() and "invalid clid");
		// TODO use an alternate (and more efficient) container for the special classdefs atomid=0

		auto it = pClassdefs.find(clid);
		if (unlikely(it == pClassdefs.end()))
		{
			assert(false and "failed to find clid");
			it = pClassdefs.find(CLID{});
		}
		return *(it->second);
	}


	const Classdef& ClassdefTable::classdef(const CLID& clid) const
	{
		assert(not clid.isVoid() and "invalid clid");
		// TODO use an alternate (and more efficient) container for the special classdefs atomid=0

		// pick first substitutes
		if (clid.atomid() == layer.atomid)
		{
			assert(clid.lvid() < layer.count);
			if (layer.flags[clid.lvid()])
				return layer.storage[clid.lvid()];
		}

		auto it = pClassdefs.find(clid);
		if (unlikely(it == pClassdefs.end()))
		{
			assert(false and "classdef not found");
			it = pClassdefs.find(CLID{});
		}
		auto& result = *(it->second);

		if (result.clid.atomid() == layer.atomid) // dealing with hard links
		{
			auto lvid = result.clid.lvid();
			assert(lvid < layer.count);
			if (layer.flags[lvid])
				return layer.storage[lvid];
		}
		return result;
	}


	const Classdef& ClassdefTable::classdefFollowClassMember(const CLID& clid) const
	{
		auto& cdef = classdef(clid);
		auto* atom = findClassdefAtom(cdef);
		if (atom and atom->isMemberVariable())
			return classdef(atom->returnType.clid);
		return cdef;
	}


	Funcdef& ClassdefTable::addClassdefInterface(const CLID& clid, const AnyString& name)
	{
		assert(not clid.isVoid() and "invalid clid");

		// empty name is allowed here (for representing the parent atom)
		auto* funcdef = (name.empty()) ? new Funcdef(nullptr): new Funcdef(stringrefs.refstr(name));
		auto& ref = classdef(clid);
		ref.interface.add(funcdef);
		return *funcdef;
	}


	Funcdef& ClassdefTable::addClassdefInterfaceSelf(const CLID& clid, const AnyString& name)
	{
		assert(not clid.isVoid() and "invalid clid");

		// empty name is allowed here (for representing the parent atom)
		auto* funcdef = (name.empty()) ? new Funcdef(nullptr): new Funcdef(stringrefs.refstr(name));
		classdef(clid).interface.pSelf = funcdef;
		return *funcdef;
	}



	void ClassdefTable::bulkCreate(std::vector<CLID>& out, uint32_t atomid, uint32_t count)
	{
		assert(atomid > 0);

		++count; // 1-based
		out.clear();
		out.reserve(count);
		out.push_back(CLID{});

		for (yuint32 i = 1; i != count; ++i)
		{
			// the new classid, made from the atom id
			CLID clid{atomid, i};
			out.push_back(clid);

			// check that the entry does not already exists
			assert(pClassdefs.find(clid) == pClassdefs.end());
			// insert the new classdef
			pClassdefs.insert(std::make_pair(clid, new Classdef{clid}));
		}
	}

	void ClassdefTable::bulkAppend(uint32_t atomid, uint32_t offset, uint32_t count)
	{
		assert(atomid > 0);
		for (uint32_t i = offset; i != offset + count; ++i)
		{
			// the new classid, made from the atom id
			CLID clid{atomid, i};

			// check that the entry does not already exists
			assert(pClassdefs.find(clid) == pClassdefs.end());
			// insert the new classdef
			pClassdefs.insert(std::make_pair(clid, new Classdef{clid}));
		}
	}

	void ClassdefTable::registerAtom(Atom* atom)
	{
		assert(atom != nullptr and "invalid atom");

		// TODO use an alternate (and more efficient) container for this special classdefs
		if (atom)
		{
			const CLID& clid = CLID::AtomMapID(atom->atomid);
			auto* newClassdef = new Classdef(clid);
			newClassdef->mutateToAtom(atom);
			pClassdefs.insert(std::make_pair(clid, newClassdef));
		}
	}









	inline bool ClassdefTable::nameLookupForSelfInterface(Classdef& classdef)
	{
		assert(classdef.interface.hasSelf());

		auto& funcdef = classdef.interface.self();
		// already resolved
		if (not funcdef.overloads.empty())
			return true;

		// <self> with empty names are only used to distinguish overloads
		// there is no need to resolve anything here
		if (funcdef.name.empty())
			return true;

		switch (funcdef.name[0])
		{
			default:
			{
				break;
			}
			case '_':
			{
				if (funcdef.name.startsWith("__"))
				{
					nytype_t btype = nany_cstring_to_type_n(funcdef.name.c_str(), (uint) funcdef.name.size());
					if (btype != nyt_void)
						classdef.mutateToBuiltin(btype);
					return (btype != nyt_void);
				}
				break;
			}
			case 'v':
			{
				if (unlikely(funcdef.name == "void"))
				{
					classdef.mutateToBuiltin(nyt_void);
					return true;
				}
				break;
			}
			case 'a':
			{
				if (unlikely(funcdef.name == "any"))
				{
					classdef.mutateToAny();
					return true;
				}
				break;
			}
		}

		// standard name lookup
		{
			Atom* atom = (this->classdef(CLID::AtomMapID(classdef.clid.atomid())).atom);
			if (atom)
			{
				if (atom->nameLookupFromParent(funcdef.overloads.getList(), funcdef.name))
				{
					if (funcdef.overloads.size() == 1)
					{
						// a single result has been found, fully-resolved
						auto& resolvedAtom = funcdef.overloads.getList()[0].get();
						funcdef.overloads.clear();
						classdef.mutateToAtom(&resolvedAtom);
						funcdef.clid = CLID::AtomMapID(resolvedAtom.atomid);
						funcdef.atom = &resolvedAtom;
					}
					return true;
				}
			}
		}
		return false;
	}


	inline bool ClassdefTable::nameLookupForClassdef(Classdef& cdef)
	{
		bool success = true;

		bool hasSelf = cdef.interface.hasSelf();
		if (hasSelf)
			success &= nameLookupForSelfInterface(cdef);

		if (cdef.followup.empty() and cdef.interface.empty())
		{
			success &= (likely(cdef.isAny() or cdef.isLinkedToAtom() or cdef.isBuiltinOrVoid()));
		}
		else
		{
			if (not cdef.interface.empty() and hasSelf)
			{
				auto& funcdefSelf = cdef.interface.self();
				if (funcdefSelf.isFullyResolved())
				{
					Atom* parentAtom = funcdefSelf.atom;

					if (parentAtom->type == Atom::Type::funcdef)
					{
						cdef.interface.eachUnresolved([&](Funcdef& funcdef)
						{
							if (funcdef.name == "^()")
							{
								funcdef.atom = parentAtom;
								funcdef.clid = CLID::AtomMapID(parentAtom->atomid);
							}
							else
								success = false;
						});
					}
					else
					{
						cdef.interface.eachUnresolved([&](Funcdef& funcdef)
						{
							bool found = parentAtom->nameLookupOnChildren(funcdef.overloads.getList(), funcdef.name);
							if (found)
							{
								if (funcdef.overloads.size() == 1)
								{
									Atom& resolvedAtom = funcdef.overloads.getList()[0].get();
									funcdef.overloads.clear();
									funcdef.clid = CLID::AtomMapID(resolvedAtom.atomid);
									funcdef.atom = &resolvedAtom;
								}
							}
							else
								success = false;
						});
					}
				}
			}
		}

		return success;
	}


	Atom* ClassdefTable::findRawClassdefAtom(const Classdef& cdef) const
	{
		Atom* result = cdef.hasAtom() ? cdef.atom : nullptr;

		if (nullptr == result and not cdef.followup.extends.empty())
		{
			Atom* followAtom = nullptr;
			for (auto& clid: cdef.followup.extends)
			{
				auto& followup = rawclassdef(clid);
				if (followup.hasAtom())
				{
					if (unlikely(followAtom != nullptr))
						return nullptr;
					followAtom = followup.atom;
				}
			}
			result = followAtom;
		}
		return result;
	}


	Atom* ClassdefTable::findClassdefAtom(const Classdef& cdef) const
	{
		Atom* result = cdef.hasAtom() ? cdef.atom : nullptr;

		if (nullptr == result and not cdef.followup.extends.empty())
		{
			Atom* followAtom = nullptr;
			for (auto& clid: cdef.followup.extends)
			{
				auto& followup = classdef(clid);
				if (followup.hasAtom())
				{
					if (unlikely(followAtom != nullptr))
						return nullptr;
					followAtom = followup.atom;
				}
			}
			result = followAtom;
		}
		return result;
	}


	bool ClassdefTable::performNameLookup() // Logs::Report& report)
	{
		// list of errors
		std::vector<std::reference_wrapper<Classdef>> remainings;

		for (auto& pair: pClassdefs)
		{
			// atomid 0 is reserved to the real atoms created from the blueprints opcodes
			// TODO use an alternate (and more efficient) container for the special cdefs atomid=0
			if (unlikely(pair.first.atomid() == 0))
				continue;

			auto& cdef = *(pair.second);
			bool isAlias = (cdef.clid != pair.first);

			if (not isAlias) // do not handle aliases
			{
				bool success = nameLookupForClassdef(cdef);
				if (not success and Config::Traces::allTypeDefinitions)
					remainings.push_back(cdef);
			}
		}

		/*
		if (Config::Traces::allTypeDefinitions)
		{
			auto trace = report.trace();
			print(trace.text(), false);
			trace << "\n\n\n ==== remaining (" << remainings.size() << " elements) ===\n";

			if (remainings.empty())
			{
				trace << '\n';
			}
			else
			{
				for (auto& refwrapper: remainings)
					printClassdef(trace.text(), refwrapper.get().clid, refwrapper.get());
			}
			report.info(); // for beauty
			report.info(); // for beauty
		}
		*/

		// some symbol may not be resolved but it may not be a real problem
		// (for some code not instanciated for example)
		return true; // remainings.empty();
	}


	bool ClassdefTable::hasSubstitute(CLID clid) const
	{
		if (hasClassdef(clid))
			clid = classdef(clid).clid; // take into consideration symlinks

		if (clid.atomid() == layer.atomid)
		{
			assert(clid.lvid() < layer.count);
			return (layer.flags[clid.lvid()]);
		}
		return false;
	}


	void ClassdefTable::substituteResize(uint32_t count)
	{
		uint32_t previous = layer.count;
		if (count <= previous)
			return;

		layer.count = count;
		layer.flags.resize(count);
		for (uint32_t i = previous; i != count; ++i)
			layer.flags[i] = false; // true;

		layer.storage.reserve(count);
		assert(layer.storage.size() <= count);
		for (uint32_t i = static_cast<uint32_t>(layer.storage.size()); i != count; ++i)
			layer.storage.emplace_back(CLID{layer.atomid, i});

		// checking for missing classdef
		// functions are sometimes generating on the fly (ctor, clone...)
		for (uint32_t i = previous; i != count; ++i)
		{
			CLID clid{layer.atomid, i};
			if (not hasClassdef(clid))
				pClassdefs[clid] = new Classdef(clid); // any with local replacement
		}
	}


	void ClassdefTable::mergeSubstitutes()
	{
		auto atomid = layer.atomid;
		if (unlikely(atomid == (LVID) -1))
			throw "invalid atom id for merging substitutions";

		for (uint32_t i = 0; i != layer.count; ++i)
		{
			if (layer.flags[i])
				pClassdefs[CLID{atomid, i}] = new Classdef(layer.storage[i]);
		}

		// invalidate the current layer
		layer.atomid = static_cast<uint32_t>(-1);
	}


	Classdef& ClassdefTable::substitute(LVID lvid) const
	{
		assert(lvid < layer.count);
		if (not layer.flags[lvid])
		{
			layer.flags[lvid] = true;
			auto& newcdef = layer.storage[lvid];

			// preserve qualifiers
			auto it = pClassdefs.find(CLID{layer.atomid, lvid});
			if (it != pClassdefs.end())
				newcdef.qualifiers = (*(it->second)).qualifiers;

			// set clid
			newcdef.clid.reclass(layer.atomid, lvid);
			return newcdef;
		}
		return layer.storage[lvid];
	}


	Classdef& ClassdefTable::addSubstitute(nytype_t kind, Atom* atom, const Qualifiers& qualifiers) const
	{
		// atom can be null
		layer.flags.push_back(true);
		layer.storage.emplace_back();
		++layer.count;
		assert(layer.count == layer.flags.size());
		assert(layer.count == layer.storage.size());

		auto& ret = layer.storage.back();
		switch (kind)
		{
			case nyt_any:  ret.mutateToAtom(atom); break;
			case nyt_void: ret.mutateToVoid(); break;
			default:       ret.mutateToBuiltin(kind);
		}

		// preserve qualifiers
		ret.qualifiers = qualifiers;
		// set clid
		ret.clid.reclass(layer.atomid, layer.count - 1);
		return ret;
	}


	AnyString ClassdefTable::keyword(const Atom& atom) const
	{
		switch (atom.type)
		{
			default: return atom.keyword();

			// for vardef, something more subtle can be done
			case Atom::Type::vardef:
			{
				// the clid might be invalid in development and this function is used for debugging
				// however, to indicate an issue, no valid identifier will be returned
				if (likely(not atom.returnType.clid.isVoid()))
				{
					if (hasClassdef(atom.returnType.clid)) // to be fault-tolerant
					{
						auto& qualifiers = classdef(atom.returnType.clid).qualifiers;
						if (qualifiers.constant)
						{
							if (qualifiers.ref)
								return "cref";
							return "const";
						}
						if (qualifiers.ref)
							return "ref";
						return "var";
					}
					return "var{invalid-clid}";
				}

				return "var[no-return-type]";
			}
		}
		return "auto";
	}



} // namespace Nany
