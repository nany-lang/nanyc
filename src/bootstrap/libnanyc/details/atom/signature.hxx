#pragma once
#include "signature.h"



namespace ny
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


	inline bool Signature::operator == (const Signature& rhs) const
	{
		return (parameters == rhs.parameters and tmplparams == rhs.tmplparams);
	}



} // namespace ny


namespace std
{
	template<>
	struct hash<ny::Signature>
	{
		using argument_type = ny::Signature;
		using result_type   = std::size_t;

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
	class Append<YString, ny::Signature> final
	{
	public:
		static void Perform(String& out, const ny::Signature& rhs);
	};


} // namespace CString
} // namespace Extension
} // namespace Yuni



// for std::cout
std::ostream& operator << (std::ostream& out, const ny::Signature& rhs);
