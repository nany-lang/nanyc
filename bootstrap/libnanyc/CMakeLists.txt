project(libnany)

get_filename_component(libnany_config_folder "${CMAKE_CURRENT_BINARY_DIR}" REALPATH)
file(MAKE_DIRECTORY "${libnany_config_folder}")

if (NOT "${nany_version_string}" STREQUAL "${__LIBNANYC_CONFIG_HEADER_CACHE}"
	OR NOT EXISTS "${libnany_config_folder}/libnanyc-version.h")
	nmessage("generating 'libnanyc-version.h' in ${libnany_config_folder}/libnanyc-version.h")
	configure_file("cmake/configure-version.h.cmake" "${libnany_config_folder}/libnanyc-version.h" @ONLY)
	set(__LIBNANYC_CONFIG_HEADER_CACHE "${nany_version_string}" CACHE STRING "" FORCE)
else()
	nmessage("ignoring libnanyc-version.h ${libnany_config_folder}/libnanyc-version.h (cache)")
endif()

if (NOT EXISTS "${libnany_config_folder}/libnanyc-config.h")
	nmessage("generating 'libnanyc-config.h' in ${libnany_config_folder}/libnanyc-config.h")
	configure_file("cmake/configure-config.h.cmake" "${libnany_config_folder}/libnanyc-config.h" @ONLY)
else()
	nmessage("ignoring libnanyc-config.h ${libnany_config_folder}/libnanyc-config.h (already exists)")
endif()

if (NOT EXISTS "${libnany_config_folder}/libnanyc-traces.h")
	nmessage("generating 'libnanyc-traces.h' in ${libnany_config_folder}/libnanyc-traces.h")
	configure_file("cmake/configure-traces.h.cmake" "${libnany_config_folder}/libnanyc-traces.h" @ONLY)
else()
	nmessage("ignoring libnanyc-traces.h ${libnany_config_folder}/libnanyc-traces.h (already exists)")
endif()

# generate nsl:std.core
get_filename_component(nsl_stdcore_root "${CMAKE_CURRENT_LIST_DIR}/../../src/nsl/std.core/" REALPATH)
get_filename_component(nsl_root "${CMAKE_CURRENT_LIST_DIR}/../../src/nsl/" REALPATH)
set(nsl_root "${nsl_root}" CACHE INTERNEL "NSL Root path")

include("${nsl_root}/nsl.cmake")

set(__embed_nsl_hxx_dir "${CMAKE_CURRENT_BINARY_DIR}/details/compiler")
set(__embed_nsl_hxx "${__embed_nsl_hxx_dir}/embed-nsl.hxx")
add_custom_command(
	OUTPUT  "${__embed_nsl_hxx}"
	COMMAND "$<TARGET_FILE:nyt-index-generator>"
		-o "${__embed_nsl_hxx}"
		-d "${nsl_root}"
		-f "${nsl_files}"
		-u "${nsl_files_unittest}"
	DEPENDS
		nyt-index-generator
		${nsl_files}
		${nsl_files_unittest}
	VERBATIM
)

list(LENGTH nsl_files nsl_files_count)
list(LENGTH nsl_files_unittest nsl_files_unittest_count)
nmessage("standard library: ${nsl_files_count} files, (${nsl_files_unittest_count} test files)")

add_library(libnanyc-grammar STATIC EXCLUDE_FROM_ALL
	${NANY_YGR}
	${NANY_GRAMMAR_H} ${NANY_GRAMMAR_HXX} ${NANY_GRAMMAR_CPP}
)

add_dependencies(libnanyc-grammar nanyc-grammar-cpp)

set(mode_libnany "SHARED")
if ("${NANY_STATIC}")
	set(mode_libnany "STATIC")
endif()

