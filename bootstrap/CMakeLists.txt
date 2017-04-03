cmake_minimum_required(VERSION 3.7.1)
project(nany-bootstrap)

include(CMakeParseArguments)

# for information related to nany, see settings.txt


include("cmake/messages.cmake")

nmessage("nany c++/bootstrap")
nmessage("")
nmessage("cmake ${CMAKE_VERSION} in '${CMAKE_SYSTEM}' ${CMAKE_SYSTEM_PROCESSOR}")
nmessage("  from '${CMAKE_COMMAND}', '${CMAKE_ROOT}'")
nmessage("current listdir: ${CMAKE_CURRENT_LIST_DIR}")
nmessage("")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "" OR (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "debug"))
	# release by default
	set(CMAKE_BUILD_TYPE "debug")
endif()
nmessage("configuration: ${CMAKE_BUILD_TYPE}")

#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_MACOSX_RPATH 1)

include("cmake/utils.cmake")
include("cmake/build-settings.cmake")
include("cmake/compilation-database.cmake")



#
# Architecture
#
include(CheckSymbolExists)
if(WIN32)
	check_symbol_exists("_M_AMD64" "" RTC_ARCH_X64)
	if(NOT RTC_ARCH_X64)
		check_symbol_exists("_M_IX86" "" RTC_ARCH_X86)
	endif(NOT RTC_ARCH_X64)
	# add check for arm here
	# see http://msdn.microsoft.com/en-us/library/b0084kay.aspx
else(WIN32)
	check_symbol_exists("__i386__" "" RTC_ARCH_X86)
	check_symbol_exists("__x86_64__" "" RTC_ARCH_X64)
	check_symbol_exists("__arm__" "" RTC_ARCH_ARM)
endif(WIN32)


#
# Underlying platform
#
set(nany_os_debian 0)
if (NOT WIN32)
	set(nany_os_windows 0)
	if (EXISTS "/etc/debian_version")
		set(nany_os_debian 1)
		system(nany_arch dpkg --print-architecture)
	endif()
else()
	set(nany_os_windows 1)
endif()


#
# -- Yuni - (aka libyuni)
#
configure_file("cmake/yuni-profilebuild-template.cmake" "ext/yuni/src/ProfileBuild.cmake")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext/yuni/src/CMakeLists.txt")
	nerror("The extenal library 'yuni' is not present (submodule)")
	message(FATAL_ERROR "aborting")
endif()
add_subdirectory("ext/yuni/src")
include_directories("ext/yuni/src")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")


if (MSVC)
	# DynCALL does not like SAFESEH on Windows x86
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
endif()


#
# -- dyncall - (Generic Dynamic FFI package / http://www.dyncall.org/)
#
add_subdirectory("ext/dyncall")




#
# -- Nany GRAMMAR - (Nany grammar to C++ translator)
#
get_filename_component(NANY_GRAMMAR_SOURCEDIR   "${CMAKE_CURRENT_SOURCE_DIR}/../grammar"  REALPATH)
get_filename_component(NANY_YGR                 "${NANY_GRAMMAR_SOURCEDIR}/nany.ygr"      REALPATH)
get_filename_component(NANY_GRAMMAR_TARGETDIR   "${CMAKE_CURRENT_BINARY_DIR}/libnanyc/details/grammar" REALPATH)
get_filename_component(NANY_GRAMMAR_CPP         "${NANY_GRAMMAR_TARGETDIR}/nany.cpp"      REALPATH)
get_filename_component(NANY_GRAMMAR_H           "${NANY_GRAMMAR_TARGETDIR}/nany.h"        REALPATH)
get_filename_component(NANY_GRAMMAR_HXX         "${NANY_GRAMMAR_TARGETDIR}/nany.hxx"      REALPATH)
get_filename_component(NANY_GRAMMAR_FILE_MARKER "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/nany-bootstrap-grammar-cxx-classes" REALPATH)

