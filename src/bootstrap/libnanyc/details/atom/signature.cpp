#include "signature.h"
#include "details/atom/atom.h"
#include "nany/nany.h"
#include <yuni/core/stl/hash-combine.h>

using namespace Yuni;





namespace Yuni
{
namespace Extension
{
namespace CString
{

	template<>
	void Append<YString, Nany::Signature>::Perform(String& out, const Nany::Signature& rhs)
	{
		out << "signature";

		if (not rhs.tmplparams.empty())
		{
			out << "<:";
			for (uint i = 0; i != rhs.tmplparams.size(); ++i)
			{
				if (i != 0)
					out.write(", ", 2);

				auto& tmplparam = rhs.tmplparams[i];
				if (tmplparam.atom and tmplparam.kind == nyt_any)
					out << Nany::CLID::AtomMapID(tmplparam.atom->atomid);
				else
					out << nany_type_to_cstring(tmplparam.kind);
			}
			out << ":>";
		}

		out << '(';
		for (uint i = 0; i != rhs.parameters.size(); ++i)
		{
			if (i != 0)
				out.write(", ", 2);

			auto& param = rhs.parameters[i];
			if (param.atom and param.kind == nyt_any)
				out << Nany::CLID::AtomMapID(param.atom->atomid);
			else
				out << nany_type_to_cstring(param.kind);
		}
		out << ')';
	}


} // namespace CString
} // namespace Extension
} // namespace Yuni





namespace Nany
{


	void Signature::Parameters::resize(uint count)
	{
		pParamtypes.clear();
		pParamtypes.resize(count);
		/*pParamtypes.reserve(count);
		for (uint i = 0; i != count; ++i)
			pParamtypes.emplace_back(std::make_unique<Paramtype>());
		pParamtypes.shrink_to_fit();*/
	}


	inline void Signature::Parameters::hash(size_t& seed) const
	{
		uint count = (uint) pParamtypes.size();
		for (uint i = 0; i != count; ++i)
		{
			auto& element = pParamtypes[i];

			Yuni::HashCombine(seed, reinterpret_cast<uint64_t>(element.atom));
			Yuni::HashCombine(seed, static_cast<uint>(element.kind));

			Yuni::HashCombine(seed, static_cast<uint8_t>(element.qualifiers.ref));
			Yuni::HashCombine(seed, static_cast<uint8_t>(element.qualifiers.constant));
			Yuni::HashCombine(seed, static_cast<uint8_t>(element.qualifiers.nullable));
		}
	}


	size_t Signature::hash() const
	{
		size_t seed = 0;
		assert(static_cast<size_t>(Config::maxPushedParameters) > parameters.size());
		assert(static_cast<size_t>(Config::maxPushedParameters) > tmplparams.size());

		parameters.hash(seed);
		if (not tmplparams.empty())
		{
			// adding an arbitrary value, to avoid collision some other functios
			// with the same set of parameter types
			Yuni::HashCombine(seed, static_cast<uint8_t>(1));

			tmplparams.hash(seed);
		}
		return seed;
	}


	bool Signature::Parameters::operator == (const Signature::Parameters& rhs) const
	{
		uint32_t count = (uint32_t) pParamtypes.size();
		if (count == (uint32_t) rhs.pParamtypes.size())
		{
			for (uint32_t i = 0; i != count; ++i)
			{
				if (pParamtypes[i] != rhs.pParamtypes[i])
					return false;
			}
		}
		return true;
	}



} // namespace Nany






std::ostream& operator << (std::ostream& out, const Nany::Signature& rhs)
{
	String s;
	s << rhs;
	out << s;
	return out;
}
