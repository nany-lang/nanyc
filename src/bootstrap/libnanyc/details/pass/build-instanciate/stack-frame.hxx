#pragma once
#include "stack-frame.h"





namespace ny
{
namespace Pass
{
namespace Instanciate
{


	inline void LVIDInfo::fillLogEntryWithLocation(Logs::Report& entry) const
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
		return static_cast<uint32_t>(lvids.size());
	}


	inline uint32_t AtomStackFrame::findLocalVariable(const AnyString& name) const
	{
		uint32_t count = static_cast<uint32_t>(lvids.size());
		for (uint32_t i = count; i--; )
		{
			if (lvids[i].userDefinedName == name)
				return i;
		}
		return 0u;
	}


	inline void AtomStackFrame::resizeRegisterCount(uint32_t count, ClassdefTableView& table)
	{
		if (count >= lvids.size())
			lvids.resize(count);
		table.substituteResize(count);
	}


	inline bool AtomStackFrame::verify(uint32_t lvid) const
	{
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
} // namespace ny
