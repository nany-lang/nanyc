// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

/// \important THIS FILE IS AUTOMATICALLY GENERATED

/// \file    std.core/i64.ny
/// \brief   Implementation of the class i64, Signed integer with width of exactly 64 bits
/// \ingroup std.core

public class i64 {
	operator new;

	operator new (cref x: i64) {
		pod = x.pod;
	}

	operator new (cref x: i32) {
		pod = x.pod;
	}

	operator new (cref x: i16) {
		pod = x.pod;
	}

	operator new (cref x: i8) {
		pod = x.pod;
	}

	operator new (cref x: u32) {
		pod = x.pod;
	}

	operator new (cref x: u16) {
		pod = x.pod;
	}

	operator new (cref x: u8) {
		pod = x.pod;
	}

	#[nosuggest] operator new (self pod: __i64);

	#[nosuggest] operator new (self pod: __i32);

	#[nosuggest] operator new (self pod: __i16);

	#[nosuggest] operator new (self pod: __i8);

	#[nosuggest] operator new (self pod: __u32);

	#[nosuggest] operator new (self pod: __u16);

	#[nosuggest] operator new (self pod: __u8);

	func as<:T:>
		-> new T(!!as(#[__nanyc_synthetic] typeof(std.asBuiltin(new T)), pod));

	operator ++self: ref i64 {
		pod = !!inc(pod);
		return self;
	}

	operator self++: ref i64 {
		ref tmp = new i64(pod);
		pod = !!inc(pod);
		return tmp;
	}

	operator --self: ref i64 {
		pod = !!dec(pod);
		return self;
	}

	operator self--: ref i64 {
		ref tmp = new i64(pod);
		pod = !!dec(pod);
		return tmp;
	}

	operator += (cref x: i64): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __i64): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: i32): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __i32): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: i16): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __i16): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: i8): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __i8): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: u32): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __u32): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: u16): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __u16): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator += (cref x: u8): ref i64 {
		pod = !!add(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __u8): ref i64 {
		pod = !!add(pod, x);
		return self;
	}

	operator -= (cref x: i64): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __i64): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: i32): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __i32): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: i16): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __i16): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: i8): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __i8): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: u32): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __u32): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: u16): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __u16): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator -= (cref x: u8): ref i64 {
		pod = !!sub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __u8): ref i64 {
		pod = !!sub(pod, x);
		return self;
	}

	operator *= (cref x: i64): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __i64): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator *= (cref x: i32): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __i32): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator *= (cref x: i16): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __i16): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator *= (cref x: i8): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __i8): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator *= (cref x: u32): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __u32): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator *= (cref x: u16): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __u16): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator *= (cref x: u8): ref i64 {
		pod = !!imul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __u8): ref i64 {
		pod = !!imul(pod, x);
		return self;
	}

	operator /= (cref x: i64): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __i64): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

	operator /= (cref x: i32): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __i32): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

	operator /= (cref x: i16): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __i16): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

	operator /= (cref x: i8): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __i8): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

	operator /= (cref x: u32): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __u32): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

	operator /= (cref x: u16): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __u16): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

	operator /= (cref x: u8): ref i64 {
		pod = !!idiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __u8): ref i64 {
		pod = !!idiv(pod, x);
		return self;
	}

private:
	var pod = 0__i64;

} // class i64




#[__nanyc_builtinalias: igt] public operator > (a: cref i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: cref i64, b: __i64): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: __i64): __bool;
#[__nanyc_builtinalias: igt] public operator > (a: cref i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: cref i64, b: __i32): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: __i32): __bool;
#[__nanyc_builtinalias: igt] public operator > (a: cref i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: cref i64, b: __i16): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: __i16): __bool;
#[__nanyc_builtinalias: igt] public operator > (a: cref i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: cref i64, b: __i8): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: igt, nosuggest] public operator > (a: __i64, b: __i8): __bool;

#[__nanyc_builtinalias: igte] public operator >= (a: cref i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: cref i64, b: __i64): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: __i64): __bool;
#[__nanyc_builtinalias: igte] public operator >= (a: cref i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: cref i64, b: __i32): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: __i32): __bool;
#[__nanyc_builtinalias: igte] public operator >= (a: cref i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: cref i64, b: __i16): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: __i16): __bool;
#[__nanyc_builtinalias: igte] public operator >= (a: cref i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: cref i64, b: __i8): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: igte, nosuggest] public operator >= (a: __i64, b: __i8): __bool;

