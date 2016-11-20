#pragma once
#include "qualifiers.h"



namespace ny
{

	inline void Qualifiers::merge(const Qualifiers& rhs)
	{
		if (rhs.nullable)
			nullable = true;

		if (rhs.constant)
			constant = true;

		if (rhs.ref)
			ref = true;
	}


	inline bool Qualifiers::operator == (const Qualifiers& rhs) const
	{
		// ignoring propset, since only a metadata for property instanciation
		return (constant == rhs.constant) and (ref == rhs.ref) and (nullable == rhs.nullable);
	}

	inline bool Qualifiers::operator != (const Qualifiers& rhs) const
	{
		return not (operator == (rhs));
	}


	inline void Qualifiers::clear()
	{
		constant.clear();
		ref.clear();
		nullable.clear();
	}




} // namespace ny
