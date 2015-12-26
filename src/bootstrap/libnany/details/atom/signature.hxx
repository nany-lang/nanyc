#pragma once
#include "signature.h"



namespace Nany
{

	inline uint Signature::Parameters::size() const
	{
		return (uint) pParamtypes.size();
	}


	inline bool Signature::Parameters::empty() const
	{
		return pParamtypes.empty();
	}


	inline Signature::Paramtype& Signature::Parameters::operator [] (uint index)
	{
		assert(index < pParamtypes.size());
		return pParamtypes[index];
	}


	inline const Signature::Paramtype& Signature::Parameters::operator [] (uint index) const
	{
		assert(index < pParamtypes.size());
		return pParamtypes[index];
	}


	inline bool Signature::operator != (const Signature& rhs) const
	{
		return not (*this == rhs);
	}


	inline bool Signature::Paramtype::operator == (const Signature::Paramtype& rhs) const
	{
		return (kind == rhs.kind and atom == rhs.atom and qualifiers == rhs.qualifiers);
	}


	inline bool Signature::Paramtype::operator != (const Signature::Paramtype& rhs) const
	{
		return not (operator == (rhs));
	}




} // namespace Nany


namespace std
{
	template<>
	struct hash<Nany::Signature>
	{
		typedef Nany::Signature argument_type;
		typedef std::size_t result_type;

		result_type operator() (const argument_type& signature) const
		{
			return signature.hash();
		}
	};

} // namespace std


namespace Yuni
{
namespace Extension
{
namespace CString
{

	template<class YString>
	class Append<YString, Nany::Signature> final
	{
	public:
		static void Perform(String& out, const Nany::Signature& rhs);
	};


} // namespace CString
} // namespace Extension
} // namespace Yuni



// for std::cout
std::ostream& operator << (std::ostream& out, const Nany::Signature& rhs);
