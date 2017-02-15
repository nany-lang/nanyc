// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;


public class HashMap<:K,V:> {

	var count
		-> m_count;

	var empty
		-> m_count == 0u;


	func clear {
		if m_count != 0u then
			doClear();
	}

private:
	func flagIsEmpty(flag: u32): bool -> flag == 1u;
	func flagIsDeleted(flag: u32): bool -> flag == 2u;

	func doClear {
		m_count = 0u;
		m_buckets = 0u;
		m_occupied = 0u;
		m_upperBound = 0u;
		m_keys.clear();
		m_values.clear();
		m_flags.clear();
	}

	func findIndex(cref key): u32 {
		if m_buckets != 0u then {
			var k = std.hash(key);
			while not flagIsEmpty() and () do {

			}
		}
		return 0u;
	}

	var m_count = 0u;
	var m_buckets = 0u;
	var m_occupied = 0u;
	var m_upperBound = 0u;
	var m_keys = new std.Array<:K:>;
	var m_values = new std.Array<:V:>;
	var m_flags = new std.Array<:u32:>;
}
