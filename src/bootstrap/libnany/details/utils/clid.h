#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/fwd.h"
#include <functional> // std::hash




namespace Nany
{

	class CLID final
	{
	public:
		//! Create a CLID for an atom id
		static CLID AtomMapID(yuint32 atomid);

	public:
		//! Default constructor
		CLID();
		//! Default constructor with initial value
		CLID(yuint32 atomid, LVID lvid);
		//! Copy constructor
		CLID(const CLID&);

		//! Get if the clid is valid (aka != 0)
		bool isVoid() const;

		//! update the class id from lvid only
		void reclass(LVID lvid);
		//! update the class id from {atom id, lvid}
		void reclass(yuint32 atomid, LVID lvid);
		//! update to 'void'
		void reclassToVoid();

		//! Get the atom id of the clid
		yuint32 atomid() const;
		//! Get the lvid part
		LVID lvid() const;


		//! hash of the clid
		size_t hash() const;

		//! Assignment operator
		CLID& operator = (const CLID&);
		//! Equal operator
		bool operator == (const CLID&) const;
		//! Not equal operator
		bool operator != (const CLID&) const;
		//! Comparison operator
		bool operator <  (const CLID&) const;


	private:
		union { yuint32 u32[2]; yuint64 u64; } data;

	}; // class CLID




} // namespace Nany




inline std::ostream& operator << (std::ostream& out, const Nany::CLID& rhs);

#include "clid.hxx"