add_library(libnanyc ${mode_libnany}
	"${libnany_config_folder}/libnanyc-config.h"
	"${libnany_config_folder}/libnanyc-traces.h"
	"${libnany_config_folder}/libnanyc-version.h"
	"${__embed_nsl_hxx}"
	"details/ast/ast.cpp"
	"details/ast/ast.h"
	"details/ast/ast.hxx"
	"details/atom/atom-map.cpp"
	"details/atom/atom-map.h"
	"details/atom/atom-map.hxx"
	"details/atom/atom.cpp"
	"details/atom/atom.h"
	"details/atom/atom.hxx"
	"details/atom/classdef-follow.cpp"
	"details/atom/classdef-follow.h"
	"details/atom/classdef-follow.hxx"
	"details/atom/classdef-overloads.cpp"
	"details/atom/classdef-overloads.h"
	"details/atom/classdef-overloads.hxx"
	"details/atom/classdef-table-view.cpp"
	"details/atom/classdef-table-view.h"
	"details/atom/classdef-table-view.hxx"
	"details/atom/classdef-table.cpp"
	"details/atom/classdef-table.h"
	"details/atom/classdef-table.hxx"
	"details/atom/classdef.cpp"
	"details/atom/classdef.h"
	"details/atom/classdef.hxx"
	"details/atom/ctype.cpp"
	"details/atom/ctype.h"
	"details/atom/funcdef.cpp"
	"details/atom/funcdef.h"
	"details/atom/funcdef.hxx"
	"details/atom/interface.cpp"
	"details/atom/interface.h"
	"details/atom/interface.hxx"
	"details/atom/qualifiers.h"
	"details/atom/qualifiers.hxx"
	"details/atom/signature.cpp"
	"details/atom/signature.h"
	"details/atom/signature.hxx"
	"details/atom/type.h"
	"details/atom/vardef.h"
	"details/atom/visibility.cpp"
	"details/atom/visibility.h"
	"details/compiler/compdb.h"
	"details/compiler/compiler.cpp"
	"details/compiler/compiler.h"
	"details/errors/complain.cpp"
	"details/errors/complain.h"
	"details/errors/errors.cpp"
	"details/errors/errors.h"
	"details/errors/exception.cpp"
	"details/errors/exception.h"
	"details/intrinsic/catalog.cpp"
	"details/intrinsic/catalog.h"
	"details/intrinsic/catalog.hxx"
	"details/intrinsic/intrinsic.h"
	"details/io/adapter/dummy.cpp"
	"details/io/adapter/localfolder.cpp"
	"details/ir/emit.h"
	"details/ir/instruction.h"
	"details/ir/ir.h"
	"details/ir/isa/data.cpp"
	"details/ir/isa/data.h"
	"details/ir/isa/opcodes.h"
	"details/ir/isa/printer.inc.hpp"
	"details/ir/sequence.cpp"
	"details/ir/sequence.h"
	"details/ir/sequence.hxx"
	"details/pass/a-src2ast/ast-from-source.h"
	"details/pass/b-ast-normalize/normalize.cpp"
	"details/pass/b-ast-normalize/normalize.h"
	"details/pass/c-ast2ir/context.cpp"
	"details/pass/c-ast2ir/context.h"
	"details/pass/c-ast2ir/context.hxx"
	"details/pass/c-ast2ir/exception.cpp"
	"details/pass/c-ast2ir/exception.h"
	"details/pass/c-ast2ir/scope-array.cpp"
	"details/pass/c-ast2ir/scope-attributes.cpp"
	"details/pass/c-ast2ir/scope-call.cpp"
	"details/pass/c-ast2ir/scope-class.cpp"
	"details/pass/c-ast2ir/scope-closure.cpp"
	"details/pass/c-ast2ir/scope-expr.cpp"
	"details/pass/c-ast2ir/scope-for.cpp"
	"details/pass/c-ast2ir/scope-func.cpp"
	"details/pass/c-ast2ir/scope-generic-type-parameters.cpp"
	"details/pass/c-ast2ir/scope-if.cpp"
	"details/pass/c-ast2ir/scope-in.cpp"
	"details/pass/c-ast2ir/scope-intrinsic.cpp"
	"details/pass/c-ast2ir/scope-new.cpp"
	"details/pass/c-ast2ir/scope-number.cpp"
	"details/pass/c-ast2ir/scope-object.cpp"
	"details/pass/c-ast2ir/scope-on-scope-fail.cpp"
	"details/pass/c-ast2ir/scope-on.cpp"
	"details/pass/c-ast2ir/scope-return-raise.cpp"
	"details/pass/c-ast2ir/scope-scope.cpp"
	"details/pass/c-ast2ir/scope-stmt.cpp"
	"details/pass/c-ast2ir/scope-string.cpp"
	"details/pass/c-ast2ir/scope-switch.cpp"
	"details/pass/c-ast2ir/scope-type.cpp"
	"details/pass/c-ast2ir/scope-typedef.cpp"
	"details/pass/c-ast2ir/scope-typeof.cpp"
	"details/pass/c-ast2ir/scope-unittest.cpp"
	"details/pass/c-ast2ir/scope-var.cpp"
	"details/pass/c-ast2ir/scope-while.cpp"
	"details/pass/c-ast2ir/scope.cpp"
	"details/pass/c-ast2ir/scope.h"
	"details/pass/c-ast2ir/scope.hxx"
	"details/pass/c-ast2ir/source-ast-to-ir.cpp"
	"details/pass/c-ast2ir/source-ast-to-ir.h"
	"details/pass/d-object-map/attach.cpp"
	"details/pass/d-object-map/mapping.cpp"
	"details/pass/d-object-map/mapping.h"
	"details/program/program.h"
	"details/reporting/fwd.h"
	"details/reporting/levels.h"
	"details/reporting/message.cpp"
	"details/reporting/message.h"
	"details/reporting/report.cpp"
	"details/reporting/report.h"
	"details/reporting/report.hxx"
	"details/semantic/assignment.cpp"
	"details/semantic/atom-factory.cpp"
	"details/semantic/atom-factory.h"
	"details/semantic/capture-variable.cpp"
	"details/semantic/debug.cpp"
	"details/semantic/debug.h"
	"details/semantic/deprecated-error.cpp"
	"details/semantic/deprecated-error.h"
	"details/semantic/error-not-declared.cpp"
	"details/semantic/exception.cpp"
	"details/semantic/exception.h"
	"details/semantic/func-overload-match.cpp"
	"details/semantic/func-overload-match.h"
	"details/semantic/intrinsics-lang.cpp"
	"details/semantic/intrinsics-lang.cpp"
	"details/semantic/intrinsics-nanyc.cpp"
	"details/semantic/intrinsics-nanyc.cpp"
	"details/semantic/intrinsics.h"
	"details/semantic/intrinsics.h"
	"details/semantic/member-variable-default-clone.cpp"
	"details/semantic/member-variable-default-dispose.cpp"
	"details/semantic/member-variable-default-init.cpp"
	"details/semantic/member-variable.h"
	"details/semantic/opcode-alias.cpp"
	"details/semantic/opcode-allocate.cpp"
	"details/semantic/opcode-assign.cpp"
	"details/semantic/opcode-blueprint.cpp"
	"details/semantic/opcode-call.cpp"
	"details/semantic/opcode-commontype.cpp"
	"details/semantic/opcode-ensure-resolved.cpp"
	"details/semantic/opcode-follow.cpp"
	"details/semantic/opcode-identify.cpp"
	"details/semantic/opcode-intrinsic.cpp"
	"details/semantic/opcode-jzraise.cpp"
	"details/semantic/opcode-onscopefail.cpp"
	"details/semantic/opcode-pragma.cpp"
	"details/semantic/opcode-push.cpp"
	"details/semantic/opcode-raise.cpp"
	"details/semantic/opcode-ref-unref.cpp"
	"details/semantic/opcode-return.cpp"
	"details/semantic/opcode-scope-end.cpp"
	"details/semantic/opcode-self.cpp"
	"details/semantic/opcode-sizeof.cpp"
	"details/semantic/opcode-stackalloc.cpp"
	"details/semantic/opcode-stacksize.cpp"
	"details/semantic/opcode-store.cpp"
	"details/semantic/opcode-typeisobject.cpp"
	"details/semantic/overloaded-func-call-resolution.cpp"
	"details/semantic/overloaded-func-call-resolution.h"
	"details/semantic/ref-unref.h"
	"details/semantic/semantic-analysis.cpp"
	"details/semantic/semantic-analysis.h"
	"details/semantic/semantic-analysis.hxx"
	"details/semantic/stack-frame.cpp"
	"details/semantic/stack-frame.h"
	"details/semantic/stack-frame.hxx"
	"details/semantic/type-check.cpp"
	"details/semantic/type-check.h"
	"details/semantic/update-all-stackalloc.cpp"
	"details/type/builtin.h"
	"details/utils/check-for-valid-identifier-name.cpp"
	"details/utils/check-for-valid-identifier-name.h"
	"details/utils/clid.cpp"
	"details/utils/clid.h"
	"details/utils/clid.hxx"
	"details/utils/dataregister.h"
	"details/utils/memory-allocator.h"
	"details/utils/origin.h"
	"details/utils/stringrefs.cpp"
	"details/utils/stringrefs.h"
	"details/utils/stringrefs.hxx"
	"details/vm/allocator.h"
	"details/vm/io.cpp"
	"details/vm/io.h"
	"details/vm/machine.cpp"
	"details/vm/machine.h"
	"details/vm/runtime/std.console.cpp"
	"details/vm/runtime/std.core.h"
	"details/vm/runtime/std.core.string.cpp"
	"details/vm/runtime/std.digest.cpp"
	"details/vm/runtime/std.env.cpp"
	"details/vm/runtime/std.io.cpp"
	"details/vm/runtime/std.memory.cpp"
	"details/vm/runtime/std.os.process.cpp"
	"details/vm/stack.cpp"
	"details/vm/stack.h"
	"details/vm/stack.hxx"
	"details/vm/stacktrace.cpp"
	"details/vm/stacktrace.h"
	"details/vm/stacktrace.hxx"
	"details/vm/thread.cpp"
	"details/vm/thread.h"
	"details/vm/types.h"
	"include/nanyc/allocator.h"
	"include/nanyc/console.h"
	"include/nanyc/io.h"
	"include/nanyc/library.h"
	"include/nanyc/nanyc.h"
	"include/nanyc/program.h"
	"include/nanyc/report.h"
	"include/nanyc/types.h"
	"include/nanyc/utils.h"
	"include/nanyc/vm.h"
	"libnanyc.h"
	#
	"allocator.cpp"
	"bugreport.cpp"
	"console.cpp"
	"library.c"
	"nanyc.c"
	"program.cpp"
	"report.cpp"
	"utils.cpp"
	"vm.cpp"
)

