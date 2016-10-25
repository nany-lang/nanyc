// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.console;


public class Out {
	func write(cref value: string) {
		var size = value.size.pod;
		if size != 0__u32 then
			!!__nanyc_console_out(value.m_cstr, size);
	}

	func write(cref value: bool)
		-> write(if value then "true" else "false");

	func write(cref value)
		-> write((new string) += value);

	func flush
		-> !!__nanyc_console_out_flush;

	//! Are colors supported
	var colors
		-> new bool(!!__nanyc_console_out_has_colors);

} // class Console


public class Error {
	func write(cref value) {
		var size = value.size.pod;
		if size != 0__u32 then
			!!__nanyc_console_err(value.m_cstr, size);
	}

	func write(cref value: bool)
		-> write(if value then "true" else "false");

	func write(cref value)
		-> write((new string) += value);

	func flush
		-> !!__nanyc_console_err_flush;

	//! Are colors supported
	var colors
		-> new bool(!!__nanyc_console_err_has_colors);

} // class Error




public operator << (ref out: std.console.Out, cref value): ref std.console.Out {
	out.write(value);
	return out;
}

public operator << (ref out: std.console.Error, cref value): ref std.console.Error {
	out.write(value);
	return out;
}
