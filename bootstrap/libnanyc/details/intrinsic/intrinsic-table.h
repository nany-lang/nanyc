#pragma once
#include "libnanyc.h"
#include <yuni/core/bind.h>
#include "details/intrinsic/intrinsic.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdarg.h>


namespace ny {


struct IntrinsicTable final {
	//! Default constructor
	IntrinsicTable();
	//! Copy constructor
	IntrinsicTable(const IntrinsicTable&) = default;

	//! Add a new intrinsic
	template<class T> bool add(const AnyString& name, T callback);

	//! Get if an intrinsic exists
	bool exists(const AnyString& name) const;
	//! Get if empty
	bool empty() const;
	//! The total number of user-defined intrinsics
	uint32_t size() const;

	//! Find an intrinsic by its name
	yuni::Ref<Intrinsic> find(const AnyString& name) const;

	//! Get the intrinsic for a given intrinsic id
	const Intrinsic& operator [] (uint32_t id) const;


private:
	//! All intrinsics
	std::vector<yuni::Ref<Intrinsic>> m_intrinsics;
	//! All intrinsics, ordered by their name
	std::unordered_map<AnyString, yuni::Ref<Intrinsic>> m_names;

}; // class IntrinsicTable


} // namespace ny

#include "intrinsic-table.hxx"