add_custom_command(
	OUTPUT
		"${NANY_GRAMMAR_CPP}" "${NANY_GRAMMAR_H}" "${NANY_GRAMMAR_HXX}"
		"${NANY_GRAMMAR_FILE_MARKER}"
	COMMENT "generating c++ Nany parser from grammar"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${NANY_GRAMMAR_TARGETDIR}"
	COMMAND "$<TARGET_FILE:yuni-parser-generator>" --format=cpp -i "${NANY_YGR}" -n ny::AST -o "${NANY_GRAMMAR_TARGETDIR}"
	COMMAND "${CMAKE_COMMAND}" -E touch "${NANY_GRAMMAR_FILE_MARKER}"
	VERBATIM
	DEPENDS yuni-parser-generator "${NANY_YGR}")

set_source_files_properties("${NANY_GRAMMAR_H}"   PROPERTIES GENERATED true)
set_source_files_properties("${NANY_GRAMMAR_HXX}" PROPERTIES GENERATED true)
set_source_files_properties("${NANY_GRAMMAR_CPP}" PROPERTIES GENERATED true)


add_custom_target(nanyc-grammar-cpp
	DEPENDS yuni-parser-generator
		"${NANY_GRAMMAR_CPP}" "${NANY_GRAMMAR_H}" "${NANY_GRAMMAR_HXX}"
		"${NANY_GRAMMAR_FILE_MARKER}"
	SOURCES "${NANY_YGR}")

if (NOT EXISTS "${NANY_GRAMMAR_CPP}")
	file(WRITE "${NANY_GRAMMAR_CPP}" "")
endif()
if (NOT EXISTS "${NANY_GRAMMAR_H}")
	file(WRITE "${NANY_GRAMMAR_H}" "")
endif()
if (NOT EXISTS "${NANY_GRAMMAR_HXX}")
	file(WRITE "${NANY_GRAMMAR_HXX}" "")
endif()






#
# Compilation Flags
#
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






# Common Tools
#include_directories("${CMAKE_CURRENT_BINARY_DIR}/") # include "ncs.grammar/..."
#include_directories("${CMAKE_CURRENT_LIST_DIR}/libnanyc") # "nany/nany.h"


# Tools
add_subdirectory(tools)

# Nany C library
add_subdirectory("libnanyc")

# tools
add_subdirectory(src)

#
# Test
#
get_filename_component(nany_tests_root "${CMAKE_CURRENT_LIST_DIR}/../../tests/" REALPATH)
#add_custom_target(check
#	DEPENDS nanyc-check-syntax
#	COMMAND "$<TARGET_FILE:nanyc-check-syntax>" "--use-filename-convention" "${nany_tests_root}/parsing"
#	COMMAND "$<TARGET_FILE:nanyc-check-syntax>"
#		"${nany_tests_root}/build"
#		"${nany_tests_root}/contest"
#		"${nany_tests_root}/benchmark"
#	VERBATIM)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/empty.ny" "func main {}\n")
add_custom_target(check
	DEPENDS nanyc-unittest
	COMMAND "${CMAKE_COMMAND}" -E "echo" # for beauty
	COMMAND "$<TARGET_FILE:nanyc-unittest>" --with-nsl "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/empty.ny"
	VERBATIM)




add_custom_target(install-libnanyc
	DEPENDS libnanyc
	COMMAND
		"${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnanyc"
		-P "${CMAKE_BINARY_DIR}/cmake_install.cmake")

add_custom_target(install-nany
	DEPENDS nany
	COMMAND
		"${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=nany"
		-P "${CMAKE_BINARY_DIR}/cmake_install.cmake")

add_custom_target(install-libnanyc-dev
  DEPENDS libnanyc
  COMMAND
      "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnanyc-dev"
      -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")

add_custom_target(install-libnanyc-dev-tools
  DEPENDS libnanyc nanyc-check-syntax nanyc-dump-ast
  COMMAND
      "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnanyc-dev-tools"
      -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")


#
# Packages
#
if (nany_os_debian)
	include("cmake/packages-deb.cmake")
endif()
if (WIN32)
	include("cmake/packages-msi.cmake")
endif()

# The end !
