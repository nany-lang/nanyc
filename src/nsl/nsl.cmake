#
# Standard Library (NSL) for Nany C++/Bootstrap
#
set(nsl_files
	# std.core
	"${nsl_root}/std.core/bool.ny"
	"${nsl_root}/std.core/string.ny"
	"${nsl_root}/std.core/unittest-string.ny"
	"${nsl_root}/std.core/f64.ny"
	"${nsl_root}/std.core/f32.ny"
	"${nsl_root}/std.core/i64.ny"
	"${nsl_root}/std.core/u64.ny"
	"${nsl_root}/std.core/i32.ny"
	"${nsl_root}/std.core/u32.ny"
	"${nsl_root}/std.core/i16.ny"
	"${nsl_root}/std.core/u16.ny"
	"${nsl_root}/std.core/u8.ny"
	"${nsl_root}/std.core/i8.ny"
	"${nsl_root}/std.core/utils.ny"
	"${nsl_root}/std.core/pointer.ny"
	"${nsl_root}/std.core/ascii.ny"

	# C types
	"${nsl_root}/std.c/ctypes.ny"

	# std.env
	"${nsl_root}/std.env/env.ny"

	# std.io
	"${nsl_root}/std.io/path.ny"
	"${nsl_root}/std.io/file.ny"
	"${nsl_root}/std.io/file-object.ny"
	"${nsl_root}/std.io/folder.ny"
	"${nsl_root}/std.io/folder-object.ny"
	"${nsl_root}/std.io/io.ny"
	"${nsl_root}/std.io/unittest.ny"

	# std.math
	"${nsl_root}/std.math/math.ny"

	# std.memory
	"${nsl_root}/std.memory/utils.ny"

	# std.os
	"${nsl_root}/std.os/process.ny"
	"${nsl_root}/std.os/os.ny"

	# Console
	"${nsl_root}/std.console/console.ny"
	"${nsl_root}/std.console/global.ny"

	# Digest
	"${nsl_root}/std.digest/digest.ny"

	CACHE INTERNAL "Nany Standard Library - File list")
