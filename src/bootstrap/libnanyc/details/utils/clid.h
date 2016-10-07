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
		static CLID AtomMapID(uint32_t atomid);

	public:
		//! Default constructor
		CLID() = default;
		//! Default constructor with initial value
		CLID(uint32_t atomid, uint32_t lvid);
		//! Copy constructor
		CLID(const CLID&) = default;

		//! Get if the clid is valid (aka != 0)
		bool isVoid() const;

		//! update the class id from lvid only
		void reclass(uint32_t lvid);
		//! update the class id from {atom id, lvid}
		void reclass(uint32_t atomid, uint32_t lvid);
		//! update to 'void'
		void reclassToVoid();

		//! Get the atom id of the clid
		uint32_t atomid() const;
		//! Get the lvid part
		uint32_t lvid() const;

		//! hash of the clid
		size_t hash() const;

		//! Assignment operator
		CLID& operator = (const CLID&) = default;
		//! Equal operator
		bool operator == (const CLID&) const;
		//! Not equal operator
		bool operator != (const CLID&) const;
		//! Comparison operator
		bool operator <  (const CLID&) const;


	private:
		union { uint32_t u32[2]; uint64_t u64; } m_data = {{0,0}};

	}; // class CLID




} // namespace Nany




std::ostream& operator << (std::ostream& out, const Nany::CLID& rhs);

#include "clid.hxx"
