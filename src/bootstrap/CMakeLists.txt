project(nany-bootstrap)
cmake_minimum_required(VERSION 3.0)

include(CMakeParseArguments)

# for information related to nany, see settings.txt




include("cmake/messages.cmake")

nmessage("Nany Compiler Suite c++/bootstrap")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "" OR (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "debug"))
	# release by default
	set(CMAKE_BUILD_TYPE "debug")
endif()
nmessage("configuration: ${CMAKE_BUILD_TYPE}")

#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

include("cmake/utils.cmake")
include("cmake/settings.cmake")





#include("cmake/cpack.cmake")


#
# -- Facilities for YCM - (vim/YouCompleteMe)
#
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(nany_compilation_database_folder "${CMAKE_CURRENT_BINARY_DIR}")
configure_file("cmake/you-complete-me.vim.template.cmake" ".ycm_extra_conf.py")


#
# -- Yuni - (aka libyuni)
#
configure_file("cmake/yuni-profilebuild-template.cmake" "ext/yuni/src/ProfileBuild.cmake")

if (NOT EXISTS "ext/yuni/src/CMakeLists.txt")
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
#include(ExternalProject)
#externalproject_add(ext_dyncall
#	SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/dyncall/"
#	PREFIX "${CMAKE_CURRENT_LIST_DIR}/ext/dyncall"
#	INSTALL_COMMAND ""
#	CMAKE_ARGS
#	"-DCMAKE_BUILD_TYPE=Release")
#add_library(dyncall UNKNOWN IMPORTED)
#add_dependencies(dyncall ext_dyncall)



#
# -- Nany GRAMMAR - (Nany grammar to C++ translator)
#
get_filename_component(NANY_GRAMMAR_SOURCEDIR   "${CMAKE_CURRENT_SOURCE_DIR}/../grammar"  REALPATH)
get_filename_component(NANY_YGR                 "${NANY_GRAMMAR_SOURCEDIR}/nany.ygr"      REALPATH)
get_filename_component(NANY_GRAMMAR_TARGETDIR   "${CMAKE_CURRENT_BINARY_DIR}/libnany/details/grammar" REALPATH)
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
	COMMAND "$<TARGET_FILE:yuni-parser-generator>" --format=cpp -i "${NANY_YGR}" -n Nany::AST -o "${NANY_GRAMMAR_TARGETDIR}"
	COMMAND "${CMAKE_COMMAND}" -E touch "${NANY_GRAMMAR_FILE_MARKER}"
	VERBATIM
	DEPENDS yuni-parser-generator "${NANY_YGR}")

set_source_files_properties("${NANY_GRAMMAR_H}"   PROPERTIES GENERATED true)
set_source_files_properties("${NANY_GRAMMAR_HXX}" PROPERTIES GENERATED true)
set_source_files_properties("${NANY_GRAMMAR_CPP}" PROPERTIES GENERATED true)


add_custom_target(nany-grammar-cpp
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

set(extra_flags "")
if (UNIX)
	if (CMAKE_COMPILER_IS_GNUCC)
		set(extra_flags "${extra_flags} -fvisibility=hidden")
	endif()
endif()


set(CMAKE_C_FLAGS_DEBUG     "${YN_FLAGS_C_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE   "${YN_FLAGS_C_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG   "${YN_FLAGS_CXX_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${YN_FLAGS_CXX_RELEASE}")






# Common Tools
#include_directories("${CMAKE_CURRENT_BINARY_DIR}/") # include "ncs.grammar/..."
#include_directories("${CMAKE_CURRENT_LIST_DIR}/libnany") # "nany/nany.h"


# Tools
add_subdirectory(tools)

# Nany C library
add_subdirectory("libnany")


add_executable(nany "nany.cpp")
target_link_libraries(nany PRIVATE libnany yuni-static-core)
set_target_properties(nany PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")


add_executable(nyt-print-ast  print-ast.cpp)
target_link_libraries(nyt-print-ast PRIVATE libnany-grammar yuni-static-core)

add_executable(nyt-check-syntax  check-syntax.cpp)
target_link_libraries(nyt-check-syntax PRIVATE libnany yuni-static-core)


#
# Test
#
get_filename_component(nany_tests_root "${CMAKE_CURRENT_LIST_DIR}/../../tests/" REALPATH)
add_custom_target(check
	DEPENDS nyt-check-syntax
	COMMAND "$<TARGET_FILE:nyt-check-syntax>" "--use-filename-convention" "${nany_tests_root}/parsing"
	COMMAND "$<TARGET_FILE:nyt-check-syntax>"
		"${nany_tests_root}/build"
		"${nany_tests_root}/contest"
		"${nany_tests_root}/benchmark"
	VERBATIM)

# The end !
