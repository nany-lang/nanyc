project(nany-bootstrap)
cmake_minimum_required(VERSION 3.0)

include(CMakeParseArguments)

# for information related to nany, see settings.txt




include("cmake/messages.cmake")

nmessage("Nany Compiler Suite c++/bootstrap")
include("cmake/utils.cmake")
include("cmake/settings.cmake")


if ("${CMAKE_BUILD_TYPE}" STREQUAL "" OR (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "debug"))
	# release by default
	set(CMAKE_BUILD_TYPE "debug")
endif()
nmessage("configuration: ${CMAKE_BUILD_TYPE}")




include("cmake/cpack.cmake")

configure_file("cmake/yuni-profilebuild-template.cmake" "ext/yuni/src/ProfileBuild.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(nany_compilation_database_folder "${CMAKE_CURRENT_BINARY_DIR}")
configure_file("cmake/you-complete-me.vim.template.cmake" ".ycm_extra_conf.py")

if (NOT EXISTS "ext/yuni/src/CMakeLists.txt")
	nerror("The extenal library 'yuni' is not present (submodule)")
	message(FATAL_ERROR "aborting")
endif()
add_subdirectory("ext/yuni/src")
include_directories("ext/yuni/src")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")




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
	COMMAND "$<TARGET_FILE:yuni-parser-generator>" --format=cpp -i "${NANY_YGR}" -n Nany -o "${NANY_GRAMMAR_TARGETDIR}"
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


#
# config.h
#
message(STATUS "generating config.h")
if (NOT EXISTS "config.h")
	configure_file("cmake/config.h.cmake" "config.h")
else()
	nmessage("ignoring config.h: already exists")
endif()





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
	VERSION   "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")


add_executable(nyt-print-ast  print-ast.cpp)
target_link_libraries(nyt-print-ast PRIVATE libnany yuni-static-core)

add_executable(nyt-check-syntax  check-syntax.cpp)
target_link_libraries(nyt-check-syntax PRIVATE libnany yuni-static-core)



# The end !
