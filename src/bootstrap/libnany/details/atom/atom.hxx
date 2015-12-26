#pragma once
#include "atom.h"



namespace Nany
{

	inline CLID Atom::clid() const
	{
		return CLID::AtomMapID(atomid);
	}


	inline bool Atom::isNamespace() const
	{
		return type == Type::namespacedef;
	}

	inline bool Atom::isClass() const
	{
		return type == Type::classdef;
	}

	inline bool Atom::isFunction() const
	{
		return type == Type::funcdef;
	}

	inline bool Atom::isOperator() const
	{
		return type == Type::funcdef and name[0] == '^';
	}

	inline bool Atom::isMemberVarDefaultInit() const
	{
		return type == Type::funcdef and name[0] == '^'
			and name.startsWith("^default-var-%");
	}


	inline bool Atom::isMemberVariable() const
	{
		return type == Type::vardef;
	}

	inline bool Atom::isClassMember() const
	{
		return (type != Type::classdef) and (parent != nullptr) and (parent->type == Type::classdef);
	}


	inline bool Atom::isPublicOrPublished() const
	{
		return (visibility == nyv_public) or (visibility == nyv_published);
	}


	inline bool Atom::hasReturnType() const
	{
		return not returnType.clid.isVoid();
	}


	template<class C>
	inline void Atom::eachChild(const C& callback)
	{
		for (auto& pair: pChildren)
		{
			if (not callback(*pair.second))
				return;
		}
	}

	template<class C>
	inline void Atom::eachChild(const C& callback) const
	{
		for (auto& pair: pChildren)
		{
			if (not callback(*pair.second))
				return;
		}
	}


	inline bool Atom::hasMember(const AnyString& name) const
	{
		switch (type)
		{
			case Type::funcdef: return (name == "^()");
			default:            return (pChildren.count(name) != 0);
		}
		return false;
	}


	template<class C>
	inline void Atom::eachChild(const AnyString& needle, const C& callback)
	{
		assert(not needle.empty());
		auto range = pChildren.equal_range(needle);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (not callback(*(it->second)))
				return;
		}
	}

	template<class C>
	inline void Atom::eachChild(const AnyString& needle, const C& callback) const
	{
		assert(not needle.empty());
		auto range = pChildren.equal_range(needle);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (not callback(*(it->second)))
				return;
		}
	}


	inline uint Atom::Parameters::size() const
	{
		return pCount;
	}


	inline bool Atom::Parameters::empty() const
	{
		return pCount == 0;
	}


	inline bool Atom::Parameters::append(const CLID& clid, const AnyString& name)
	{
		if (pCount < Config::maxPushedParameters)
		{
			pParams[pCount].first  = name;
			pParams[pCount].second.clid = clid;
			++pCount;
			return true;
		}
		return false;
	}


	inline yuint32 Atom::Parameters::findByName(const AnyString& name, yuint32 offset) const
	{
		for (uint i = offset; i < pCount; ++i)
		{
			if (name == pParams[i].first)
				return i;
		}
		return static_cast<yuint32>(-1);
	}


	inline const AnyString& Atom::Parameters::name(uint index) const
	{
		assert(index < pCount);
		return pParams[index].first;
	}

	inline const Vardef& Atom::Parameters::vardef(uint index) const
	{
		assert(index < pCount);
		return pParams[index].second;
	}


	inline const std::pair<AnyString, Vardef>& Atom::Parameters::operator [] (uint index) const
	{
		assert(index < pCount);
		return pParams[index];
	}


	inline const IR::Program& Atom::instance(uint32_t instanceid) const
	{
		assert(instanceid < instances.size());
		return *(instances[instanceid].get());
	}


	inline const IR::Program* Atom::fetchInstance(uint32_t instanceid) const
	{
		return (instanceid < instances.size()) ? instances[instanceid].get() : nullptr;
	}


	inline AnyString Atom::fetchInstanceCaption(uint32_t instanceid) const
	{
		return (instanceid < instances.size())
			? AnyString{pSymbolInstances[instanceid]} : AnyString{};
	}


	inline uint64_t Atom::runtimeSizeof() const
	{
		return classinfo.nextFieldIndex * sizeof(uint64_t);
	}


	inline YString Atom::printFullname() const
	{
		YString out;
		printFullname(out);
		return out;
	}


} // namespace Nany