#[__nanyc_builtinalias: ilt] public operator < (a: cref i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: cref i64, b: __i64): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: __i64): __bool;
#[__nanyc_builtinalias: ilt] public operator < (a: cref i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: cref i64, b: __i32): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: __i32): __bool;
#[__nanyc_builtinalias: ilt] public operator < (a: cref i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: cref i64, b: __i16): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: __i16): __bool;
#[__nanyc_builtinalias: ilt] public operator < (a: cref i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: cref i64, b: __i8): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: ilt, nosuggest] public operator < (a: __i64, b: __i8): __bool;

#[__nanyc_builtinalias: ilte] public operator <= (a: cref i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: cref i64, b: __i64): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: __i64): __bool;
#[__nanyc_builtinalias: ilte] public operator <= (a: cref i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: cref i64, b: __i32): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: __i32): __bool;
#[__nanyc_builtinalias: ilte] public operator <= (a: cref i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: cref i64, b: __i16): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: __i16): __bool;
#[__nanyc_builtinalias: ilte] public operator <= (a: cref i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: cref i64, b: __i8): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: ilte, nosuggest] public operator <= (a: __i64, b: __i8): __bool;





#[__nanyc_builtinalias: eq] public operator == (a: cref i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref i64, b: __i64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: __i64): __bool;
#[__nanyc_builtinalias: eq] public operator == (a: cref i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref i64, b: __i32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: __i32): __bool;
#[__nanyc_builtinalias: eq] public operator == (a: cref i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref i64, b: __i16): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: __i16): __bool;
#[__nanyc_builtinalias: eq] public operator == (a: cref i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref i64, b: __i8): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __i64, b: __i8): __bool;

#[__nanyc_builtinalias: neq] public operator != (a: cref i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref i64, b: __i64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: cref i64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: __i64): __bool;
#[__nanyc_builtinalias: neq] public operator != (a: cref i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref i64, b: __i32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: cref i32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: __i32): __bool;
#[__nanyc_builtinalias: neq] public operator != (a: cref i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref i64, b: __i16): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: cref i16): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: __i16): __bool;
#[__nanyc_builtinalias: neq] public operator != (a: cref i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref i64, b: __i8): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: cref i8): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __i64, b: __i8): __bool;





#[__nanyc_builtinalias: add] public operator + (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: add] public operator + (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: add] public operator + (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: add] public operator + (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: add, nosuggest] public operator + (a: __i64, b: __i8): any;


#[__nanyc_builtinalias: sub] public operator - (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: sub] public operator - (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: sub] public operator - (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: sub] public operator - (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: sub, nosuggest] public operator - (a: __i64, b: __i8): any;


#[__nanyc_builtinalias: idiv] public operator / (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: idiv] public operator / (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: idiv] public operator / (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: idiv] public operator / (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: idiv, nosuggest] public operator / (a: __i64, b: __i8): any;


#[__nanyc_builtinalias: imul] public operator * (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: imul] public operator * (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: imul] public operator * (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: imul] public operator * (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: imul, nosuggest] public operator * (a: __i64, b: __i8): any;






#[__nanyc_builtinalias: and] public operator and (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: and] public operator and (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: and] public operator and (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: and] public operator and (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: and, nosuggest] public operator and (a: __i64, b: __i8): any;


#[__nanyc_builtinalias: or] public operator or (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: or] public operator or (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: or] public operator or (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: or] public operator or (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: or, nosuggest] public operator or (a: __i64, b: __i8): any;


#[__nanyc_builtinalias: xor] public operator xor (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: xor] public operator xor (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: xor] public operator xor (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: xor] public operator xor (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: xor, nosuggest] public operator xor (a: __i64, b: __i8): any;


#[__nanyc_builtinalias: modi] public operator mod (a: cref i64, b: cref i64): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: cref i64, b: __i64): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: cref i64): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: __i64): any;

#[__nanyc_builtinalias: modi] public operator mod (a: cref i64, b: cref i32): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: cref i64, b: __i32): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: cref i32): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: __i32): any;

#[__nanyc_builtinalias: modi] public operator mod (a: cref i64, b: cref i16): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: cref i64, b: __i16): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: cref i16): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: __i16): any;

#[__nanyc_builtinalias: modi] public operator mod (a: cref i64, b: cref i8): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: cref i64, b: __i8): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: cref i8): any;
#[__nanyc_builtinalias: modi, nosuggest] public operator mod (a: __i64, b: __i8): any;