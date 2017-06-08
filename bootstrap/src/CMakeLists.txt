### nanyc
add_executable(nanyc
	"nanyc.cpp"
	"nanyc-utils.cpp"
	"nanyc-utils.h"
)
target_link_libraries(nanyc PRIVATE libnanyc yuni-static-core)
set_target_properties(nanyc PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)
install(TARGETS nanyc RUNTIME DESTINATION "bin" COMPONENT "nanyc")


### nanyc-unittest
add_executable(nanyc-unittest "unittest.cpp")
target_link_libraries(nanyc-unittest PRIVATE libnanyc yuni-static-core)
set_target_properties(nanyc-unittest
	PROPERTIES VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

### nanyc-unittest
add_executable(nanyc-unittest-new "nanyc-unittest.cpp")
target_link_libraries(nanyc-unittest-new PRIVATE libnanyc yuni-static-core)
set_target_properties(nanyc-unittest-new
	PROPERTIES VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)


### nanyc-dump-ast
# note: this executable does not rely on `libnanyc` library on purpose
# (if something goes wrong with libnanyc, it might be essential to simply print the input AST)
add_executable(nanyc-dump-ast
	dump-ast.cpp ${nysrc_grammar} ${nysrc_details_grammar}
)
target_include_directories(nanyc-dump-ast PRIVATE "${CMAKE_CURRENT_LIST_DIR}/libnanyc")
add_dependencies(nanyc-dump-ast nanyc-grammar-cpp)
target_link_libraries(nanyc-dump-ast PRIVATE yuni-static-core libnanyc-grammar)
set_target_properties(nanyc-dump-ast PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)
# nanyc-dump-ast does not use libnanyc (on purpose)
target_compile_definitions(nanyc-dump-ast PRIVATE "NANY_VERSION=${nany_version_string}")
install(TARGETS nanyc-dump-ast RUNTIME DESTINATION "bin" COMPONENT "libnanyc-dev-tools")


### nanyc-check-syntax
add_executable(nanyc-check-syntax  check-syntax.cpp)
target_link_libraries(nanyc-check-syntax PRIVATE libnanyc yuni-static-core)
set_target_properties(nanyc-check-syntax PROPERTIES
	VERSION "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)
install(TARGETS nanyc-check-syntax RUNTIME DESTINATION "bin" COMPONENT "libnanyc-dev-tools")
