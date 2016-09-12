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
		#if !LIBYUNI_CLID_UNION
		: m_atomid(atomid)
		, m_lvid(lvid)
		#else
		: m_data{{atomid, lvid}}
		#endif
	{
		assert(lvid < 1000000); // arbitrary integrity check
	}


	inline bool CLID::isVoid() const
	{
		#if !LIBYUNI_CLID_UNION
		return !m_atomid and !m_lvid;
		#else
		return !m_data.u64;
		#endif
	}


	inline void CLID::reclass(uint32_t lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		#if !LIBYUNI_CLID_UNION
		m_lvid = lvid;
		#else
		m_data.u32[1] = lvid;
		#endif
	}


	inline void CLID::reclass(uint32_t atomid, uint32_t lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		#if !LIBYUNI_CLID_UNION
		m_atomid = atomid;
		m_lvid = lvid;
		#else
		m_data.u32[0] = atomid;
		m_data.u32[1] = lvid;
		#endif
	}


	inline void CLID::reclassToVoid()
	{
		#if !LIBYUNI_CLID_UNION
		m_atomid = 0;
		m_lvid = 0;
		#else
		m_data.u64 = 0;
		#endif
	}


	inline bool CLID::operator == (const CLID& rhs) const
	{
		#if !LIBYUNI_CLID_UNION
		return m_atomid == rhs.m_atomid and m_lvid == rhs.m_lvid;
		#else
		return m_data.u64 == rhs.m_data.u64;
		#endif
	}


	inline bool CLID::operator != (const CLID& rhs) const
	{
		#if !LIBYUNI_CLID_UNION
		return m_atomid != rhs.m_atomid or m_lvid != rhs.m_lvid;
		#else
		return m_data.u64 != rhs.m_data.u64;
		#endif
	}


	inline bool CLID::operator <  (const CLID& rhs) const
	{
		#if !LIBYUNI_CLID_UNION
		return m_atomid < rhs.m_atomid and m_lvid < rhs.m_lvid;
		#else
		return m_data.u64 < rhs.m_data.u64;
		#endif
	}


	inline size_t CLID::hash() const
	{
		#if !LIBYUNI_CLID_UNION
		return std::hash<uint32_t>()(m_atomid) xor std::hash<uint32_t>()(m_lvid);
		#else
		return std::hash<uint64_t>()(m_data.u64);
		#endif
	}


	inline uint32_t CLID::atomid() const
	{
		#if !LIBYUNI_CLID_UNION
		return m_atomid;
		#else
		return m_data.u32[0];
		#endif
	}


	inline uint32_t CLID::lvid() const
	{
		#if !LIBYUNI_CLID_UNION
		return m_lvid;
		#else
		return m_data.u32[1];
		#endif
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

