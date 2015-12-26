#pragma once
#include <yuni/yuni.h>
#include <yuni/core/bind.h>
#include "details/intrinsic/intrinsic.h"
#include <unordered_map>
#include <vector>
#include <memory>



namespace Nany
{

	class IntrinsicTable final
	{
	public:
		//! Default constructor
		IntrinsicTable();
		//! Copy constructor
		IntrinsicTable(const IntrinsicTable&) = default;
		//! Destructor
		~IntrinsicTable() = default;

		/*!
		** \brief Add a new intrinsic
		*/
		template<class T> bool add(const AnyString& name, T callback);

		/*!
		** \brief Get if an intrinsic exists
		*/
		bool exists(const AnyString& name) const;

		//! Get if empty
		bool empty() const;


		/*!
		** \brief Find an intrinsic by its name
		*/
		const Intrinsic* find(const AnyString& name) const;


		//! Register defaults
		void registerStdCore();



	private:
		//! Register all intrinsics related to 'bool'
		void registerBool();

	private:
		//! All intrinsics
		std::vector<Intrinsic::Ptr> pIntrinsics;
		//! All intrinsics, ordered by their name
		std::unordered_map<AnyString, std::reference_wrapper<Intrinsic>> pByNames;

	}; // class IntrinsicTable





} // namespace Nany

#include "intrinsic-table.hxx"
