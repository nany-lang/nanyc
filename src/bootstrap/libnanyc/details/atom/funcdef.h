#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include "details/utils/clid.h"
#include "type.h"
#include <vector>
#include "classdef-overloads.h"



namespace Nany
{

	/*!
	** \brief Function definition
	*/
	class Funcdef final
		: public Yuni::IIntrusiveSmartPtr<Funcdef, false, Yuni::Policy::SingleThreaded>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<Funcdef, false, Yuni::Policy::SingleThreaded>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<Funcdef>::Ptr  Ptr;

	public:
		//! \name Queries
		//@{
		//! Get if the funcdef has parameters
		bool hasParameters() const;
		//! Get if the funcdef has a return type ('void' otherwise)
		bool hasReturnType() const;

		//! Is operator ?
		bool isOperator() const;
		//! Is a function ?
		bool isFunc() const;
		//@}

		//! \name Utilities
		//@{
		//! Reset the return type
		void resetReturnType(const CLID&);
		//! Reset the return type to void
		void resetReturnTypeToVoid();

		//! Append an unamed parameter
		void appendParameter(const CLID&);
		//! Append a named parameter
		void appendParameter(const AnyString& name, const CLID&);
		//@}


		//! \name Name resolution
		//@{
		//! Get if the funcdef is resolved or has overloads
		bool isPartiallyResolved() const;
		//! Get if the funcdef is fully resolved
		bool isFullyResolved() const;
		//@}


		//! \name Debugging
		//@{
		//! Export the func definition
		void print(Yuni::String& out, bool clearBefore = true) const;
		//@}


	public:
		//! Name of the func or the operator
		const AnyString name;
		//! clid
		CLID clid;
		//! Return type (null means void)
		CLID rettype;
		//! parameters (name if name parameter, type)
		std::vector<std::pair<AnyString, CLID>> parameters;

		Atom* atom = nullptr;
		//! Overloads
		ClassdefOverloads overloads;


	private:
		friend class ClassdefTable;
		//! Default constructor
		explicit Funcdef(const AnyString& name);

	}; // class Funcdef








} // namespace Nany

#include "funcdef.hxx"
