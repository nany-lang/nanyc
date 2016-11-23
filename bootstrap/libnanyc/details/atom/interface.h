#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "type.h"
#include "funcdef.h"


namespace ny {


class ClassdefInterface final {
public:
	/*!
	** \brief Get if the container is empty
	*/
	bool empty() const;

	/*!
	** \brief Empty the interface
	*/
	void clear();

	bool hasSelf() const;

	Funcdef& self();
	const Funcdef& self() const;

	//! Print
	void print(Yuni::String& out, bool clearBefore = true) const;

	template<class C> void eachUnresolved(const C& callback);


private:
	// \brief Add a new funcdef as part as the interface of the current classdef
	void add(Funcdef* funcdef);

private:
	//! Interface restrictions
	std::vector<Funcdef::Ptr> pInterface;
	//! Self
	Funcdef::Ptr pSelf;
	friend class ClassdefTable;

}; // class ClassdefInterface


} // namespace ny

#include "interface.hxx"
