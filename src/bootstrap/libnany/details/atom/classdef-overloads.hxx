#pragma once
#include "classdef-overloads.h"



namespace Nany
{

	inline bool ClassdefOverloads::empty() const
	{
		return pOverloads.empty();
	}


	inline uint ClassdefOverloads::size() const
	{
		return (uint) pOverloads.size();
	}


	inline void ClassdefOverloads::clear()
	{
		pOverloads.clear();
	}


	inline std::vector<std::reference_wrapper<Atom>>&  ClassdefOverloads::getList()
	{
		return pOverloads;
	}

	inline const std::vector<std::reference_wrapper<Atom>>&  ClassdefOverloads::getList() const
	{
		return pOverloads;
	}




} // namespace Nany
