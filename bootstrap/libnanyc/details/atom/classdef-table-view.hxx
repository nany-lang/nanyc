#pragma once
#include "classdef-table-view.h"



namespace ny
{

	inline ClassdefTableView::ClassdefTableView(ClassdefTable& table)
		: table(table)
	{}


	inline ClassdefTableView::ClassdefTableView(ClassdefTableView& view)
		: table(view.table)
	{}


	inline ClassdefTableView::ClassdefTableView(ClassdefTableView& view, uint32_t atomid, uint count)
		: table(view.table)
	{
		// preparing the new layer via the local data, then swapping
		previous.atomid = atomid;

		// reminder: parameters are 2-based
		// 0 -> invalid
		// 1 -> return value / type
		// 2 -> first parameter (or first register if no parameter)
		previous.flags.reserve(count + 2); // parameters are 2-base
		previous.storage.reserve((count + 2) * 2); // arbitrary, but at least 'count+2'

		// swap the layer
		previous.swap(table.m_layer);
		canSwap = true;
	}


	inline ClassdefTableView::~ClassdefTableView()
	{
		if (canSwap)
			table.m_layer.swap(previous);
	}


	inline AnyString ClassdefTableView::keyword(const Atom& atom) const
	{
		return table.keyword(atom);
	}


	inline const Classdef& ClassdefTableView::classdefFollowClassMember(const CLID& clid) const
	{
		return table.classdefFollowClassMember(clid);
	}

	inline bool ClassdefTableView::hasClassdef(const CLID& clid) const
	{
		return table.hasClassdef(clid);
	}


	inline Atom* ClassdefTableView::findClassdefAtom(const Classdef& cdef) const
	{
		return table.findClassdefAtom(cdef);
	}

	inline Atom* ClassdefTableView::findRawClassdefAtom(const Classdef& cdef) const
	{
		return table.findRawClassdefAtom(cdef);
	}


	inline const Classdef& ClassdefTableView::classdef(const CLID& clid) const
	{
		return table.classdef(clid);
	}


	inline const Classdef& ClassdefTableView::rawclassdef(const CLID& clid) const
	{
		return table.rawclassdef(clid);
	}


	inline bool ClassdefTableView::hasSubstitute(const CLID& clid) const
	{
		return table.hasSubstitute(clid);
	}


	inline Classdef& ClassdefTableView::substitute(uint32_t lvid)
	{
		return table.substitute(lvid);
	}



	inline Classdef& ClassdefTableView::addSubstitute(nytype_t kind, Atom* atom, const Qualifiers& qualifiers)
	{
		return table.addSubstitute(kind, atom, qualifiers);
	}


	inline uint32_t ClassdefTableView::substituteAtomID() const
	{
		return table.substituteAtomID();
	}


	inline void ClassdefTableView::substituteResize(uint count)
	{
		table.substituteResize(count);
	}


	inline const AtomMap& ClassdefTableView::atoms() const
	{
		return table.atoms;
	}

	inline AtomMap& ClassdefTableView::atoms()
	{
		return table.atoms;
	}


	inline ClassdefTable& ClassdefTableView::originalTable()
	{
		return table;
	}


	inline const ClassdefTable& ClassdefTableView::originalTable() const
	{
		return table;
	}



} // namespace ny
