// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

/// \important THIS FILE IS AUTOMATICALLY GENERATED

unittest std.core.xor {
	assert((0 xor 0) == 0);
	assert((0 xor 10) == 10);
	assert((0 xor 255) == 255);
	assert((0 xor 666) == 666);
	assert((0 xor 1024) == 1024);
	assert((0 xor 123456) == 123456);
	assert((0 xor 65535) == 65535);
	assert((10 xor 0) == 10);
	assert((10 xor 10) == 0);
	assert((10 xor 255) == 245);
	assert((10 xor 666) == 656);
	assert((10 xor 1024) == 1034);
	assert((10 xor 123456) == 123466);
	assert((10 xor 65535) == 65525);
	assert((255 xor 0) == 255);
	assert((255 xor 10) == 245);
	assert((255 xor 255) == 0);
	assert((255 xor 666) == 613);
	assert((255 xor 1024) == 1279);
	assert((255 xor 123456) == 123583);
	assert((255 xor 65535) == 65280);
	assert((666 xor 0) == 666);
	assert((666 xor 10) == 656);
	assert((666 xor 255) == 613);
	assert((666 xor 666) == 0);
	assert((666 xor 1024) == 1690);
	assert((666 xor 123456) == 123098);
	assert((666 xor 65535) == 64869);
	assert((1024 xor 0) == 1024);
	assert((1024 xor 10) == 1034);
	assert((1024 xor 255) == 1279);
	assert((1024 xor 666) == 1690);
	assert((1024 xor 1024) == 0);
	assert((1024 xor 123456) == 124480);
	assert((1024 xor 65535) == 64511);
	assert((123456 xor 0) == 123456);
	assert((123456 xor 10) == 123466);
	assert((123456 xor 255) == 123583);
	assert((123456 xor 666) == 123098);
	assert((123456 xor 1024) == 124480);
	assert((123456 xor 123456) == 0);
	assert((123456 xor 65535) == 73151);
	assert((65535 xor 0) == 65535);
	assert((65535 xor 10) == 65525);
	assert((65535 xor 255) == 65280);
	assert((65535 xor 666) == 64869);
	assert((65535 xor 1024) == 64511);
	assert((65535 xor 123456) == 73151);
	assert((65535 xor 65535) == 0);
}