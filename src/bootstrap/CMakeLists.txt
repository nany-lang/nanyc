project(nany-bootstrap)
cmake_minimum_required(VERSION 3.2)

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

include("cmake/utils.cmake")
include("cmake/build-settings.cmake")



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

#
# -- Google RE2 (fast, safe, thread-friendly alternative to backtracking regular expression engines)
#
#add_subdirectory("ext/re2")
#set_target_properties(re2 PROPERTIES EXCLUDE_FROM_ALL true)




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
	COMMAND "$<TARGET_FILE:yuni-parser-generator>" --format=cpp -i "${NANY_YGR}" -n Nany::AST -o "${NANY_GRAMMAR_TARGETDIR}"
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


#
# nany
#
add_executable(nanyc "nany.cpp")
target_link_libraries(nanyc PRIVATE libnanyc yuni-static-core)
set_target_properties(nanyc PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")


#
# nanyc-dump-ast
#
# note: this executable does not rely on `libnanyc` library on purpose
# (if something goes wrong with libnanyc, it might be essential to simply print the input AST)
add_executable(nanyc-dump-ast
	dump-ast.cpp ${nysrc_grammar} ${nysrc_details_grammar})
target_include_directories(nanyc-dump-ast PRIVATE "${CMAKE_CURRENT_LIST_DIR}/libnanyc")
add_dependencies(nanyc-dump-ast nanyc-grammar-cpp)
target_link_libraries(nanyc-dump-ast PRIVATE yuni-static-core libnanyc-grammar)
set_target_properties(nanyc-dump-ast PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")
# nanyc-dump-ast does not use libnanyc (on purpose)
target_compile_definitions(nanyc-dump-ast PRIVATE "NANY_VERSION=${nany_version_string}")


#
# nanyc-check-syntax
#
add_executable(nanyc-check-syntax  check-syntax.cpp)
target_link_libraries(nanyc-check-syntax PRIVATE libnanyc yuni-static-core)
set_target_properties(nanyc-check-syntax PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")


#
# Test
#
get_filename_component(nany_tests_root "${CMAKE_CURRENT_LIST_DIR}/../../tests/" REALPATH)
add_custom_target(check
	DEPENDS nanyc-check-syntax
	COMMAND "$<TARGET_FILE:nanyc-check-syntax>" "--use-filename-convention" "${nany_tests_root}/parsing"
	COMMAND "$<TARGET_FILE:nanyc-check-syntax>"
		"${nany_tests_root}/build"
		"${nany_tests_root}/contest"
		"${nany_tests_root}/benchmark"
	VERBATIM)





install(TARGETS nanyc
	RUNTIME DESTINATION "bin"
	COMPONENT "nanyc")

install(TARGETS nanyc-dump-ast
	RUNTIME DESTINATION "bin"
	COMPONENT "libnanyc-dev-tools")

install(TARGETS nanyc-check-syntax
	RUNTIME DESTINATION "bin"
	COMPONENT "libnanyc-dev-tools")



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
if (NOT WIN32)
	include("cmake/packages-deb-rpm.cmake")
	create_rules_for_packages(deb)
	create_rules_for_packages(rpm)
else()
	include("cmake/wix.cmake")
	create_rules_for_msi()
endif()

# The end !
