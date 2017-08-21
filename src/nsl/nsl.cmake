function (make_component_from_collection)
	set(options)
	set(oneValueArgs COLLECTION COMPONENT)
	set(multiValueArgs)
	cmake_parse_arguments(opts "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	install(
		DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${opts_COLLECTION}"
		DESTINATION "${NANYC_COLLECTION_SYSTEM_PATH}"
		COMPONENT "${opts_COMPONENT}"
		USE_SOURCE_PERMISSIONS
		FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
		DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
		FILES_MATCHING
			PATTERN "*.ny"
			PATTERN "*.collection"
	)
endfunction()

make_component_from_collection(
	COLLECTION nsl.selftest
	COMPONENT  nanyc-nsl-selftest
)
make_component_from_collection(
	COLLECTION std.digest.md5
	COMPONENT  nanyc-nsl-digest-md5
)

#
# Standard Library (NSL) for Nany C++/Bootstrap
#
set(nsl_files
	"${nsl_root}/std.console/console.ny"
	"${nsl_root}/std.console/global.ny"
	"${nsl_root}/std.core/ascii.ny"
	"${nsl_root}/std.core/bool.ny"
	"${nsl_root}/std.core/containers/array.ny"
	"${nsl_root}/std.core/ctypes.ny"
	"${nsl_root}/std.core/details/string.ny"
	"${nsl_root}/std.core/environment.ny"
	"${nsl_root}/std.core/f32.ny"
	"${nsl_root}/std.core/f64.ny"
	"${nsl_root}/std.core/i16.ny"
	"${nsl_root}/std.core/i32.ny"
	"${nsl_root}/std.core/i64.ny"
	"${nsl_root}/std.core/i8.ny"
	"${nsl_root}/std.core/memory.ny"
	"${nsl_root}/std.core/optional.ny"
	"${nsl_root}/std.core/pointer.ny"
	"${nsl_root}/std.core/std-utils.ny"
	"${nsl_root}/std.core/string.ny"
	"${nsl_root}/std.core/u16.ny"
	"${nsl_root}/std.core/u32.ny"
	"${nsl_root}/std.core/u64.ny"
	"${nsl_root}/std.core/u8.ny"
	"${nsl_root}/std.core/utils.ny"
	"${nsl_root}/std.io/file-object.ny"
	"${nsl_root}/std.io/file.ny"
	"${nsl_root}/std.io/folder-object.ny"
	"${nsl_root}/std.io/folder.ny"
	"${nsl_root}/std.io/io.ny"
	"${nsl_root}/std.io/path.ny"
	"${nsl_root}/std.math/math.ny"
	"${nsl_root}/std.os/os.ny"
	"${nsl_root}/std.os/process.ny"
	CACHE INTERNAL "Nany Standard Library - File list"
)
