#pragma once
#include <yuni/yuni.h>
#include <vector>
#include "details/fwd.h"



namespace ny
{

	class ClassdefOverloads final
	{
	public:
		//! Default constructor
		ClassdefOverloads() = default;


		bool empty() const;

		uint size() const;

		void print(Yuni::String& out, const ClassdefTableView& table, bool clearBefore = true) const;

		void clear();

		std::vector<std::reference_wrapper<Atom>>&  getList();
		const std::vector<std::reference_wrapper<Atom>>&  getList() const;


	private:
		//! Follow some indexed parameter (atomid/parameter index type)
		std::vector<std::reference_wrapper<Atom>> m_overloads;

	}; // class ClassdefFollow






} // namespace ny

#include "classdef-overloads.hxx"
