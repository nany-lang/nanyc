
add_executable(nyt-nanyfile-to-cstring EXCLUDE_FROM_ALL
	nsl-nany-to-cstring.cpp)

add_executable(nyt-nsl-core-generator EXCLUDE_FROM_ALL
	nsl-core-generator.cpp)

target_link_libraries(nyt-nsl-core-generator  PRIVATE yuni-static-core)
target_link_libraries(nyt-nanyfile-to-cstring PRIVATE yuni-static-core)

set_target_properties(nyt-nanyfile-to-cstring PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

set_target_properties(nyt-nsl-core-generator PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)