set_target_properties(libnanyc PROPERTIES
	VERSION   "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	SOVERSION "${nany_version_major}"
)

set_target_properties(libnanyc-grammar PROPERTIES
	VERSION   "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	SOVERSION "${nany_version_major}"
)

target_link_libraries(libnanyc-grammar PRIVATE yuni-static-core)
target_link_libraries(libnanyc PRIVATE yuni-static-core libnanyc-grammar dyncall_s)

set_property(TARGET libnanyc PROPERTY MACOSX_RPATH true)
set_property(TARGET libnanyc PROPERTY LIBRARY_OUTPUT_NAME "nanyc")
set_property(TARGET libnanyc PROPERTY ARCHIVE_OUTPUT_NAME "nanyc")
set_property(TARGET libnanyc PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../")
set_property(TARGET libnanyc PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../")

set_property(TARGET libnanyc-grammar PROPERTY MACOSX_RPATH true)
set_property(TARGET libnanyc-grammar PROPERTY LIBRARY_OUTPUT_NAME "nanyc-grammar")
set_property(TARGET libnanyc-grammar PROPERTY ARCHIVE_OUTPUT_NAME "nanyc-grammar")

target_compile_definitions(libnanyc PRIVATE "LIBNANYC_DLL_EXPORT=1")

target_include_directories(libnanyc PRIVATE "${libnany_config_folder}")
target_include_directories(libnanyc PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../ext/dyncall/")
target_include_directories(libnanyc PUBLIC  "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(libnanyc PUBLIC  "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(libnanyc PUBLIC  "${CMAKE_CURRENT_BINARY_DIR}/include")
target_include_directories(libnanyc PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(libnanyc PRIVATE "${__embed_nsl_hxx_dir}")
target_include_directories(libnanyc-grammar PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

add_dependencies(libnanyc nanyc-grammar-cpp)
add_dependencies(libnanyc nyt-nanyfile-to-cstring)

install(
	TARGETS libnanyc
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "libnanyc"
)

if (YUNI_COMPILER_IS_CLANG)
	function(add_clang_static_analysis target)
	    get_target_property(SRCs ${target} SOURCES)
	    add_library(${target}_analyze OBJECT EXCLUDE_FROM_ALL ${SRCs})
		set_target_properties(${target}_analyze PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD true)
		set_target_properties(${target}_analyze PROPERTIES COMPILE_FLAGS "--analyze")
		target_include_directories(${target}_analyze PRIVATE "${libnany_config_folder}")
		target_include_directories(${target}_analyze PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../ext/dyncall/")
		target_include_directories(${target}_analyze PUBLIC  "${CMAKE_CURRENT_LIST_DIR}")
		add_dependencies(${target}_analyze nanyc-grammar-cpp)
		add_dependencies(${target}_analyze nyt-nanyfile-to-cstring)
	endfunction()
	add_clang_static_analysis(libnanyc)
endif()
