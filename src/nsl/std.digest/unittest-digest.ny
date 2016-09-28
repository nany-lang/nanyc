// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.




unittest std.digest.md5
{
	var tryMD5 = func (cref text: string, cref expect: string)
	{
		ref digest = std.digest.md5(text);
		if digest != expect then
		{
			printerr("error: fail '\(text)'\n");
			printerr("       got: '\(digest)'\n");
			printerr("    expect: '\(expect)'\n");
		}
		ref digest2 = std.digest.md5(text.data, 0__u64 + text.size.pod);
		if digest2 != expect then
		{
			printerr("error: fail '\(text)'\n");
			printerr("       got: '\(digest2)'\n");
			printerr("    expect: '\(expect)'\n");
		}
	}

	// from RFC
	tryMD5("a", expect: "0cc175b9c0f1b6a831c399e269772661");
	tryMD5("abc", expect: "900150983cd24fb0d6963f7d28e17f72");
	tryMD5("message digest", expect: "f96b697d7cb7938d525a2f31aaf161d0");
	tryMD5("abcdefghijklmnopqrstuvwxyz", expect: "c3fcd3d76192e4007dfb496cca67e13b");
	tryMD5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		expect: "d174ab98d277d9f5a5611c2c9f419d9f");
	tryMD5("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
		expect: "57edf4a22be3c955ac49da2e2107b67a");
	tryMD5("", expect: "d41d8cd98f00b204e9800998ecf8427e");

	// Home made
	tryMD5("value", expect: "2063c1608d6e0baf80249c42e2be5804");
	tryMD5("日本", expect: "4dbed2e657457884e67137d3514119b3");
}





// -*- mode: nany;-*-
// vim: set filetype=nany:
