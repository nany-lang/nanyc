#pragma once
#include "classdef-table.h"




namespace ny
{

	inline void ClassdefTable::LayerItem::swap(ClassdefTable::LayerItem& rhs)
	{
		std::swap(atomid,   rhs.atomid);
		std::swap(flags,    rhs.flags);
		std::swap(storage,  rhs.storage);
		std::swap(count,    rhs.count);
	}


	inline bool ClassdefTable::hasClassdef(const CLID& clid) const
	{
		return (clid.atomid() == m_layer.atomid)
			? (clid.lvid() < m_layer.count and m_layer.flags[clid.lvid()])
			: (0 != m_classdefs.count(clid));
	}


	inline Funcdef& ClassdefTable::addClassdefInterface(const Classdef& classdef, const AnyString& name)
	{
		// the classdef may actually be an alias
		// as a consequence we have to perform another lookup
		return addClassdefInterface(classdef.clid, name);
	}

	inline Funcdef& ClassdefTable::addClassdefInterfaceSelf(const Classdef& classdef, const AnyString& name)
	{
		// the classdef may actually be an alias
		// as a consequence we have to perform another lookup
		return addClassdefInterfaceSelf(classdef.clid, name);
	}


	inline Funcdef& ClassdefTable::addClassdefInterfaceSelf(const CLID& clid)
	{
		return addClassdefInterfaceSelf(clid, nullptr);
	}


	inline Funcdef& ClassdefTable::addClassdefInterfaceSelf(const Classdef& classdef)
	{
		return addClassdefInterfaceSelf(classdef.clid, nullptr);
	}


	inline LVID ClassdefTable::substituteAtomID() const
	{
		return m_layer.atomid;
	}



} // namespace ny
