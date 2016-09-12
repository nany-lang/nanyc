#pragma once
#include "clid.h"



namespace Nany
{

	inline CLID CLID::AtomMapID(uint32_t atomid)
	{
		assert(atomid != 0);
		return CLID{atomid, 0};
	}


	inline CLID::CLID(uint32_t atomid, uint32_t lvid)
		: m_atomid(atomid)
		, m_lvid(lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
	}


	inline bool CLID::isVoid() const
	{
		return !m_atomid and !m_lvid;
	}


	inline void CLID::reclass(uint32_t lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		m_lvid = lvid;
	}


	inline void CLID::reclass(uint32_t atomid, uint32_t lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		m_atomid = atomid;
		m_lvid = lvid;
	}


	inline void CLID::reclassToVoid()
	{
		m_atomid = 0;
		m_lvid = 0;
	}


	inline bool CLID::operator == (const CLID& rhs) const
	{
		return m_atomid == rhs.m_atomid and m_lvid == rhs.m_lvid;
	}


	inline bool CLID::operator != (const CLID& rhs) const
	{
		return m_atomid != rhs.m_atomid or m_lvid != rhs.m_lvid;
	}


	inline bool CLID::operator <  (const CLID& rhs) const
	{
		return m_atomid < rhs.m_atomid and m_lvid < rhs.m_lvid;
	}


	inline size_t CLID::hash() const
	{
		return std::hash<uint32_t>()(m_atomid) xor std::hash<uint32_t>()(m_lvid);
	}


	inline uint32_t CLID::atomid() const
	{
		return m_atomid;
	}


	inline uint32_t CLID::lvid() const
	{
		return m_lvid;
	}




} // namespace Nany




namespace std
{
	template<> struct hash<Nany::CLID> final
	{
		inline size_t operator() (const Nany::CLID& clid) const
		{
			return clid.hash();
		}
	};

} // namespace std




namespace Yuni
{
namespace Extension
{
namespace CString
{

	template<class CStringT>
	class Append<CStringT, Nany::CLID> final
	{
	public:
		static void Perform(CStringT& string, const Nany::CLID& rhs)
		{
			string << '{' << rhs.atomid() << ':' << rhs.lvid() << '}';
		}
	};


} // namespace CString
} // namespace Extension
} // namespace Yuni



inline std::ostream& operator << (std::ostream& out, const Nany::CLID& rhs)
{
	out << '{' << rhs.atomid() << ':' << rhs.lvid() << '}';
	return out;
}

