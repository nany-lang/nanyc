// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;




class Ascii
{
	operator new;

	#[nosuggest] operator new (value: __u8)
	{
		m_value = value;
	}

	operator new (cref value: u8)
	{
		m_value = value.pod;
	}

	operator new (cref ascii: Ascii)
	{
		m_value = ascii.m_value;
	}


	//! Integer representation
	var asU8 -> { get: new u8(m_value), set: reset(value) };

	//! Get if the ascii is null (\0)
	var zero
		-> (m_value == 0__u8);

	//! Get if the ascii is a space (real space or tab)
	// '\f', '\n', '\r', '\t', '\v'
	var blank
		-> m_value == 32__u8 or m_value == 9__u8
		or m_value == 10__u8 or m_value == 11__u8 or m_value == 12__u8
		or m_value == 0__u8;

	//! Get if the ascii is a digit
	var digit
		-> m_value >= 48__u8 and m_value <= 57__u8;

	func toDigit
		-> if isDigit then new u8(m_value - 48__u8) else 0u8;

	//! Get if the ascii is a space (real space or tab)
	var tab
		-> new bool(m_value == 9__u8);

	//! Get if the ascii is a space (real space or tab)
	var space
		-> new bool(m_value == 32__u8);

	//! Get if the ascii is a lower case letter
	var lower
		-> new bool(m_value >= 97__u8 and m_value <= 122__u8);

	//! Get if the ascii is an upper case letter
	var upper
		-> new bool(m_value >= 65__u8 and m_value <= 90__u8);

	//! Get if the ascii is a letter (lower case or upper case)
	var letter
		-> isLower or isUpper;

	//! Get if the ascii is a valid value
	var valid
		-> new bool(m_value < 127__u8);


internal:
	func reset(value: __u8)    { m_value = value; }
	func reset(cref value: u8) { m_value = value.pod; }

	//! internal representation of an ascii
	var m_value: __u8 = 0__u8;
}






public operator == (cref a: Ascii, cref b: Ascii): bool
	-> new bool(a.m_value == b.m_value);

public operator != (cref a: Ascii, cref b: Ascii): bool
	-> new bool(a.m_value != b.m_value);

public operator < (cref a: Ascii, cref b: Ascii): bool
	-> new bool(a.m_value < b.m_value);

public operator > (cref a: Ascii, cref b: Ascii): bool
	-> new bool(a.m_value > b.m_value);

public operator <= (cref a: Ascii, cref b: Ascii): bool
	-> new bool(a.m_value <= b.m_value);

public operator > (cref a: Ascii, cref b: Ascii): bool
	-> new bool(a.m_value >= b.m_value);





// -*- mode: nany;-*-
// vim: set filetype=nany:
