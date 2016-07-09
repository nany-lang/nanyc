#pragma once
#include <yuni/yuni.h>
#include <yuni/core/tribool.h>




namespace Nany
{


	class Qualifiers final
	{
	public:
		struct Default final
		{
			//! Constant by default ?
			static constexpr bool constant = false;
			//! nullable by default ?
			static constexpr bool nullable = false;
			//! reference by default ?
			static constexpr bool ref = true;
		};


	public:
		//! Get if constant
		Yuni::Tribool constant = Default::constant;
		//! By reference, and not by copy
		Yuni::Tribool ref = Default::ref;
		//! Nullable, can be null ?
		Yuni::Tribool nullable = Default::nullable;

		/*!
		** \brief Merge with another qualifiers
		*/
		void merge(const Qualifiers&);

		/*!
		** \brief Clear
		*/
		void clear();


		//! \name Operators
		//@{
		//! Comparison
		bool operator == (const Qualifiers&) const;
		//! Not equal
		bool operator != (const Qualifiers&) const;
		//@}

	}; // class Qualifiers









} // namespace Nany

#include "qualifiers.hxx"
