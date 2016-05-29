#pragma once
#include "classdef.h"



namespace Nany
{

	inline bool Classdef::isBuiltin() const
	{
		return (kind != nyt_void) and (kind != nyt_any);
	}


	inline bool Classdef::isBuiltinOrVoid() const
	{
		return kind != nyt_any;
	}

	inline bool Classdef::isRawPointer() const
	{
		return kind == nyt_ptr and (atom == nullptr);
	}

	inline bool Classdef::isBuiltinU64() const
	{
		return kind == nyt_u64;
	}

	inline bool Classdef::isBuiltingUnsigned() const
	{
		switch (kind)
		{
			case nyt_u8:
			case nyt_u16:
			case nyt_u32:
			case nyt_u64: return true;
			default: return false;
		}
	}


	inline bool Classdef::isVoid() const
	{
		return kind == nyt_void;
	}


	inline bool Classdef::isLinkedToAtom() const
	{
		assert((nullptr == atom or (kind == nyt_any))
			and "the kind of a classdef must be 'any' if an atom is provided");
		return nullptr != atom;
	}


	inline bool Classdef::isAny() const
	{
		return (nullptr == atom) and (kind == nyt_any);
	}


	inline bool Classdef::hasAtom() const
	{
		return nullptr != atom;
	}


	inline bool Classdef::isVariable() const
	{
		return instance;
	}


	inline void Classdef::mutateToVoid()
	{
		kind = nyt_void;
		atom = nullptr;
	}


	inline void Classdef::mutateToBuiltin(nytype_t newkind)
	{
		assert(newkind != nyt_void and newkind != nyt_any);
		kind = newkind;
		atom = nullptr;
	}


	inline void Classdef::mutateToAny()
	{
		kind = nyt_any;
		atom = nullptr;
	}


	inline void Classdef::mutateToAtom(Atom* newAtom)
	{
		kind = nyt_any;
		atom = newAtom;
	}


	inline void Classdef::mutateToPtr2Func(Atom* newAtom)
	{
		kind = nyt_ptr;
		atom = newAtom;
	}



	inline bool Classdef::hasConstraints() const
	{
		return not isAny() or not interface.empty() or not followup.empty(); // TODO qualifiers ???
	}


	inline void Classdef::import(const Classdef& rhs)
	{
		kind = rhs.kind;
		atom = rhs.atom;

		parentclid = rhs.parentclid;
		instance   = rhs.instance;
		interface  = rhs.interface;  // mandatory to preserve constraints
		followup   = rhs.followup;   // same here
		origins    = rhs.origins;

		// qualifiers should be preserved
		// qualifiers = rhs.qualifiers;
	}


	inline YString Classdef::print(const ClassdefTableView& table) const
	{
		YString out;
		print(out, table);
		return out;
	}




} // namespace Nany
