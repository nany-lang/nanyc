// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


func genericFuncStandalone {
}

unittest std.core.funcs.generics.standalone {
	genericFuncStandalone();
}

class ClassWithGenericFunc {
	func foo<:T:> {
	}
}

unittest std.core.funcs.generics.withinClass {
	var x = new ClassWithGenericFunc;
	x.foo<:bool:>();
}

class GenClassWithGenericFunc<:T:> {
	func bar<:T:> {
	}
}

unittest std.core.funcs.generics.withinGenericClass {
	var x = new GenClassWithGenericFunc<:bool:>;
	x.bar<:bool:>();
}

func genericFuncParam<:T:>
	-> new T;

unittest std.core.funcs.generics.print.simpletype {
	print(genericFuncParam<:u32:>());
}

unittest std.core.funcs.generics.identity {
	32i = genericFuncParam<:i32:>();
	32u = genericFuncParam<:u32:>();
}
