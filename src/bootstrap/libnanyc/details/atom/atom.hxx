#pragma once
#include "atom.h"



namespace Nany
{

	inline Atom* Atom::parentScope()
	{
		return (!scopeForNameResolution) ? parent : scopeForNameResolution;
	}

	inline const Atom* Atom::parentScope() const
	{
		return (!scopeForNameResolution) ? parent : scopeForNameResolution;
	}


	inline AnyString Atom::name() const
	{
		return pName;
	}


	inline bool Atom::canCaptureVariabes() const
	{
		return flags.has(Flags::captureVariables)
			or (isClassMember() and parent->flags.has(Flags::captureVariables));
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

	inline bool Atom::isAnonymousClass() const
	{
		return type == Type::classdef and pName.empty();
	}

	inline bool Atom::isCtor() const
	{
		return category(Category::ctor);
	}

	inline bool Atom::isDtor() const
	{
		return category(Category::dtor);
	}

	inline bool Atom::isCloneCtor() const
	{
		return category(Category::clone);
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
		return category(Category::funcoperator);
	}

	inline bool Atom::isSpecial() const
	{
		return category(Category::special);
	}

	inline bool Atom::isView() const
	{
		return category(Category::view);
	}

	inline bool Atom::isFunctor() const
	{
		return category(Category::functor);
	}

	inline bool Atom::isCapturedVariable() const
	{
		return category(Category::capturedVar);
	}

	inline bool Atom::isMemberVarDefaultInit() const
	{
		return category(Category::defvarInit);
	}

	inline bool Atom::isMemberVariable() const
	{
		return (type == Type::vardef) and category(Category::classParent);
	}

	inline bool Atom::isClassMember() const
	{
		return (type != Type::classdef) and category(Category::classParent);
	}

	inline bool Atom::isPropertyGet() const
	{
		return category(Category::propget);
	}

	inline bool Atom::isPropertySet() const
	{
		return category(Category::propset);
	}

	inline bool Atom::isPropertySetCustom() const
	{
		return category(Category::propsetCustom);
	}

	inline bool Atom::isProperty() const
	{
		return isPropertyGet() or isPropertySet() or isPropertySetCustom();
	}

	inline bool Atom::isUnittest() const
	{
		return category(Category::unittest);
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

	inline bool Atom::isContextual() const
	{
		return hasGenericParameters() // generic classes
			or pName.empty() // anonymous classes
			or (parent and parent->isFunction()); // anything inside a func which may depend from parameters
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
			case Type::funcdef: return category(Category::functor);
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
			? AnyString{pInstancesMD[instanceid].symbol} : AnyString{};
	}


	inline void Atom::updateInstance(uint32_t id, Yuni::String& symbol, const Classdef& rettype)
	{
		auto& md = pInstancesMD[id];
		md.rettype.import(rettype);
		md.rettype.qualifiers = rettype.qualifiers;
		md.symbol.swap(symbol);
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




} // namespace Nany
