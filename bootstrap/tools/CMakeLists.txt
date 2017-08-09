function(make_nanyc_tool)
	set(options)
	set(oneValueArgs TARGET)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(OPTS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	add_executable("${OPTS_TARGET}" EXCLUDE_FROM_ALL
		${OPTS_SOURCES}
	)
	target_link_libraries("${OPTS_TARGET}" PRIVATE yuni-static-core)
	set_target_properties("${OPTS_TARGET}" PROPERTIES
		VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
	)
endfunction()

make_nanyc_tool(
	TARGET nanyc-devtool-nsl-integer-float-generator
	SOURCES nsl-integer-float-generator.cpp
)

make_nanyc_tool(
	TARGET nyt-index-generator
	SOURCES nsl-index-generator.cpp
)
