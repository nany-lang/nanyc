// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



unittest std.core.string.stayempty
{
	var s = "";
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');

	s.clear();
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');

	s = "";
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');

	s.append("");
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');

	s += "";
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');

	s << "";
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');

	s.reserve(0u);
	assert(s.size == 0u);
	assert(s.capacity == 0u);
	assert(s.empty);
	assert(s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.count('_') == 0u);
	assert(s.last == '\0');
	assert(s.first == '\0');
}


unittest std.core.string.smallstring
{
	var s = "hello";
	assert(s.capacity >= 5u);
	assert(s.size == 5u);
	assert(not s.empty);
	assert(not s.blank);
	assert(not s.contains('z'));
	assert(not s.contains(' '));
	assert(s.contains('o'));
	assert(s.count('l') == 2u);
	assert(s.count('h') == 1u);
	assert(s.count('o') == 1u);
	assert(s.last == 'o');
	assert(s.first == 'h');

	s += " world !";
	assert(s.capacity >= 13u);
	assert(s.size == 13u);
	assert(not s.empty);
	assert(not s.blank);
	assert(not s.contains('z'));
	assert(s.contains(' '));
	assert(s.contains('o'));
	assert(s.contains('!'));
	assert(s.count('l') == 3u);
	assert(s.count('h') == 1u);
	assert(s.count('o') == 2u);
	assert(s.count(' ') == 2u);
	assert(s.last == '!');
	assert(s.first == 'h');
}






// -*- mode: nany;-*-
// vim: set filetype=nany:
