#pragma once
#include "clid.h"
#include <iosfwd>



namespace Nany
{

	inline CLID CLID::AtomMapID(uint32_t atomid)
	{
		assert(atomid != 0);
		return CLID{atomid, 0};
	}


	inline CLID::CLID(uint32_t atomid, uint32_t lvid)
		: m_data{{atomid, lvid}}
	{
		assert(lvid < 1000000); // arbitrary integrity check
	}


	inline bool CLID::isVoid() const
	{
		return !m_data.u64;
	}


	inline void CLID::reclass(uint32_t lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		m_data.u32[1] = lvid;
	}


	inline void CLID::reclass(uint32_t atomid, uint32_t lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		m_data.u32[0] = atomid;
		m_data.u32[1] = lvid;
	}


	inline void CLID::reclassToVoid()
	{
		m_data.u64 = 0;
	}


	inline bool CLID::operator == (const CLID& rhs) const
	{
		return m_data.u64 == rhs.m_data.u64;
	}


	inline bool CLID::operator != (const CLID& rhs) const
	{
		return m_data.u64 != rhs.m_data.u64;
	}


	inline bool CLID::operator <  (const CLID& rhs) const
	{
		return m_data.u64 < rhs.m_data.u64;
	}


	inline size_t CLID::hash() const
	{
		return std::hash<uint64_t>()(m_data.u64);
	}


	inline uint32_t CLID::atomid() const
	{
		return m_data.u32[0];
	}


	inline uint32_t CLID::lvid() const
	{
		return m_data.u32[1];
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
