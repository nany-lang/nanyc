configure_file("cmake/yuni-profilebuild-template.cmake" "ext/yuni/src/ProfileBuild.cmake")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext/yuni/src/CMakeLists.txt")
	nerror("The extenal library 'yuni' is not present (submodule)")
	message(FATAL_ERROR "aborting")
endif()
add_subdirectory("ext/yuni/src")
include_directories("ext/yuni/src")

# Compilation Flags
file(READ "${CMAKE_CURRENT_BINARY_DIR}/ext/yuni/src/compiler-flags-debug-cc"    YN_FLAGS_C_DEBUG)
file(READ "${CMAKE_CURRENT_BINARY_DIR}/ext/yuni/src/compiler-flags-release-cc"  YN_FLAGS_C_RELEASE)
file(READ "${CMAKE_CURRENT_BINARY_DIR}/ext/yuni/src/compiler-flags-debug-cxx"   YN_FLAGS_CXX_DEBUG)
file(READ "${CMAKE_CURRENT_BINARY_DIR}/ext/yuni/src/compiler-flags-release-cxx" YN_FLAGS_CXX_RELEASE)

set(extra_flags_debug "")
set(extra_flags_release "")

if (CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_compiler_flag("-flto" NANY_HAS_FLAG_LTO)
	if (NANY_HAS_FLAG_LTO)
		# linking issues with LTO currently
		#set(extra_flags_release "${extra_flags_release} -flto")
		#set(CMAKE_STATIC_LINKER_FLAGS_RELEASE}" ${CMAKE_STATIC_LINKER_FLAGS_RELEASE} -flto")
		#set(CMAKE_SHARED_LINKER_FLAGS_RELEASE}" ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -flto")
	endif()

	check_cxx_compiler_flag("-Werror=unused-result" NANY_HAS_FLAG_W2R_RESULT)
	if (NANY_HAS_FLAG_W2R_RESULT)
		set(extra_flags_release "${extra_flags_release} -Werror=unused-result")
		set(extra_flags_debug   "${extra_flags_debug} -Werror=unused-result")
	endif()
endif()

set(CMAKE_C_FLAGS_DEBUG     "${YN_FLAGS_C_DEBUG} ${extra_flags_debug}")
set(CMAKE_C_FLAGS_RELEASE   "${YN_FLAGS_C_RELEASE} ${extra_flags_release}")
set(CMAKE_CXX_FLAGS_DEBUG   "${YN_FLAGS_CXX_DEBUG} ${extra_flags_debug}")
set(CMAKE_CXX_FLAGS_RELEASE "${YN_FLAGS_CXX_RELEASE} ${extra_flags_release}")
