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

		set("nany_${key}" "${value}" CACHE STRING "" FORCE)
		nmessage("${key}: ${value}")
	endif()
endforeach()

unset(file_settings_data)



set(nany_version_string "${nany_version_major}.${nany_version_minor}.${nany_version_patch}")
if (NOT "${nany_version_prerelease}" STREQUAL "")
	set(nany_version_string "${nany_version_string}-${nany_version_prerelease}")
endif()
if (NOT "${nany_version_metadata}" STREQUAL "")
	set(nany_version_string "${nany_version_string}+${nany_version_metadata}")
endif()
