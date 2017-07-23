// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


unittest std.core.closure.identity {
	var f = func (x) -> x;
	assert(f(10) == 10);
	assert(f(42u) == 42u);
	assert(f("hello world") == "hello world");
}

unittest std.core.closure.mult2 {
	var f = func (x) -> x * 2;
	assert(f(10) == 20);
}

unittest std.core.closure.capture {
	var y = 4;
	var f = func (x) -> x * y;
	assert(f(10) == 40);
}
