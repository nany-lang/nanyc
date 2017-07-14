// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

unittest std.core.optional.noparams {
	std.optional<:u64:>();
}

unittest std.core.optional.fromvalue {
	std.optional(666);
}

unittest std.core.optional.deref {
	on scope fail(err: std.InvalidPointer) {
	};
	var p = std.optional(42);
	var x = 42 + p.deref();
	print(x);
}
