#pragma once
#include "clid.h"



namespace Nany
{

	inline CLID CLID::AtomMapID(yuint32 atomid)
	{
		assert(atomid != 0);
		return CLID{atomid, 0};
	}



	inline CLID::CLID()
	{
		data.u64 = 0;
	}


	inline CLID::CLID(yuint32 atomid, LVID lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		data.u32[0] = lvid;
		data.u32[1] = atomid;
	}


	inline CLID::CLID(const CLID& rhs)
		: data(rhs.data)
	{}


	inline bool CLID::isVoid() const
	{
		return data.u64 == 0;
	}


	inline void CLID::reclass(LVID lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		data.u32[0] = lvid;
	}


	inline void CLID::reclass(yuint32 atomid, LVID lvid)
	{
		assert(lvid < 1000000); // arbitrary integrity check
		data.u32[0] = lvid;
		data.u32[1] = atomid;
	}


	inline void CLID::reclassToVoid()
	{
		data.u64 = 0;
	}


	inline CLID& CLID::operator = (const CLID& rhs)
	{
		data = rhs.data;
		return *this;
	}


	inline bool CLID::operator == (const CLID& rhs) const
	{
		return data.u64 == rhs.data.u64;
	}


	inline bool CLID::operator != (const CLID& rhs) const
	{
		return data.u64 != rhs.data.u64;
	}


	inline bool CLID::operator <  (const CLID& rhs) const
	{
		return data.u64 < rhs.data.u64;
	}


	inline size_t CLID::hash() const
	{
		return static_cast<size_t>(data.u64);
	}


	inline yuint32 CLID::atomid() const
	{
		return data.u32[1];
	}


	inline LVID CLID::lvid() const
	{
		return data.u32[0];
	}




} // namespace Nany




namespace std
{
	template<> struct hash<Nany::CLID> final
	{
		size_t operator() (const Nany::CLID& clid) const
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
			string << "{" << rhs.atomid() << ':' << rhs.lvid() << '}';
		}
	};


} // namespace CString
} // namespace Extension
} // namespace Yuni



inline std::ostream& operator << (std::ostream& out, const Nany::CLID& rhs)
{
	out << "{" << rhs.atomid() << ':' << rhs.lvid() << '}';
	return out;
}

