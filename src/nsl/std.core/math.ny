// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.math;


public func min(ref a, ref b): ref
	-> if a < b then a else b;

public func max(ref a, ref b): ref
	-> if a < b then b else a;

public func equals(ref a, ref b): bool
	-> a == b;
