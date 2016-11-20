#pragma once
#include "classdef-overloads.h"



namespace ny
{

	inline bool ClassdefOverloads::empty() const
	{
		return m_overloads.empty();
	}


	inline uint ClassdefOverloads::size() const
	{
		return (uint) m_overloads.size();
	}


	inline void ClassdefOverloads::clear()
	{
		m_overloads.clear();
	}


	inline std::vector<std::reference_wrapper<Atom>>&  ClassdefOverloads::getList()
	{
		return m_overloads;
	}

	inline const std::vector<std::reference_wrapper<Atom>>&  ClassdefOverloads::getList() const
	{
		return m_overloads;
	}




} // namespace ny
