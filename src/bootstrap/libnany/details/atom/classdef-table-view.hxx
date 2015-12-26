#pragma once
#include "classdef-table-view.h"



namespace Nany
{

	inline ClassdefTableView::ClassdefTableView(ClassdefTable& table)
		: table(table)
	{}


	inline ClassdefTableView::ClassdefTableView(ClassdefTableView& view)
		: table(view.table)
	{}


	inline ClassdefTableView::ClassdefTableView(ClassdefTableView& view, LVID atomid, uint count)
		: table(view.table)
	{
		previous.atomid = atomid;

		// reminder: parameters are 2-based
		// 0 -> invalid
		// 1 -> return value / type
		// 2 -> first parameter (or first register if no parameter)
		previous.flags.reserve(count + 2); // parameters are 2-base
		previous.storage.reserve((count + 2) * 2); // arbitrary, but at least 'count+2'

		previous.swap(table.layer);
		canSwap = true;
	}


	inline ClassdefTableView::~ClassdefTableView()
	{
		if (canSwap)
			table.layer.swap(previous);
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


	inline Match ClassdefTableView::isSimilarTo(const CTarget* target, const Classdef& cdef, const Classdef& to, bool allowImplicit) const
	{
		return table.isSimilarTo(target, cdef, to, allowImplicit);
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


	inline Classdef& ClassdefTableView::substitute(LVID lvid)
	{
		return table.substitute(lvid);
	}


	inline Classdef& ClassdefTableView::addSubstitute(nytype_t kind, Atom* atom, const Qualifiers& qualifiers)
	{
		return table.addSubstitute(kind, atom, qualifiers);
	}


	inline LVID ClassdefTableView::substituteAtomID() const
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


	inline void ClassdefTableView::mergeSubstitutes()
	{
		table.mergeSubstitutes();
	}



} // namespace Nany
