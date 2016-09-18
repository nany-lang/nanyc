// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// \file    bool.ny
// \brief   Implementation of the class bool
// \ingroup std.core




// \brief   Boolean datatype
// \ingroup std.core
class bool
{
	operator new;
	operator new(self pod: __bool) {}

	var pod: __bool = __false;
}





#[__nanyc_shortcircuit: __false, __nanyc_builtinalias: and, nosuggest]
public operator and (a: __bool, b: __bool): __bool;

#[__nanyc_shortcircuit: __false, __nanyc_builtinalias: and]
public operator and (cref a: bool, cref b: bool): ref bool;

#[__nanyc_shortcircuit: __false, __nanyc_builtinalias: and, nosuggest]
public operator and (a: __bool, cref b: bool): ref bool;

#[__nanyc_shortcircuit: __false, __nanyc_builtinalias: and, nosuggest]
public operator and (cref a: bool, b: __bool): ref bool;



#[__nanyc_shortcircuit: __true, __nanyc_builtinalias: or, nosuggest]
public operator or (a: __bool, b: __bool): __bool;

#[__nanyc_shortcircuit: __true, __nanyc_builtinalias: or]
public operator or (cref a: bool, cref b: bool): ref bool;

#[__nanyc_shortcircuit: __true, __nanyc_builtinalias: or, nosuggest]
public operator or (a: __bool, cref b: bool): ref bool;

#[__nanyc_shortcircuit: __true, __nanyc_builtinalias: or, nosuggest]
public operator or (cref a: bool, b: __bool): ref bool;


#[__nanyc_builtinalias: gt] public operator > (a: cref bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: cref bool, b: __bool): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: gt, nosuggest] public operator > (a: __bool, b: __bool): __bool;

#[__nanyc_builtinalias: gte] public operator >= (a: cref bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: cref bool, b: __bool): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: gte, nosuggest] public operator >= (a: __bool, b: __bool): __bool;

#[__nanyc_builtinalias: lt] public operator < (a: cref bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: cref bool, b: __bool): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: lt, nosuggest] public operator < (a: __bool, b: __bool): __bool;

#[__nanyc_builtinalias: lte] public operator <= (a: cref bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: cref bool, b: __bool): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: lte, nosuggest] public operator <= (a: __bool, b: __bool): __bool;



#[__nanyc_builtinalias: eq] public operator == (a: cref bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: cref bool, b: __bool): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: eq, nosuggest] public operator == (a: __bool, b: __bool): __bool;

#[__nanyc_builtinalias: neq] public operator != (a: cref bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: cref bool, b: __bool): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __bool, b: cref bool): ref bool;
#[__nanyc_builtinalias: neq, nosuggest] public operator != (a: __bool, b: __bool): __bool;


#[__nanyc_builtinalias: not, nosuggest] public operator not (a: __bool): __bool;
#[__nanyc_builtinalias: not] public operator not (a: cref bool): ref bool;




// -*- mode: nany;-*-
// vim: set filetype=nany:
