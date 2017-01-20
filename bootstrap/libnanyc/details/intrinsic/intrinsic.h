#pragma once
#include "libnanyc.h"
#include <yuni/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include "nany/nany.h"
#include "libnanyc-config.h"
#include <array>




namespace ny {

/*!
** \brief Definition of a single user-defined intrinsic
*/
struct Intrinsic final
	: public Yuni::IIntrusiveSmartPtr<Intrinsic, false, Yuni::Policy::SingleThreaded>
	, Yuni::NonCopyable<Intrinsic> {
public:
	Intrinsic(void* callback): callback(callback) {}
	Intrinsic(const Intrinsic&) = default;
	Intrinsic& operator = (const Intrinsic&) = delete;


public:
	//! C-Callback
	void* callback = nullptr;
	//! name of the intrinsic
	const yuni::CString<40,false> name;
	//! The return type
	nytype_t rettype = nyt_void;
	//! The total number of parameters
	uint32_t paramcount = 0;
	//! All parameter types
	std::array<nytype_t, config::maxPushedParameters> params;
	//! Intrinsic ID
	uint32_t id = (uint32_t) - 1;

}; // struct Intrinsic


} // namespace ny
