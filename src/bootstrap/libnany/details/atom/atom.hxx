#pragma once
#include "atom.h"



namespace Nany
{

	inline bool Atom::canCaptureVariabes() const
	{
		return flags.get(Flags::captureVariables)
			or (isClassMember() and parent->flags.get(Flags::captureVariables));
	}


	inline Atom* Atom::createDummy()
	{
		return new Atom("", Type::classdef);
	}


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

	inline bool Atom::isUnit() const
	{
		return type == Type::unit;
	}

	inline bool Atom::isTypeAlias() const
	{
		return type == Type::typealias;
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
		return type == Type::vardef and parent and (parent->type == Type::classdef);
	}

	inline bool Atom::isClassMember() const
	{
		return (type != Type::classdef) and parent and (parent->type == Type::classdef);
	}


	inline bool Atom::isPublicOrPublished() const
	{
		return (visibility == nyv_public) or (visibility == nyv_published);
	}


	inline bool Atom::hasReturnType() const
	{
		return not returnType.clid.isVoid();
	}


	inline bool Atom::hasGenericParameters() const
	{
		return not tmplparams.empty();
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


	inline void Atom::Parameters::swap(Parameters& rhs)
	{
		std::swap(pData, rhs.pData);
		std::swap(shortcircuitValue, rhs.shortcircuitValue);
	}


	template<class T>
	inline void Atom::Parameters::each(const T& callback) const
	{
		if (!!pData)
		{
			uint32_t count = pData->count;
			auto& params = pData->params;
			for (uint32_t i = 0; i != count; ++i)
			{
				auto& pair = params[i];
				callback(i, pair.first, pair.second);
			}
		}
	}

	inline uint Atom::Parameters::size() const
	{
		return (!!pData) ? pData.get()->count : 0;
	}

	inline bool Atom::Parameters::empty() const
	{
		return !pData;
	}

	inline bool Atom::Parameters::append(const CLID& clid, const AnyString& name)
	{
		if (!pData)
			pData = std::make_unique<Data>();
		auto& internal = *pData.get();

		if (likely(internal.count < Config::maxPushedParameters))
		{
			internal.params[internal.count].first  = name;
			internal.params[internal.count].second.clid = clid;
			++internal.count;
			return true;
		}
		return false;
	}


	inline yuint32 Atom::Parameters::findByName(const AnyString& name, yuint32 offset) const
	{
		if (!!pData)
		{
			auto& internal = *pData.get();
			uint32_t count = internal.count;
			for (uint32_t i = offset; i < count; ++i)
			{
				if (name == internal.params[i].first)
					return i;
			}
		}
		return static_cast<yuint32>(-1);
	}


	inline const AnyString& Atom::Parameters::name(uint index) const
	{
		assert(!!pData and index < pData->count);
		return pData->params[index].first;
	}

	inline const Vardef& Atom::Parameters::vardef(uint index) const
	{
		assert(!!pData and index < pData->count);
		return pData->params[index].second;
	}


	inline const std::pair<AnyString, Vardef>& Atom::Parameters::operator [] (uint index) const
	{
		assert(!!pData and index < pData->count);
		return pData->params[index];
	}


	inline const IR::Sequence& Atom::instance(uint32_t instanceid) const
	{
		assert(instanceid < instances.size());
		return *(instances[instanceid].get());
	}


	inline const IR::Sequence* Atom::fetchInstance(uint32_t instanceid) const
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


	inline uint32_t Atom::size() const
	{
		return (uint32_t) pChildren.size();
	}


	inline bool Atom::empty() const
	{
		return pChildren.empty();
	}


	inline uint32_t Atom::findInstance(IR::Sequence*& sequence, Atom*& remapAtom, Signature& signature)
	{
		auto it = pInstancesBySign.find(signature);
		if (it != pInstancesBySign.end())
		{
			auto& metadata = it->second;
			sequence  = metadata.sequence;
			remapAtom = metadata.remapAtom;

			auto& storedRetType = it->first.returnType;
			signature.returnType.import(storedRetType);
			signature.returnType.qualifiers = storedRetType.qualifiers;
			return metadata.instanceid;
		}
		return (uint32_t) -1;
	}





} // namespace Nany
