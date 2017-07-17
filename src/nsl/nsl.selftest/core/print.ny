// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


unittest std.core.print {
	print("hello world");
}

unittest std.core.print.builtins {
	print(42__u8);
	print(44__u16);
	print(46__u32);
	print(48__u64);
	print(69__i8);
	print(71__i16);
	print(73__i32);
	print(75__i64);
}

unittest std.core.print.integers {
	print(42u);
	print(44u);
	print(46u);
	print(48u);
	print(69);
	print(71);
	print(73);
	print(75);
}

unittest std.core.print.bools {
	print(false);
	print(true);
	print(__true);
	print(__false);
}

unittest std.core.print.pointers {
	print(null);
}
