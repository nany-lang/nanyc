// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

uses std.io;

unittest std.io.path.extension {
	var try_path_extension = func (cref file: string, cref expect: string, cref withDot: string) {
		var expectHas = (not withDot.empty);
		ref expectDot = withDot;
		ref expectNoDot = expect;
		var has   = std.io.path.has_extension(file);
		var dot   = std.io.path.extension(file);
		var nodot = std.io.path.extension(file, withDot: false);
		if has != expectHas or dot != expectDot or nodot != expectNoDot then {
			printerr("error: fail '\(file)'\n");
			printerr("       got: has: '\(has)', ext: '\(dot)', nodot: '\(nodot)'\n");
			printerr("    expect: has: '\(expectHas)', ext: '\(expectDot)', nodot: '\(expectNoDot)'\n");
		}
	}
	try_path_extension("foo.txt", expect: "txt", withDot: ".txt");
	try_path_extension("foo.", expect: "", withDot: ".");
	try_path_extension(".", expect: "", withDot: ".");
	try_path_extension("", expect: "", withDot: "");
	try_path_extension("foo", expect: "", withDot: "");
	try_path_extension("foo/", expect: "", withDot: "");
	try_path_extension("foo/txt", expect: "", withDot: "");
	try_path_extension("foo/t", expect: "", withDot: "");
	try_path_extension("foo/baz.n", expect: "n", withDot: ".n");
	try_path_extension("foo/baz.ny", expect: "ny", withDot: ".ny");
	try_path_extension("foo/baz.veryVeryLong even with spaces", expect: "veryVeryLong even with spaces", withDot: ".veryVeryLong even with spaces");
	try_path_extension("foo/.config", expect: "config", withDot: ".config");
	try_path_extension("../../../././././foo.bmp", expect: "bmp", withDot: ".bmp");
}

unittest std.io.path.filename {
	var try_path_filename = func (cref path: string, cref expect: string) {
		ref got = std.io.path.filename(path);
		if got != expect then {
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}
	try_path_filename("/foo/baz/baz.txt", expect: "baz.txt");
	try_path_filename("/foo/baz/baz", expect: "baz");
	try_path_filename("/foo/baz//.config", expect: ".config");
	try_path_filename("/foo/baz/", expect: "");
	try_path_filename("/foo/", expect: "");
	try_path_filename("/", expect: "");
	try_path_filename("", expect: "");
}

unittest std.io.path.folder {
	var try_path_folder = func (cref path: string, cref expect: string) {
		ref got = std.io.path.folder(path);
		if got != expect then {
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}
	try_path_folder("/foo/baz/baz.txt", expect: "/foo/baz");
	try_path_folder("/foo/baz/baz", expect: "/foo/baz");
	try_path_folder("/foo/baz//.config", expect: "/foo/baz/");
	try_path_folder("/foo/baz/", expect: "/foo/baz");
	try_path_folder("/foo/", expect: "/foo");
	try_path_folder("/", expect: "");
	try_path_folder("", expect: "");
}

unittest std.io.path.parent {
	var try_path_parent_folder = func (cref path: string, cref expect: string) {
		ref got = std.io.path.parent(path);
		if got != expect then {
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}
	try_path_parent_folder("/foo/baz/sub-folder/taz", expect: "/foo/baz");
	try_path_parent_folder("/foo/baz/baz.txt", expect: "/foo");
	try_path_parent_folder("/foo/baz/baz", expect: "/foo");
	try_path_parent_folder("/foo/baz//.config", expect: "/foo/baz");
	try_path_parent_folder("/foo/baz/", expect: "/foo");
	try_path_parent_folder("/foo/", expect: "");
	try_path_parent_folder("/", expect: "");
	try_path_parent_folder("", expect: "");
}

unittest std.io.path.stem {
	var try_path_stem = func (cref path: string, cref expect: string) {
		ref got = std.io.path.stem(path);
		if got != expect then {
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}
	try_path_stem("/foo/baz/baz.txt", expect: "baz");
	try_path_stem("/foo/baz/baz", expect: "baz");
	try_path_stem("/foo/baz//.config", expect: "");
	try_path_stem("/foo/baz/", expect: "");
	try_path_stem("/foo/", expect: "");
	try_path_stem("/", expect: "");
	try_path_stem("", expect: "");
}

unittest std.io.path.normalize {
	var try_path_normalize = func (cref path: string, cref expect: string) {
		var got = std.io.path.normalize(path);
		if got != expect then {
			printerr("error: fail '\(path)'\n");
			printerr("       got: '\(got)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}
	try_path_normalize("", expect: "");
	try_path_normalize(".", expect: ".");
	try_path_normalize("..", expect: "..");
	try_path_normalize("/", expect: "/");
	try_path_normalize("/foo", expect: "/foo");
	try_path_normalize("foo", expect: "foo");
	try_path_normalize("o", expect: "o");
	try_path_normalize(".config", expect: ".config");
	try_path_normalize(".config/", expect: ".config");
	try_path_normalize(".config/.", expect: ".config");

	try_path_normalize("/foo/bar/.", expect: "/foo/bar");
	try_path_normalize("/foo/bar/./", expect: "/foo/bar");
	try_path_normalize("/foo/bar/..", expect: "/foo");
	try_path_normalize("/foo/bar/../", expect: "/foo");
	try_path_normalize("/foo/bar/../baz", expect: "/foo/baz");
	try_path_normalize("/foo/bar/../..", expect: "/");
	try_path_normalize("/foo/bar/../../", expect: "/");
	try_path_normalize("/foo/bar/../../baz", expect: "/baz");
	try_path_normalize("/foo/bar/../../../baz", expect: "/baz");
	try_path_normalize("/foo/bar/../../../../baz", expect: "/baz");
	try_path_normalize("/./foo", expect: "/foo");
	try_path_normalize("/../foo", expect: "/foo");
	try_path_normalize("/foo.", expect: "/foo.");
	try_path_normalize("/.foo", expect: "/.foo");
	try_path_normalize("/foo..", expect: "/foo..");
	try_path_normalize("/..foo", expect: "/..foo");
	try_path_normalize("/./../foo", expect: "/foo");
	try_path_normalize("/./foo/.", expect: "/foo");
	try_path_normalize("/foo/./bar", expect: "/foo/bar");
	try_path_normalize("/foo/../bar", expect: "/bar");
	try_path_normalize("/foo//", expect: "/foo");
	try_path_normalize("/foo///bar//", expect: "/foo/bar");

	try_path_normalize("./foo/bar/.", expect: "foo/bar");
	try_path_normalize("./foo/bar/./", expect: "foo/bar");
	try_path_normalize("./foo/bar/..", expect: "foo");
	try_path_normalize("./foo/bar/../", expect: "foo");
	try_path_normalize("./foo/bar/../baz", expect: "foo/baz");
	try_path_normalize("./foo/bar/../..", expect: ".");
	try_path_normalize("./foo/bar/../../", expect: ".");
	try_path_normalize("./foo/bar/../../baz", expect: "baz");
	try_path_normalize("./foo/bar/../../../baz", expect: "../baz");
	try_path_normalize("./foo/bar/../../../../baz", expect: "../../baz");
}
