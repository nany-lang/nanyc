
add_executable(nyt-nsl-core-generator
	nsl-core-generator.cpp)

target_link_libraries(nyt-nsl-core-generator PRIVATE libnany yuni-static-core)
