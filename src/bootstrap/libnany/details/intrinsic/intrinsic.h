#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include "nany/nany.h"




namespace Nany
{

	class Intrinsic final
		: public Yuni::IIntrusiveSmartPtr<Intrinsic, false, Yuni::Policy::SingleThreaded>
		, Yuni::NonCopyable<Intrinsic>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<Intrinsic, false, Yuni::Policy::SingleThreaded>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<Intrinsic>::Ptr  Ptr;


	public:
		static constexpr uint maxParameterCount = 12;


	public:
		Intrinsic(const AnyString& name)
			: name(name)
		{}

		Intrinsic(const Intrinsic&) = default;


	public:
		//! name of the intrinsic
		const Yuni::ShortString64 name;

		//! C-Callback
		void* callback = nullptr;

		//! The return type
		nytype_t rettype = nyt_void;

		//! The total number of parameters
		uint paramcount = 0;
		//! All parameter types
		std::array<nytype_t, maxParameterCount> params;


	}; // class Intrinsic




} // namespace Nany
