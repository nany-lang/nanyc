cmake_minimum_required(VERSION 3.7.1)

#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

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

include("cmake/utils.cmake")
include("cmake/build-settings.cmake")
include("cmake/compilation-database.cmake")
include("cmake/arch-platform.cmake")

include("cmake/libyuni.cmake")

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
