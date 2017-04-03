file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../../build-settings.txt" file_settings_data)

foreach (line ${file_settings_data})
	if (NOT "${line}" MATCHES "^//.*$")
		string(REGEX MATCH "[^:]+:" key "${line}")
		string(REGEX MATCH ":.*" value "${line}")
		string(LENGTH "${key}" length)
		math(EXPR length "${length} - 1")
		string(SUBSTRING "${key}" 0 ${length} key)
		string(SUBSTRING "${value}" 1 -1 value)
		string(STRIP "${key}" key)
		string(STRIP "${value}" value)

		set("nany_${key}" "${value}" CACHE INTERNAL "")
		nmessage("${key}: ${value}")
	endif()
endforeach()

unset(file_settings_data)


if ("${nany_version_metadata}" STREQUAL "")
	if (NOT "$ENV{TRAVIS_TAG}" STREQUAL "")
		# do not append a metadata when compiled from a tag
	else()
		set(GIT_COMMIT_HASH "")
		if(NOT "$ENV{TRAVIS_COMMIT_RANGE}" STREQUAL "")
			string(REPLACE "..." ";" GIT_COMMIT_HASH "$ENV{TRAVIS_COMMIT_RANGE}")
			list(GET GIT_COMMIT_HASH 0 __start)
			list(GET GIT_COMMIT_HASH 1 __end)
			string(SUBSTRING "${__start}" 0 7 __start)
			string(SUBSTRING "${__end}" 0 7 __end)
			set(GIT_COMMIT_HASH "${__start}-${__end}")
		elseif(NOT "$ENV{TRAVIS_COMMIT}" STREQUAL "")
			string(SUBSTRING "$ENV{TRAVIS_COMMIT}" 0 7 GIT_COMMIT_HASH)
		else()
			find_program(bin_git NAMES "git")
			if (bin_git)
				execute_process(COMMAND "${bin_git}" log -1 --format=%h
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE GIT_COMMIT_HASH
					OUTPUT_STRIP_TRAILING_WHITESPACE)
			endif()
		endif()

		if (NOT "${GIT_COMMIT_HASH}" STREQUAL "")
			if (NOT "$ENV{TRAVIS_PULL_REQUEST}" STREQUAL "" AND NOT "$ENV{TRAVIS_PULL_REQUEST}" STREQUAL "false")
				set(GIT_COMMIT_HASH "github-$ENV{TRAVIS_PULL_REQUEST}-${GIT_COMMIT_HASH}")
			endif()

			set(nany_version_metadata "${GIT_COMMIT_HASH}" CACHE INTERNAL "")
		endif()
	endif()
endif()




set(nany_version "${nany_version_major}.${nany_version_minor}.${nany_version_patch}" CACHE INTERNAL "")
set(nany_version_string "${nany_version}")
if (NOT "${nany_version_prerelease}" STREQUAL "")
	set(nany_version_string "${nany_version_string}-${nany_version_prerelease}" CACHE INTERNAL "")
endif()
if (NOT "${nany_version_metadata}" STREQUAL "")
	set(nany_version_string "${nany_version_string}+${nany_version_metadata}" CACHE INTERNAL "")
endif()

nmessage("") # for beauty (and to find it easily)
nmessage("version: ${nany_version_string}")

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
