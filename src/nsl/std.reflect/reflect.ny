// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;


public func reflect<:T:>: ref
	-> new std.reflection.Typeinfo<:T:>;

public func reflect(cref expr): ref
	-> new std.reflection.Typeinfo<:typeof(expr):>;
