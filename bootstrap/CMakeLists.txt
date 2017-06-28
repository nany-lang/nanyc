cmake_minimum_required(VERSION 3.7.1)

#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

project(nany-bootstrap)

option(WITH_PACKAGE_DEB "Build DEB control" OFF)

include(CMakeParseArguments)

# for information related to nany, see settings.txt


include("cmake/messages.cmake")
include(GNUInstallDirs)

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

include("cmake/libdyncall.cmake")
include("cmake/libyuni.cmake")
include("cmake/grammar.cmake")

add_subdirectory(tools)
add_subdirectory("libnanyc")
add_subdirectory(src)

get_filename_component(nany_tests_root "${CMAKE_CURRENT_LIST_DIR}/../../tests/" REALPATH)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/empty.ny" "func main {}\n")
add_custom_target(check
	DEPENDS nanyc-unittest
	COMMAND "${CMAKE_COMMAND}" -E "echo" # for beauty
	COMMAND "$<TARGET_FILE:nanyc-unittest>" --with-nsl "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/empty.ny"
	VERBATIM
)

if (WITH_PACKAGE_DEB)
	include("cmake/package-deb.cmake")
endif()
