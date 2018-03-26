// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

/// \important THIS FILE IS AUTOMATICALLY GENERATED

/// \file    std.core/f32.ny
/// \brief   Implementation of the class f32, single-precision floating-point number (32bits)
/// \ingroup std.core

public class f32 {
	operator new;

	operator new(cref x: f32) {
		pod = x.pod;
	}

	#[nosuggest] operator new(self pod: __f32);

	func as<:T:>: ref {
		return new T(!!as(#[__nanyc_synthetic] typeof(std.asBuiltin(new T)), pod));
	}

	operator ++self: ref f32 {
		pod = !!finc(pod);
		return self;
	}

	operator self++: ref f32 {
		ref tmp = new f32(pod);
		pod = !!finc(pod);
		return tmp;
	}

	operator --self: ref f32 {
		pod = !!fdec(pod);
		return self;
	}

	operator self--: ref f32 {
		ref tmp = new f32(pod);
		pod = !!fdec(pod);
		return tmp;
	}

	operator += (cref x: f32): ref f32 {
		pod = !!fadd(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __f32): ref f32 {
		pod = !!fadd(pod, x);
		return self;
	}

	operator -= (cref x: f32): ref f32 {
		pod = !!fsub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __f32): ref f32 {
		pod = !!fsub(pod, x);
		return self;
	}

	operator *= (cref x: f32): ref f32 {
		pod = !!fmul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __f32): ref f32 {
		pod = !!fmul(pod, x);
		return self;
	}

	operator /= (cref x: f32): ref f32 {
		pod = !!fdiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __f32): ref f32 {
		pod = !!fdiv(pod, x);
		return self;
	}

private:
	var pod = 0__f32;

} // class f32




#[__nanyc_builtinalias: fgt] public operator > (a: cref f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: fgt, nosuggest] public operator > (a: cref f32, b: __f32): ref bool;
#[__nanyc_builtinalias: fgt, nosuggest] public operator > (a: __f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: fgt, nosuggest] public operator > (a: __f32, b: __f32): __bool;

#[__nanyc_builtinalias: fgte] public operator >= (a: cref f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: fgte, nosuggest] public operator >= (a: cref f32, b: __f32): ref bool;
#[__nanyc_builtinalias: fgte, nosuggest] public operator >= (a: __f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: fgte, nosuggest] public operator >= (a: __f32, b: __f32): __bool;

#[__nanyc_builtinalias: flt] public operator < (a: cref f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: flt, nosuggest] public operator < (a: cref f32, b: __f32): ref bool;
#[__nanyc_builtinalias: flt, nosuggest] public operator < (a: __f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: flt, nosuggest] public operator < (a: __f32, b: __f32): __bool;

#[__nanyc_builtinalias: flte] public operator <= (a: cref f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: flte, nosuggest] public operator <= (a: cref f32, b: __f32): ref bool;
#[__nanyc_builtinalias: flte, nosuggest] public operator <= (a: __f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: flte, nosuggest] public operator <= (a: __f32, b: __f32): __bool;

#[__nanyc_builtinalias: eq] public operator == (a: cref f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref f32, b: __f32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __f32, b: __f32): __bool;

#[__nanyc_builtinalias: neq] public operator != (a: cref f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref f32, b: __f32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __f32, b: cref f32): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __f32, b: __f32): __bool;



#[__nanyc_builtinalias: fadd] public operator + (a: cref f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fadd, nosuggest] public operator + (a: cref f32, b: __f32): ref f32;
#[__nanyc_builtinalias: fadd, nosuggest] public operator + (a: __f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fadd, nosuggest] public operator + (a: __f32, b: __f32): __f32;


#[__nanyc_builtinalias: fsub] public operator - (a: cref f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fsub, nosuggest] public operator - (a: cref f32, b: __f32): ref f32;
#[__nanyc_builtinalias: fsub, nosuggest] public operator - (a: __f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fsub, nosuggest] public operator - (a: __f32, b: __f32): __f32;


#[__nanyc_builtinalias: fdiv] public operator / (a: cref f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fdiv, nosuggest] public operator / (a: cref f32, b: __f32): ref f32;
#[__nanyc_builtinalias: fdiv, nosuggest] public operator / (a: __f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fdiv, nosuggest] public operator / (a: __f32, b: __f32): __f32;


#[__nanyc_builtinalias: fmul] public operator * (a: cref f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fmul, nosuggest] public operator * (a: cref f32, b: __f32): ref f32;
#[__nanyc_builtinalias: fmul, nosuggest] public operator * (a: __f32, b: cref f32): ref f32;
#[__nanyc_builtinalias: fmul, nosuggest] public operator * (a: __f32, b: __f32): __f32;