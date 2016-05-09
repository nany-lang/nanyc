project(nany-bootstrap)
cmake_minimum_required(VERSION 3.2)

include(CMakeParseArguments)

# for information related to nany, see settings.txt


include("cmake/messages.cmake")

nmessage("Nany Compiler Suite c++/bootstrap")
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

set(extra_flags_debug "")
set(extra_flags_release "")

if (CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_compiler_flag("-flto" NANY_HAS_LTO)
	if (NANY_HAS_LTO)
		# linking issues with LTO currently
		#set(extra_flags_release "${extra_flags_release} -flto")
		#set(CMAKE_STATIC_LINKER_FLAGS_RELEASE}" ${CMAKE_STATIC_LINKER_FLAGS_RELEASE} -flto")
		#set(CMAKE_SHARED_LINKER_FLAGS_RELEASE}" ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -flto")
	endif()
endif()

set(CMAKE_C_FLAGS_DEBUG     "${YN_FLAGS_C_DEBUG} ${extra_flags_debug}")
set(CMAKE_C_FLAGS_RELEASE   "${YN_FLAGS_C_RELEASE} ${extra_flags_release}")
set(CMAKE_CXX_FLAGS_DEBUG   "${YN_FLAGS_CXX_DEBUG} ${extra_flags_debug}")
set(CMAKE_CXX_FLAGS_RELEASE "${YN_FLAGS_CXX_RELEASE} ${extra_flags_release}")






# Common Tools
#include_directories("${CMAKE_CURRENT_BINARY_DIR}/") # include "ncs.grammar/..."
#include_directories("${CMAKE_CURRENT_LIST_DIR}/libnany") # "nany/nany.h"


# Tools
add_subdirectory(tools)

# Nany C library
add_subdirectory("libnany")


#
# nany
#
add_executable(nany "nany.cpp")
target_link_libraries(nany PRIVATE libnany yuni-static-core)
set_target_properties(nany PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")


#
# nany-dump-ast
#
# note: this executable does not rely on `libnany` library on purpose
# (if something goes wrong with libnany, it might be essential to simply print the input AST)
add_executable(nany-dump-ast
	dump-ast.cpp ${nysrc_grammar} ${nysrc_details_grammar})
target_include_directories(nany-dump-ast PRIVATE "${CMAKE_CURRENT_LIST_DIR}/libnany")
add_dependencies(nany-dump-ast nany-grammar-cpp)
target_link_libraries(nany-dump-ast PRIVATE yuni-static-core libnany-grammar)
set_target_properties(nany-dump-ast PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")
# nany-dump-ast does not use libnany (on purpose)
target_compile_definitions(nany-dump-ast PRIVATE "NANY_VERSION=${nany_version_string}")


#
# nany-check-syntax
#
add_executable(nany-check-syntax  check-syntax.cpp)
target_link_libraries(nany-check-syntax PRIVATE libnany yuni-static-core)
set_target_properties(nany-check-syntax PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")


#
# Test
#
get_filename_component(nany_tests_root "${CMAKE_CURRENT_LIST_DIR}/../../tests/" REALPATH)
add_custom_target(check
	DEPENDS nany-check-syntax
	COMMAND "$<TARGET_FILE:nany-check-syntax>" "--use-filename-convention" "${nany_tests_root}/parsing"
	COMMAND "$<TARGET_FILE:nany-check-syntax>"
		"${nany_tests_root}/build"
		"${nany_tests_root}/contest"
		"${nany_tests_root}/benchmark"
	VERBATIM)





install(TARGETS nany
	RUNTIME DESTINATION "bin"
	COMPONENT "nany")



add_custom_target(install-libnany
	DEPENDS libnany
	COMMAND 
		"${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnany"
		-P "${CMAKE_BINARY_DIR}/cmake_install.cmake")

add_custom_target(install-nany
	DEPENDS nany
	COMMAND 
		"${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=nany"
		-P "${CMAKE_BINARY_DIR}/cmake_install.cmake")

add_custom_target(install-libnany-dev
  DEPENDS libnany
  COMMAND 
      "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnany-dev"
      -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")



macro(create_rules_for_packages output)
	if (NOT WIN32 AND NOT APPLE)
		set(destdir "/tmp/__nany-install-${nany_version}/")
		set(dev "dev")
		if ("${output}" STREQUAL "rpm")
			set(dev "devel")
		endif()

		add_custom_target(package-${output}
			DEPENDS libnany
			COMMAND rm -rf "${destdir}"

			COMMAND "${CMAKE_COMMAND}" "-E" echo ""
			COMMAND "${CMAKE_COMMAND}" "-E" echo "notes: to install 'fpm' on debian:"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           apt-get install ruby-dev gcc make"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           gem install fpm"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "       to install 'fpm' on redhat/centos:"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           yum install ruby-devel gcc make"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           gem install fpm"
			COMMAND "${CMAKE_COMMAND}" "-E" echo ""

			COMMAND "DESTDIR=${destdir}/libnany" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnany"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				--name libnany
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor "Nany"
				--description "\"Nany Headers\""
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--verbose
				-p "../../distrib/"
				-C "${destdir}/libnany"

			COMMAND "DESTDIR=${destdir}/nany" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=nany"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				--name nany
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor "Nany"
				--description "\"Nany Programming Language\""
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--depends libnany
				--verbose
				-p "../../distrib/"
				-C "${destdir}/nany"

			COMMAND "DESTDIR=${destdir}/dev" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnany-dev"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				-a all 
				--name libnany-${dev}
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor Nany
				--description "\"Libnany\""
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--depends libnany
				--verbose
				-p "../../distrib/"
				-C "${destdir}/dev"

			COMMAND rm -rf "${destdir}")
	endif()
endmacro()



create_rules_for_packages(deb)
create_rules_for_packages(rpm)

# The end !
