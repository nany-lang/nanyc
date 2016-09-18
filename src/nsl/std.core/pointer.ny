// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// \file    pointer.ny
// \brief   Raw pointer
// \ingroup std.core




/*!
** \brief Wraps a raw pointer
*/
class pointer<:T:>
{
	typedef ValueType: T;

	operator new;
	operator new(p: __pointer)
	{
		m_ptr = p;
	}

	operator new(p: __pointer, offset)
	{
		m_ptr = !!add(p, offset * !!sizeof(#[__nanyc_synthetic] T));
	}

	operator new(ref object: T)
	{
		m_ptr = !!pointer(object);
	}

	operator new(ref object: T)
	{
		m_ptr = !!add(!!pointer(object), offset * !!sizeof(#[__nanyc_synthetic] T));
	}

	//! Get if the pointer is null
	func isNull: ref bool
		-> new bool(m_ptr == null);

	//! Reset the pointer to null
	func clear
	{
		m_ptr = null;
	}

	//! Get the object referenced by the pointer
	func deref: ref T
	{
		assert(m_ptr != null);
		return !!__reinterpret(m_ptr, #[__nanyc_synthetic] T);
	}


	func move(to: __pointer)
	{
		m_ptr = to;
	}

	func move(cref to: T)
	{
		m_ptr = !!pointer(to);
	}

	func move(cref to)
	{
		m_ptr = to.m_ptr;
	}


	var addressof
		-> new u64(!!__reinterpret(m_ptr, #[__nanyc_synthetic] __u64));


	operator += (offset): ref
	{
		m_ptr = !!add(m_ptr, offset * !!sizeof(#[__nanyc_synthetic] T));
		return self;
	}

	operator -= (offset): ref
	{
		m_ptr = !!sub(m_ptr, offset * !!sizeof(#[__nanyc_synthetic] T));
		return self;
	}

	operator *= (offset): ref
	{
		m_ptr = !!mul(m_ptr, offset * !!sizeof(T));
		return self;
	}

	operator /= (offset): ref
	{
		m_ptr = !!div(m_ptr, offset * !!sizeof(T));
		return self;
	}


internal:
	var m_ptr: __pointer = null;

} // class pointer





#[__nanyc_builtinalias: not, nosuggest]
public operator not (a: __pointer): __bool;

public operator not (cref p: pointer): bool
	-> not p.m_ptr;


#[__nanyc_builtinalias: add] public operator + (a: __pointer, b: cref u64): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __pointer, b: __u64): __pointer;
#[__nanyc_builtinalias: add] public operator + (a: __pointer, b: cref u32): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __pointer, b: __u32): __pointer;
#[__nanyc_builtinalias: add] public operator + (a: __pointer, b: cref u16): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __pointer, b: __u16): __pointer;
#[__nanyc_builtinalias: add] public operator + (a: __pointer, b: cref u8): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __pointer, b: __u8): __pointer;

#[__nanyc_builtinalias: add] public operator + (a: cref u64, b: __pointer): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u64, b: __pointer): __pointer;
#[__nanyc_builtinalias: add] public operator + (a: cref u32, b: __pointer): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: __pointer): __pointer;
#[__nanyc_builtinalias: add] public operator + (a: cref u16, b: __pointer): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u16, b: __pointer): __pointer;
#[__nanyc_builtinalias: add] public operator + (a: cref u8, b: __pointer): __pointer;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u8, b: __pointer): __pointer;

#[__nanyc_builtinalias: sub] public operator - (a: __pointer, b: cref u64): __pointer;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __pointer, b: __u64): __pointer;
#[__nanyc_builtinalias: sub] public operator - (a: __pointer, b: cref u32): __pointer;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __pointer, b: __u32): __pointer;
#[__nanyc_builtinalias: sub] public operator - (a: __pointer, b: cref u16): __pointer;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __pointer, b: __u16): __pointer;
#[__nanyc_builtinalias: sub] public operator - (a: __pointer, b: cref u8): __pointer;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __pointer, b: __u8): __pointer;



#[__nanyc_builtinalias: gt]public operator > (a: cref pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: cref pointer, b: __pointer): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __pointer, b: __pointer): __bool;

#[__nanyc_builtinalias: gte] public operator >= (a: cref pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: cref pointer, b: __pointer): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __pointer, b: __pointer): __bool;

#[__nanyc_builtinalias: lt] public operator < (a: cref pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: cref pointer, b: __pointer): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __pointer, b: __pointer): __bool;

#[__nanyc_builtinalias: lte] public operator <= (a: cref pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: cref pointer, b: __pointer): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __pointer, b: __pointer): __bool;



#[__nanyc_builtinalias: eq] public operator == (a: cref pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref pointer, b: __pointer): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __pointer, b: __pointer): __bool;

#[__nanyc_builtinalias: neq] public operator != (a: cref pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref pointer, b: __pointer): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __pointer, b: cref pointer): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __pointer, b: __pointer): __bool;






// -*- mode: nany;-*-
// vim: set filetype=nany:
