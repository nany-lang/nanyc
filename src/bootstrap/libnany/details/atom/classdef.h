#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include <yuni/core/smartptr/intrusive.h>
#include "details/ir/fwd.h"
#include "details/utils/clid.h"
#include "nany/nany.h"
#include "type.h"
#include <vector>
#include "qualifiers.h"
#include "interface.h"
#include "funcdef.h"
#include "classdef-follow.h"




namespace Nany
{

	/*!
	** \brief Class definition
	*/
	class Classdef final
		: public Yuni::IIntrusiveSmartPtr<Classdef, false, Yuni::Policy::SingleThreaded>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<Classdef, false, Yuni::Policy::SingleThreaded>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<Classdef>::Ptr  Ptr;

		//! Null cdef
		static const Classdef nullcdef;


	public:
		//! \name Constructor
		//@{
		//! Default constructor
		Classdef();
		//! Copy constructor
		Classdef(const Classdef&) = default;
		//! Default constructor with a given clid
		explicit Classdef(const CLID&);
		//@}


		//! \name Queries
		//@{
		//! Get if the classdef is a builtin type (not 'void')
		bool isBuiltin() const;
		//! Get if the classdef is a builtin type ('void' included)
		bool isBuiltinOrVoid() const;

		//! Get if the classdef is a builtin unsigned int (u32, u64...)
		bool isBuiltingUnsigned() const;

		//! Get if the classdef is void
		bool isVoid() const;
		//! Get if the type is defined by an atom (an 'atom' is provided)
		bool isLinkedToAtom() const;
		//! Get if the type is 'any'
		bool isAny() const;
		//! Get if the type is a class
		bool isClass() const;
		//! Get if the type is a given class
		bool isClass(const AnyString& name) const;
		//! Get if the type is a function
		bool isPointerToFunc() const;
		//! Get if the type is a pointer to member
		bool isPointerToMember() const;

		//! Get if is a variable
		bool isVariable() const;
		//! Get if the classdef has a linked atom
		bool hasAtom() const;

		//! Get if the classdef has some constraints
		bool hasConstraints() const;
		//@}


		//! \name Mutations
		//@{
		/*!
		** \brief Mutate the defintiion to 'void'
		*/
		void mutateToVoid();

		/*!
		** \brief Mutate the type to builtin (not void)
		*/
		void mutateToBuiltin(nytype_t);

		/*!
		** \brief Mutate the type to any
		*/
		void mutateToAny();

		/*!
		** \brief Mutate to a well-known type (from atom)
		*/
		void mutateToAtom(Atom*);

		/*!
		** \brief Mutate to a ptr-2-func/method
		*/
		void mutateToPtr2Func(Atom*);

		/*!
		** \brief Import from another classdef
		*/
		void import(const Classdef& rhs);
		//@}


		//! \name Info
		//@{
		/*!
		** \brief Export the classdef to a human readable string
		**
		** \param[out] out The string where to append the data
		*/
		void print(Yuni::String& out, const ClassdefTableView& table, bool clearBefore = true) const;
		YString print(const ClassdefTableView& table) const;
		void print(Logs::Report&, const ClassdefTableView& table) const;
		//@}


		//! \name Operators
		//@{
		//! Comparison
		bool operator == (const Classdef&) const;
		//! Not equal
		bool operator != (const Classdef&) const;
		//! Assignment
		Classdef& operator = (const Classdef&) = delete;
		//@}


	public:
		//! Inner builtin type (custom type if == nyt_any)
		nytype_t kind = nyt_void;
		//! Atom
		Atom* atom = nullptr;
		//! Classdef ID
		CLID clid;
		//! Parent class ID
		CLID parentclid;
		//! Has a value ? (is a variable)
		bool instance = false;

		//! All qualifiers (can be shared between several classdef)
		Qualifiers qualifiers;
		//! Interface
		ClassdefInterface interface;
		//! Other calls to follow
		ClassdefFollow followup;

		struct
		{
			uint line = 0;
			uint offset = 0;
			//! filename (acquired by the classdef-table)
			const char* filename = nullptr;
		}
		origins;


	private:
		template<class T, class TableT> void doPrint(T& out, const TableT& table) const;

	}; // class Classdef






} // namespace Nany

#include "classdef.hxx"
