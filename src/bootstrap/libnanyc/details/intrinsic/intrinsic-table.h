#pragma once
#include <yuni/yuni.h>
#include <yuni/core/bind.h>
#include "details/intrinsic/intrinsic.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdarg.h>



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

		//! Add a new intrinsic
		template<class T> bool add(const AnyString& name, T callback);

		bool add(const AnyString& name, void* callback, nytype_t ret, va_list argp);

		//! Get if an intrinsic exists
		bool exists(const AnyString& name) const;

		//! Get if empty
		bool empty() const;

		//! The total number of user-defined intrinsics
		uint32_t size() const;

		//! Find an intrinsic by its name
		Intrinsic::Ptr find(const AnyString& name) const;

		//! Get the intrinsic for a given intrinsic id
		const Intrinsic& operator [] (uint32_t id) const;


	private:
		//! All intrinsics
		std::vector<Intrinsic::Ptr> pIntrinsics;
		//! All intrinsics, ordered by their name
		std::unordered_map<AnyString, Intrinsic::Ptr> pByNames;

	}; // class IntrinsicTable





} // namespace Nany

#include "intrinsic-table.hxx"
