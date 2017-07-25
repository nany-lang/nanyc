// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \file    u32.ny
/// \brief   Implementation of the class u32, Unsigned integer with width of exactly 32 bits
/// \ingroup std.core

/// \important THIS FILE IS AUTOMATICALLY GENERATED

/// \brief   Unsigned integer with width of exactly 32 bits
/// \ingroup std.core
public class u32 {
	operator new;

	operator new (cref x: u32) {
		pod = x.pod;
	}

	operator new (cref x: u16) {
		pod = x.pod;
	}

	operator new (cref x: u8) {
		pod = x.pod;
	}

	#[nosuggest] operator new (self pod: __u32);

	#[nosuggest] operator new (self pod: __u16);

	#[nosuggest] operator new (self pod: __u8);

	operator ++self: ref u32 {
		pod = !!inc(pod);
		return self;
	}

	operator self++: ref u32 {
		ref tmp = new u32(pod);
		pod = !!inc(pod);
		return tmp;
	}

	operator --self: ref u32 {
		pod = !!dec(pod);
		return self;
	}

	operator self--: ref u32 {
		ref tmp = new u32(pod);
		pod = !!dec(pod);
		return tmp;
	}

	operator += (cref x: u32): ref u32 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __u32): ref u32 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: u16): ref u32 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __u16): ref u32 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: u8): ref u32 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __u8): ref u32 {
		pod = !!add(pod, x);
		return self;
	}

	operator -= (cref x: u32): ref u32 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __u32): ref u32 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: u16): ref u32 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __u16): ref u32 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: u8): ref u32 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __u8): ref u32 {
		pod = !!sub(pod, x);
		return self;
	}

	operator *= (cref x: u32): ref u32 {
		pod = !!mul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __u32): ref u32 {
		pod = !!mul(pod, x);
		return self;
	}

	operator *= (cref x: u16): ref u32 {
		pod = !!mul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __u16): ref u32 {
		pod = !!mul(pod, x);
		return self;
	}

	operator *= (cref x: u8): ref u32 {
		pod = !!mul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __u8): ref u32 {
		pod = !!mul(pod, x);
		return self;
	}

	operator /= (cref x: u32): ref u32 {
		pod = !!div(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __u32): ref u32 {
		pod = !!div(pod, x);
		return self;
	}

	operator /= (cref x: u16): ref u32 {
		pod = !!div(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __u16): ref u32 {
		pod = !!div(pod, x);
		return self;
	}

	operator /= (cref x: u8): ref u32 {
		pod = !!div(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __u8): ref u32 {
		pod = !!div(pod, x);
		return self;
	}

private:
	var pod = 0__u32;

} // class u32




#[__nanyc_builtinalias: gt] public operator > (a: cref u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: cref u32, b: __u64): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: __u64): __bool;
#[__nanyc_builtinalias: gt] public operator > (a: cref u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: cref u32, b: __u32): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: __u32): __bool;
#[__nanyc_builtinalias: gt] public operator > (a: cref u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: cref u32, b: __u16): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: __u16): __bool;
#[__nanyc_builtinalias: gt] public operator > (a: cref u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: cref u32, b: __u8): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __u32, b: __u8): __bool;

#[__nanyc_builtinalias: gte] public operator >= (a: cref u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: cref u32, b: __u64): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: __u64): __bool;
#[__nanyc_builtinalias: gte] public operator >= (a: cref u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: cref u32, b: __u32): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: __u32): __bool;
#[__nanyc_builtinalias: gte] public operator >= (a: cref u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: cref u32, b: __u16): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: __u16): __bool;
#[__nanyc_builtinalias: gte] public operator >= (a: cref u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: cref u32, b: __u8): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __u32, b: __u8): __bool;

#[__nanyc_builtinalias: lt] public operator < (a: cref u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: cref u32, b: __u64): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: __u64): __bool;
#[__nanyc_builtinalias: lt] public operator < (a: cref u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: cref u32, b: __u32): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: __u32): __bool;
#[__nanyc_builtinalias: lt] public operator < (a: cref u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: cref u32, b: __u16): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: __u16): __bool;
#[__nanyc_builtinalias: lt] public operator < (a: cref u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: cref u32, b: __u8): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __u32, b: __u8): __bool;

#[__nanyc_builtinalias: lte] public operator <= (a: cref u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: cref u32, b: __u64): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: __u64): __bool;
#[__nanyc_builtinalias: lte] public operator <= (a: cref u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: cref u32, b: __u32): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: __u32): __bool;
#[__nanyc_builtinalias: lte] public operator <= (a: cref u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: cref u32, b: __u16): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: __u16): __bool;
#[__nanyc_builtinalias: lte] public operator <= (a: cref u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: cref u32, b: __u8): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __u32, b: __u8): __bool;





#[__nanyc_builtinalias: eq] public operator == (a: cref u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref u32, b: __u64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: __u64): __bool;
#[__nanyc_builtinalias: eq] public operator == (a: cref u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref u32, b: __u32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: __u32): __bool;
#[__nanyc_builtinalias: eq] public operator == (a: cref u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref u32, b: __u16): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: __u16): __bool;
#[__nanyc_builtinalias: eq] public operator == (a: cref u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref u32, b: __u8): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __u32, b: __u8): __bool;

#[__nanyc_builtinalias: neq] public operator != (a: cref u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref u32, b: __u64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: cref u64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: __u64): __bool;
#[__nanyc_builtinalias: neq] public operator != (a: cref u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref u32, b: __u32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: cref u32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: __u32): __bool;
#[__nanyc_builtinalias: neq] public operator != (a: cref u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref u32, b: __u16): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: cref u16): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: __u16): __bool;
#[__nanyc_builtinalias: neq] public operator != (a: cref u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref u32, b: __u8): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: cref u8): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __u32, b: __u8): __bool;





#[__nanyc_builtinalias: add] public operator + (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: add] public operator + (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: add] public operator + (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: add] public operator + (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __u32, b: __u8): any;


#[__nanyc_builtinalias: sub] public operator - (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: sub] public operator - (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: sub] public operator - (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: sub] public operator - (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __u32, b: __u8): any;


#[__nanyc_builtinalias: div] public operator / (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: div] public operator / (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: div] public operator / (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: div] public operator / (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: div, nosuggest] public operator / (a: __u32, b: __u8): any;


#[__nanyc_builtinalias: mul] public operator * (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: mul] public operator * (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: mul] public operator * (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: mul] public operator * (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: mul, nosuggest] public operator * (a: __u32, b: __u8): any;






#[__nanyc_builtinalias: and] public operator and (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: and] public operator and (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: and] public operator and (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: and] public operator and (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __u32, b: __u8): any;


#[__nanyc_builtinalias: or] public operator or (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: or] public operator or (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: or] public operator or (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: or] public operator or (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __u32, b: __u8): any;


#[__nanyc_builtinalias: xor] public operator xor (a: cref u32, b: cref u64): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref u32, b: __u64): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: cref u64): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: __u64): any;

#[__nanyc_builtinalias: xor] public operator xor (a: cref u32, b: cref u32): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref u32, b: __u32): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: cref u32): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: __u32): any;

#[__nanyc_builtinalias: xor] public operator xor (a: cref u32, b: cref u16): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref u32, b: __u16): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: cref u16): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: __u16): any;

#[__nanyc_builtinalias: xor] public operator xor (a: cref u32, b: cref u8): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref u32, b: __u8): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: cref u8): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __u32, b: __u8): any;