#pragma once
#include "stack-frame.h"





namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	inline void AtomStackFrame::LVIDInfo::fillLogEntryWithLocation(Logs::Report& entry) const
	{
		auto& origins = entry.origins();
		origins.location.pos.line   = file.line;
		origins.location.pos.offset = file.offset;
		origins.location.filename   = file.url;
	}


	inline AtomStackFrame::AtomStackFrame(Atom& atom)
		: atom(atom)
		, atomid(atom.atomid)
	{
		lvids.resize(atom.localVariablesCount);
	}


	inline uint32_t AtomStackFrame::localVariablesCount() const
	{
		return static_cast<yuint32>(lvids.size());
	}


	inline uint32_t AtomStackFrame::findLocalVariable(const AnyString& name) const
	{
		for (uint i = (uint) lvids.size(); i--; )
		{
			if (lvids[i].userDefinedName == name)
				return i;
		}
		return 0;
	}


	inline void AtomStackFrame::resizeRegisterCount(uint32_t count, ClassdefTableView& table)
	{
		if (count >= lvids.size())
			lvids.resize(count);
		table.substituteResize(count);
	}


	inline bool AtomStackFrame::verify(uint32_t lvid) const
	{
		if (not (lvid != 0 and lvid < lvids.size()))
			throw "piko";
		assert(lvid != 0 and lvid < lvids.size());
		return likely(not lvids[lvid].errorReported);
	}


	inline void AtomStackFrame::invalidate(uint32_t lvid)
	{
		assert(lvid != 0 and lvid < lvids.size());
		lvids[lvid].errorReported = true;
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
