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
	void Append<YString, ny::Signature>::Perform(String& out, const ny::Signature& rhs)
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
					out << ny::CLID::AtomMapID(tmplparam.atom->atomid);
				else
					out << nytype_to_cstring(tmplparam.kind);
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
				out << ny::CLID::AtomMapID(param.atom->atomid);
			else
				out << nytype_to_cstring(param.kind);
		}
		out << ')';
	}


} // namespace CString
} // namespace Extension
} // namespace Yuni




namespace ny
{


	void Signature::Parameters::resize(uint count)
	{
		m_types.clear();
		m_types.resize(count);
		/*m_types.reserve(count);
		for (uint i = 0; i != count; ++i)
			m_types.emplace_back(std::make_unique<Paramtype>());
		m_types.shrink_to_fit();*/
	}


	inline void Signature::Parameters::hash(size_t& seed) const
	{
		uint count = (uint) m_types.size();
		for (uint i = 0; i != count; ++i)
		{
			auto& element = m_types[i];

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
		uint32_t count = (uint32_t) m_types.size();
		if (count == (uint32_t) rhs.m_types.size())
		{
			for (uint32_t i = 0; i != count; ++i)
			{
				if (m_types[i] != rhs.m_types[i])
					return false;
			}
		}
		return true;
	}


} // namespace ny






std::ostream& operator << (std::ostream& out, const ny::Signature& rhs)
{
	String s;
	s << rhs;
	out << s;
	return out;
}
