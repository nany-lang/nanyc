#pragma once
#include "funcdef.h"



namespace ny
{

	inline Funcdef::Funcdef(const AnyString& name)
		: name(name)
	{}


	inline bool Funcdef::hasReturnType() const
	{
		return not rettype.isVoid();
	}


	inline bool Funcdef::hasParameters() const
	{
		return not parameters.empty();
	}


	inline void Funcdef::resetReturnTypeToVoid()
	{
		rettype.reclassToVoid();
	}


	inline void Funcdef::resetReturnType(const CLID& clid)
	{
		rettype = clid;
	}


	inline void Funcdef::appendParameter(const CLID& clid)
	{
		assert(not clid.isVoid() and "invalid classdef id");
		parameters.push_back(std::make_pair(AnyString(), clid));
	}


	inline void Funcdef::appendParameter(const AnyString& name, const CLID& clid)
	{
		assert(not clid.isVoid() and "invalid classdef id");
		parameters.push_back(std::make_pair(name, clid));
	}


	inline bool Funcdef::isOperator() const
	{
		assert(not name.empty() and "invalid funcdef name");
		return (not name.empty()) and name[0] == '@';
	}

	inline bool Funcdef::isFunc() const
	{
		assert(not name.empty() and "invalid funcdef name");
		return (not name.empty()) and name[0] != '@';
	}


	inline bool Funcdef::isPartiallyResolved() const
	{
		return (atom != nullptr) or (not overloads.empty());
	}


	inline bool Funcdef::isFullyResolved() const
	{
		return (atom != nullptr);
	}




} // namespace ny
