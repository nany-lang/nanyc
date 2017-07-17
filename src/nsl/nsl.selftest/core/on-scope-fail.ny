// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


func willRaise {
	raise 666;
}

func mayRaise(flag) {
	if flag then
		raise 666;
}

unittest std.core.on.scope.fail.local {
	on scope fail(e: i32) {
	}
	raise 666;
	assert(false);
}

unittest std.core.on.scope.fail.local.withscope {
	var x = 69;
	{
		on scope fail(e: i32) {
			assert(x == 42);
		}
		x = 42;
		raise 666;
		x = 0;
	}
	assert(x == 42);
}

unittest std.core.on.scope.fail.local.withscope2 {
	var x = 69;
	on scope fail(e: i32) {
		assert(x == 42);
	}
	{
		x = 42;
		raise 666;
		x = 0;
	}
	assert(x == 42);
}

unittest std.core.on.scope.fail.funccall {
	var x = 69;
	on scope fail(e: i32) {
		assert(x == 42);
	}
	x = 42;
	willRaise();
	assert(x == 42);
}

unittest std.core.on.scope.fail.funccall.cond {
	var x = 69;
	on scope fail(e: i32) {
		assert(x == 42);
	}
	x = 42;
	mayRaise(true);
	assert(x == 42);
}

// ---

unittest std.core.on.scope.fail.all.local {
	on scope fail {
	}
	raise 666;
	assert(false);
}

unittest std.core.on.scope.fail.all.local.withscope {
	var x = 69;
	{
		on scope fail {
			assert(x == 42);
		}
		x = 42;
		raise 666;
		x = 0;
	}
	assert(x == 42);
}

unittest std.core.on.scope.fail.all.local.withscope2 {
	var x = 69;
	on scope fail {
		assert(x == 42);
	}
	{
		x = 42;
		raise 666;
		x = 0;
	}
	assert(x == 42);
}

unittest std.core.on.scope.fail.all.funccall {
	var x = 69;
	on scope fail {
		assert(x == 42);
	}
	x = 42;
	willRaise();
	assert(x == 42);
}

unittest std.core.on.scope.fail.all.funccall.cond {
	var x = 69;
	on scope fail {
		assert(x == 42);
	}
	x = 42;
	mayRaise(true);
	assert(x == 42);
}
