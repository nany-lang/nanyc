// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

/// \important THIS FILE IS AUTOMATICALLY GENERATED

/// \file    std.core/f64.ny
/// \brief   Implementation of the class f64, double-precision floating-point number (64bits)
/// \ingroup std.core

public class f64 {
	operator new;

	operator new(cref x: f32) {
		pod = x.pod;
	}

	operator new(cref x: f64) {
		pod = x.pod;
	}

	#[nosuggest] operator new(self pod: __f32);

	#[nosuggest] operator new(self pod: __f64);

	func as<:T:>: ref {
		return new T(!!as(#[__nanyc_synthetic] typeof(std.asBuiltin(new T)), pod));
	}

	operator ++self: ref f64 {
		pod = !!finc(pod);
		return self;
	}

	operator self++: ref f64 {
		ref tmp = new f64(pod);
		pod = !!finc(pod);
		return tmp;
	}

	operator --self: ref f64 {
		pod = !!fdec(pod);
		return self;
	}

	operator self--: ref f64 {
		ref tmp = new f64(pod);
		pod = !!fdec(pod);
		return tmp;
	}

	operator += (cref x: f64): ref f64 {
		pod = !!fadd(pod, x.pod);
		return self;
	}

	#[nosuggest] operator += (x: __f64): ref f64 {
		pod = !!fadd(pod, x);
		return self;
	}

	operator -= (cref x: f64): ref f64 {
		pod = !!fsub(pod, x.pod);
		return self;
	}

	#[nosuggest] operator -= (x: __f64): ref f64 {
		pod = !!fsub(pod, x);
		return self;
	}

	operator *= (cref x: f64): ref f64 {
		pod = !!fmul(pod, x.pod);
		return self;
	}

	#[nosuggest] operator *= (x: __f64): ref f64 {
		pod = !!fmul(pod, x);
		return self;
	}

	operator /= (cref x: f64): ref f64 {
		pod = !!fdiv(pod, x.pod);
		return self;
	}

	#[nosuggest] operator /= (x: __f64): ref f64 {
		pod = !!fdiv(pod, x);
		return self;
	}

private:
	var pod = 0__f64;

} // class f64




#[__nanyc_builtinalias: fgt] public operator > (a: cref f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: fgt, nosuggest] public operator > (a: cref f64, b: __f64): ref bool;
#[__nanyc_builtinalias: fgt, nosuggest] public operator > (a: __f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: fgt, nosuggest] public operator > (a: __f64, b: __f64): __bool;

#[__nanyc_builtinalias: fgte] public operator >= (a: cref f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: fgte, nosuggest] public operator >= (a: cref f64, b: __f64): ref bool;
#[__nanyc_builtinalias: fgte, nosuggest] public operator >= (a: __f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: fgte, nosuggest] public operator >= (a: __f64, b: __f64): __bool;

#[__nanyc_builtinalias: flt] public operator < (a: cref f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: flt, nosuggest] public operator < (a: cref f64, b: __f64): ref bool;
#[__nanyc_builtinalias: flt, nosuggest] public operator < (a: __f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: flt, nosuggest] public operator < (a: __f64, b: __f64): __bool;

#[__nanyc_builtinalias: flte] public operator <= (a: cref f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: flte, nosuggest] public operator <= (a: cref f64, b: __f64): ref bool;
#[__nanyc_builtinalias: flte, nosuggest] public operator <= (a: __f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: flte, nosuggest] public operator <= (a: __f64, b: __f64): __bool;

#[__nanyc_builtinalias: eq] public operator == (a: cref f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref f64, b: __f64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __f64, b: __f64): __bool;

#[__nanyc_builtinalias: neq] public operator != (a: cref f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref f64, b: __f64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __f64, b: cref f64): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __f64, b: __f64): __bool;



#[__nanyc_builtinalias: fadd] public operator + (a: cref f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fadd, nosuggest] public operator + (a: cref f64, b: __f64): ref f64;
#[__nanyc_builtinalias: fadd, nosuggest] public operator + (a: __f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fadd, nosuggest] public operator + (a: __f64, b: __f64): __f64;


#[__nanyc_builtinalias: fsub] public operator - (a: cref f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fsub, nosuggest] public operator - (a: cref f64, b: __f64): ref f64;
#[__nanyc_builtinalias: fsub, nosuggest] public operator - (a: __f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fsub, nosuggest] public operator - (a: __f64, b: __f64): __f64;


#[__nanyc_builtinalias: fdiv] public operator / (a: cref f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fdiv, nosuggest] public operator / (a: cref f64, b: __f64): ref f64;
#[__nanyc_builtinalias: fdiv, nosuggest] public operator / (a: __f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fdiv, nosuggest] public operator / (a: __f64, b: __f64): __f64;


#[__nanyc_builtinalias: fmul] public operator * (a: cref f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fmul, nosuggest] public operator * (a: cref f64, b: __f64): ref f64;
#[__nanyc_builtinalias: fmul, nosuggest] public operator * (a: __f64, b: cref f64): ref f64;
#[__nanyc_builtinalias: fmul, nosuggest] public operator * (a: __f64, b: __f64): __f64;