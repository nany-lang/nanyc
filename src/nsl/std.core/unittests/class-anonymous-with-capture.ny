// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


func fooCall: ref {
	var x = 10;
	return new class {
		func bar -> x;
	};
}

unittest std.core.class.capture.funcCall {
	var a = fooCall().bar();
	assert(a == 10);
}

func fooProperty: ref {
	var x = 12;
	return new class {
		var y -> x;
	};
}

unittest std.core.class.capture.funcProperty {
	var a = fooProperty().y;
	assert(a == 12);
}

func fooWithParameter(z): ref {
	var x = z;
	return new class {
		var y -> x;
	};
}

unittest std.core.class.capture.funcWithParameter {
	var a = fooWithParameter(12).y;
	assert(a == 12);
	var b = fooWithParameter(42u).y;
	assert(b == 42u);
}
