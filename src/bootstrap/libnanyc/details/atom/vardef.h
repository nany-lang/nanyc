#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/utils/clid.h"
#include "type.h"




namespace Nany
{


	class Vardef final : public Yuni::NonCopyable<Vardef>
	{
	public:
		//! \name Constructors
		//@{
		//! Default constructor
		Vardef() = default;
		//! Move constructor
		Vardef(Vardef&&);
		//@}


		//! \name Operators
		//@{
		//! Copy operator
		Vardef& operator = (const Vardef&) = default;
		//! Move operator
		Vardef& operator = (Vardef&&) = default;
		//@}


	public:
		//! Attached type (class id)
		// (this value may be changed)
		CLID clid;

	}; // class Vardef








} // namespace Nany

#include "vardef.hxx"

