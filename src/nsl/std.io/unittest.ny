// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.




unittest std.io.path.extension
{
	var tryPathExtension = func (cref file: string, cref expect: string, cref withDot: string)
	{
		var expectHas = (not withDot.empty);
		ref expectDot = withDot;
		ref expectNoDot = expect;

		var has   = std.io.path.hasExtension(file);
		var dot   = std.io.path.extension(file);
		var nodot = std.io.path.extension(file, withDot: false);

		if has != expectHas or dot != expectDot or nodot != expectNoDot then
		{
			printerr("error: fail '\(file)'\n");
			printerr("       got: has: '\(has)', ext: '\(dot)', nodot: '\(nodot)'\n");
			printerr("    expect: has: '\(expectHas)', ext: '\(expectDot)', nodot: '\(expectNoDot)'\n");
		}
	}

	tryPathExtension("foo.txt", expect: "txt", withDot: ".txt");
	tryPathExtension("foo.", expect: "", withDot: ".");
	tryPathExtension(".", expect: "", withDot: ".");
	tryPathExtension("", expect: "", withDot: "");
	tryPathExtension("foo", expect: "", withDot: "");
	tryPathExtension("foo/", expect: "", withDot: "");
	tryPathExtension("foo/txt", expect: "", withDot: "");
	tryPathExtension("foo/t", expect: "", withDot: "");
	tryPathExtension("foo/baz.n", expect: "n", withDot: ".n");
	tryPathExtension("foo/baz.ny", expect: "ny", withDot: ".ny");
	tryPathExtension("foo/baz.veryVeryLong even with spaces", expect: "veryVeryLong even with spaces", withDot: ".veryVeryLong even with spaces");
	tryPathExtension("foo/.config", expect: "config", withDot: ".config");
	tryPathExtension("../../../././././foo.bmp", expect: "bmp", withDot: ".bmp");
}


unittest std.io.path.filename
{
	var tryPathFilename = func (cref path: string, cref expect: string)
	{
		ref got = std.io.path.filename(path);
		if got != expect then
		{
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}

	tryPathFilename("/foo/baz/baz.txt", expect: "baz.txt");
	tryPathFilename("/foo/baz/baz", expect: "baz");
	tryPathFilename("/foo/baz//.config", expect: ".config");
	tryPathFilename("/foo/baz/", expect: "");
	tryPathFilename("/foo/", expect: "");
	tryPathFilename("/", expect: "");
	tryPathFilename("", expect: "");
}


unittest std.io.path.folder
{
	var tryPathFolder = func (cref path: string, cref expect: string)
	{
		ref got = std.io.path.folder(path);
		if got != expect then
		{
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}

	tryPathFolder("/foo/baz/baz.txt", expect: "/foo/baz");
	tryPathFolder("/foo/baz/baz", expect: "/foo/baz");
	tryPathFolder("/foo/baz//.config", expect: "/foo/baz/");
	tryPathFolder("/foo/baz/", expect: "/foo/baz");
	tryPathFolder("/foo/", expect: "/foo");
	tryPathFolder("/", expect: "");
	tryPathFolder("", expect: "");
}


unittest std.io.path.parent
{
	var tryPathParentFolder = func (cref path: string, cref expect: string)
	{
		ref got = std.io.path.parent(path);
		if got != expect then
		{
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}

	tryPathParentFolder("/foo/baz/sub-folder/taz", expect: "/foo/baz");
	tryPathParentFolder("/foo/baz/baz.txt", expect: "/foo");
	tryPathParentFolder("/foo/baz/baz", expect: "/foo");
	tryPathParentFolder("/foo/baz//.config", expect: "/foo/baz");
	tryPathParentFolder("/foo/baz/", expect: "/foo");
	tryPathParentFolder("/foo/", expect: "");
	tryPathParentFolder("/", expect: "");
	tryPathParentFolder("", expect: "");
}


unittest std.io.path.stem
{
	var tryPathStem = func (cref path: string, cref expect: string)
	{
		ref got = std.io.path.stem(path);
		if got != expect then
		{
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}

	tryPathStem("/foo/baz/baz.txt", expect: "baz");
	tryPathStem("/foo/baz/baz", expect: "baz");
	tryPathStem("/foo/baz//.config", expect: "");
	tryPathStem("/foo/baz/", expect: "");
	tryPathStem("/foo/", expect: "");
	tryPathStem("/", expect: "");
	tryPathStem("", expect: "");
}


unittest std.io.path.normalize
{
	var tryPathNormalize = func (cref path: string, cref expect: string)
	{
		var got = std.io.path.normalize(path);
		if got != expect then
		{
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}

	tryPathNormalize("", expect: "");
	tryPathNormalize(".", expect: ".");
	tryPathNormalize("..", expect: "..");
	tryPathNormalize("/", expect: "/");
	tryPathNormalize("/foo", expect: "/foo");
	tryPathNormalize("foo", expect: "foo");
	tryPathNormalize("o", expect: "o");
	tryPathNormalize(".config", expect: ".config");
	tryPathNormalize(".config/", expect: ".config");
	tryPathNormalize(".config/.", expect: ".config");

	tryPathNormalize("/foo/bar/.", expect: "/foo/bar");
	tryPathNormalize("/foo/bar/./", expect: "/foo/bar");
	tryPathNormalize("/foo/bar/..", expect: "/foo");
	tryPathNormalize("/foo/bar/../", expect: "/foo");
	tryPathNormalize("/foo/bar/../baz", expect: "/foo/baz");
	tryPathNormalize("/foo/bar/../..", expect: "/");
	tryPathNormalize("/foo/bar/../../", expect: "/");
	tryPathNormalize("/foo/bar/../../baz", expect: "/baz");
	tryPathNormalize("/foo/bar/../../../baz", expect: "/baz");
	tryPathNormalize("/foo/bar/../../../../baz", expect: "/baz");
	tryPathNormalize("/./foo", expect: "/foo");
	tryPathNormalize("/../foo", expect: "/foo");
	tryPathNormalize("/foo.", expect: "/foo.");
	tryPathNormalize("/.foo", expect: "/.foo");
	tryPathNormalize("/foo..", expect: "/foo..");
	tryPathNormalize("/..foo", expect: "/..foo");
	tryPathNormalize("/./../foo", expect: "/foo");
	tryPathNormalize("/./foo/.", expect: "/foo");
	tryPathNormalize("/foo/./bar", expect: "/foo/bar");
	tryPathNormalize("/foo/../bar", expect: "/bar");
	tryPathNormalize("/foo//", expect: "/foo");
	tryPathNormalize("/foo///bar//", expect: "/foo/bar");

	tryPathNormalize("./foo/bar/.", expect: "foo/bar");
	tryPathNormalize("./foo/bar/./", expect: "foo/bar");
	tryPathNormalize("./foo/bar/..", expect: "foo");
	tryPathNormalize("./foo/bar/../", expect: "foo");
	tryPathNormalize("./foo/bar/../baz", expect: "foo/baz");
	tryPathNormalize("./foo/bar/../..", expect: ".");
	tryPathNormalize("./foo/bar/../../", expect: ".");
	tryPathNormalize("./foo/bar/../../baz", expect: "baz");
	tryPathNormalize("./foo/bar/../../../baz", expect: "../baz");
	tryPathNormalize("./foo/bar/../../../../baz", expect: "../../baz");
}






// -*- mode: nany;-*-
// vim: set filetype=nany:
