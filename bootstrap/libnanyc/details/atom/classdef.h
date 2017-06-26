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
#include "details/reporting/report.h"
#include "details/atom/ctype.h"


namespace ny {


struct Atom;


//! Class definition
struct Classdef final
	: public Yuni::IIntrusiveSmartPtr<Classdef, false, Yuni::Policy::SingleThreaded> {
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
	//! Get if the classdef is a builtin pointer
	bool isRawPointer() const;
	//! Get if the classdef is a builtin u64
	bool isBuiltinU64() const;
	//! Get if the classdef is a builtin u32
	bool isBuiltinU32() const;
	//! Get if the classdef is a builtin u32
	bool isBuiltinU8() const;
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

	//! Get if is a variable
	bool isVariable() const;
	//! Get if the classdef has a linked atom
	bool hasAtom() const;

	//! Get if the classdef has some constraints
	bool hasConstraints() const;
	//@}

	//! \name Mutations
	//@{
	//! Mutate the defintiion to 'void'
	void mutateToVoid();

	//! Mutate the type to builtin (not void)
	void mutateToBuiltin(CType);

	//! Mutate the type to builtin (or void)
	void mutateToBuiltinOrVoid(CType);

	//! Mutate the type to any
	void mutateToAny();

	//! Mutate to a well-known type (from atom)
	void mutateToAtom(Atom*);

	//! Mutate to a ptr-2-func/method
	void mutateToPtr2Func(Atom*);

	//! Import from another classdef
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
	//! Inner builtin type (custom type if == CType::t_any)
	CType kind = CType::t_void;
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

	struct {
		uint line = 0;
		uint offset = 0;
		//! filename (acquired by the classdef-table)
		const char* filename = nullptr;
	}
	origins;


private:
	template<class T, class TableT> void doPrint(T& out, const TableT& table) const;

}; // struct Classdef


} // namespace ny

#include "classdef.hxx"
