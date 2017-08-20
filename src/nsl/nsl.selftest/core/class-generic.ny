// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


class ClassFuncGenericRetValue {
	func foo<:T:> -> new T;
}

unittest std.core.classes.funcs.generics.retvalue {
	32i = (new ClassFuncGenericRetValue).foo<:i32:>();
}
