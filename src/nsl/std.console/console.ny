// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;



public class Console
{
	func write(cref value: string)
	{
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

	//! Error output
	var error -> new Error;


	class Error
	{
		func write(cref value)
		{
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


} // class Console



public operator << (ref console: std.Console, cref value): ref std.Console
{
	console.write(value);
	return console;
}


public operator << (ref console: std.Console.Error, cref value): ref std.Console.Error
{
	console.write(value);
	return console;
}




// -*- mode: nany;-*-
// vim: set filetype=nany:
