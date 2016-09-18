// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

namespace std.os;




//! Get if the current os is an AIX variant
public var aix
	-> new bool(!!os.is.aix);

//! Get if the current os is an BSD variant
public var bsd
	-> new bool(!!os.is.bsd);

//! Get if the current os is Cygwin (MS Windows)
public var cygwin
	-> new bool(!!os.is.cygwin);

//! Get if the current os is a GNU/Linux variant
public var linux
	-> new bool(!!os.is.linux);

//! Get if the current os is a macOS variant
public var macos
	-> new bool(!!os.is.macos);

//! Get if the current os is posix compliant
public var posix
	-> new bool(!!os.is.posix);

//! Get if the current os is an unix variant
public var unix
	-> new bool(!!os.is.unix);

//! Get if the current os is Microsoft Windows
public var windows
	-> new bool(!!os.is.windows);






// -*- mode: nany;-*-
// vim: set filetype=nany:
