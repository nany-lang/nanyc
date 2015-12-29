project(libnany)


get_filename_component(libnany_config_folder "${CMAKE_CURRENT_BINARY_DIR}" REALPATH)
file(MAKE_DIRECTORY "${libnany_config_folder}")

if (NOT EXISTS "${libnany_config_folder}/libnany-config.h")
	nmessage("generating 'libnany-config.h' in ${libnany_config_folder}/libnany-config.h")
	configure_file("config.h.cmake" "${libnany_config_folder}/libnany-config.h" @ONLY)
else()
	nmessage("ignoring libnany-config.h ${libnany_config_folder}/libnany-config.h (already exists)")
endif()


macro(addsrc)
	set(options)
	set(oneValueArgs NAME GROUP)
	set(multiValueArgs FILES)
	cmake_parse_arguments(O "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set("${O_NAME}" ${O_FILES})
	source_group("${O_GROUP}" FILES ${O_FILES})
endmacro()








addsrc(
	NAME  nysrc_grammar
	GROUP "grammar"
	FILES ${NANY_YGR})
addsrc(
	NAME  nysrc_details_grammar
	GROUP "details/grammar"
	FILES ${NANY_GRAMMAR_H} ${NANY_GRAMMAR_HXX} ${NANY_GRAMMAR_CPP})


addsrc(
	NAME  nysrc_details_pass_build_ast_normalize
	GROUP "details/pass/build-ast-normalize"
	FILES
	"details/pass/build-ast-normalize/ast-normalize.cpp")


# pass - build - ast from source
addsrc(
	NAME  nysrc_details_pass_build_ast_from_source
	GROUP "details/pass/build-ast-from=source"
	FILES
	"details/pass/build-ast-from-source/ast-from-source.cpp")


addsrc(
	NAME  nysrc_details_pass_build_ast_to_ir
	GROUP "details/pass/build-ast-to-ir"
	FILES
	"details/pass/build-ast-to-ir/producer.h"
	"details/pass/build-ast-to-ir/context.h"
	"details/pass/build-ast-to-ir/context.hxx"
	"details/pass/build-ast-to-ir/context.cpp"
	"details/pass/build-ast-to-ir/scope.h"
	"details/pass/build-ast-to-ir/scope.hxx"
	"details/pass/build-ast-to-ir/scope.cpp"
	"details/pass/build-ast-to-ir/scope-func.cpp"
	"details/pass/build-ast-to-ir/scope-typeof.cpp"
	"details/pass/build-ast-to-ir/scope-new.cpp"
	"details/pass/build-ast-to-ir/scope-var.cpp"
	"details/pass/build-ast-to-ir/scope-class.cpp"
	"details/pass/build-ast-to-ir/scope-expr.cpp"
	"details/pass/build-ast-to-ir/scope-call.cpp"
	"details/pass/build-ast-to-ir/scope-return.cpp"
	"details/pass/build-ast-to-ir/scope-stmt.cpp"
	"details/pass/build-ast-to-ir/scope-scope.cpp"
	"details/pass/build-ast-to-ir/scope-type.cpp"
	"details/pass/build-ast-to-ir/scope-number.cpp"
	"details/pass/build-ast-to-ir/scope-string.cpp"
	"details/pass/build-ast-to-ir/scope-intrinsic.cpp"
	"details/pass/build-ast-to-ir/source-ast-to-ir.cpp")


addsrc(
	NAME  nysrc_details_pass_build_attach
	GROUP "details/pass/build-attach"
	FILES
	"details/pass/build-attach-program/attach.cpp")


addsrc(
	NAME  nysrc_details_pass_build_instanciate
	GROUP "details/pass/build-instanciate"
	FILES
	"details/pass/build-instanciate/instanciate.cpp"
	"details/pass/build-instanciate/instanciate-atom.cpp"
	"details/pass/build-instanciate/instanciate.h"
	"details/pass/build-instanciate/instanciate.hxx"
	"details/pass/build-instanciate/stack-frame.h"
	"details/pass/build-instanciate/stack-frame.hxx"
	"details/pass/build-instanciate/instanciate-identify.cpp"
	"details/pass/build-instanciate/instanciate-self.cpp"
	"details/pass/build-instanciate/instanciate-allocate.cpp"
	"details/pass/build-instanciate/instanciate-stacksize.cpp"
	"details/pass/build-instanciate/instanciate-alias.cpp"
	"details/pass/build-instanciate/instanciate-store.cpp"
	"details/pass/build-instanciate/instanciate-builtin.cpp"
	"details/pass/build-instanciate/instanciate-call.cpp"
	"details/pass/build-instanciate/instanciate-sizeof.cpp"
	"details/pass/build-instanciate/instanciate-error.cpp"
	"details/pass/build-instanciate/instanciate-intrinsic.cpp"
	"details/pass/build-instanciate/instanciate-intrinsic-builtin.cpp"
	"details/pass/build-instanciate/instanciate-return.cpp"
	"details/pass/build-instanciate/instanciate-scope.cpp"
	"details/pass/build-instanciate/instanciate-pragma.cpp"
	"details/pass/build-instanciate/instanciate-assignment.cpp"
	"details/pass/build-instanciate/instanciate-ref-unref.cpp"
	"details/pass/build-instanciate/instanciate-assign.cpp"
	"details/pass/build-instanciate/instanciate-typeisobject.cpp"
	"details/pass/build-instanciate/instanciate-member-variable-default-init.cpp"
	"details/pass/build-instanciate/instanciate-member-variable-default-dispose.cpp"
	"details/pass/build-instanciate/instanciate-member-variable-default-clone.cpp"
	"details/pass/build-instanciate/overloaded-func-call-resolution.cpp"
	"details/pass/build-instanciate/overloaded-func-call-resolution.h"
	"details/pass/build-instanciate/suggestion-not-declared-in-scope.cpp")


addsrc(
	NAME  nysrc_details_vm
	GROUP "details/vm"
	FILES
	"details/vm/vm.cpp"
	"details/vm/memchecker.h"
	"details/vm/memchecker.cpp"
	"details/vm/stacktrace.h"
	"details/vm/stacktrace.cpp"
	"details/vm/vm.h")


addsrc(
	NAME  nysrc_details_utils
	GROUP "details/utils"
	FILES
	"details/utils/origin.h"
	"details/utils/stringrefs.h"
	"details/utils/stringrefs.hxx"
	"details/utils/stringrefs.cpp"
	"details/utils/check-for-valid-identifier-name.cpp"
	"details/utils/check-for-valid-identifier-name.h"
	"details/utils/clid.h"
	"details/utils/clid.hxx")


addsrc(
	NAME  nysrc_details_ast
	GROUP "details/ast"
	FILES
	"details/ast/tree-index.h"
	"details/ast/tree-index.hxx"
	"details/ast/tree-index.cpp"
	"details/ast/ast.h"
	"details/ast/ast.hxx"
	"details/ast/ast.cpp")


addsrc(
	NAME  nysrc_details_ir
	GROUP "details/ir"
	FILES
	"details/ir/ir.h"
	"details/ir/isa/opcodes.h"
	"details/ir/isa/opcodes.cpp"
	"details/ir/isa/data.h"
	"details/ir/isa/data.cpp"
	"details/ir/isa/printer.inc.hpp"
	"details/ir/scope-locker.h"
	"details/ir/program.h"
	"details/ir/program.hxx"
	"details/ir/program.cpp")


addsrc(
	NAME  nysrc_details_atom_classdef
	GROUP "details/atom/classdef"
	FILES
	"details/atom/classdef.h"
	"details/atom/classdef.hxx"
	"details/atom/classdef.cpp"
	"details/atom/classdef-table.h"
	"details/atom/classdef-table.hxx"
	"details/atom/classdef-table.cpp"
	"details/atom/classdef-table-is-similar.cpp"
	"details/atom/classdef-table-view.h"
	"details/atom/classdef-table-view.hxx"
	"details/atom/classdef-table-view.cpp"
	"details/atom/classdef-follow.h"
	"details/atom/classdef-follow.hxx"
	"details/atom/classdef-follow.cpp"
	"details/atom/classdef-overloads.h"
	"details/atom/classdef-overloads.hxx"
	"details/atom/classdef-overloads.cpp")


addsrc(
	NAME  nysrc_details_atom
	GROUP "details/atom"
	FILES
	"details/atom/funcdef.h"
	"details/atom/funcdef.cpp"
	"details/atom/funcdef.hxx"
	"details/atom/type.h"
	"details/atom/qualifiers.h"
	"details/atom/qualifiers.hxx"
	"details/atom/interface.h"
	"details/atom/interface.cpp"
	"details/atom/interface.hxx"
	"details/atom/vardef.h"
	"details/atom/vardef.hxx"
	"details/atom/vardef.cpp"
	"details/atom/local-variables.h"
	"details/atom/local-variables.hxx"
	"details/atom/local-variables.cpp"
	"details/atom/atom.h"
	"details/atom/atom.hxx"
	"details/atom/atom.cpp"
	"details/atom/atom-map.h"
	"details/atom/atom-map.hxx"
	"details/atom/atom-map.cpp"
	"details/atom/signature.h"
	"details/atom/signature.cpp"
	"details/atom/signature.hxx"
	"details/atom/func-overload-match.cpp"
	"details/atom/func-overload-match.h"
	"details/atom/func-overload-match.hxx")


addsrc(
	NAME  nysrc_details_intrinsic
	GROUP "details/intrinsic"
	FILES
	"details/intrinsic/intrinsic.h"
	"details/intrinsic/intrinsic-table.h"
	"details/intrinsic/intrinsic-table.hxx"
	"details/intrinsic/intrinsic-table.cpp"
	"details/intrinsic/register-bool.cpp")


addsrc(
	NAME  nysrc_details_context
	GROUP "details/context"
	FILES
	"details/context/context.h"
	"details/context/context.hxx"
	"details/context/context.cpp"
	"details/context/context-build.cpp"
	"details/context/build-info.h"
	"details/context/build-info.cpp"
	"details/context/target.h"
	"details/context/target.hxx"
	"details/context/target.cpp"
	"details/context/source.h"
	"details/context/source.hxx"
	"details/context/source.cpp"
	"details/context/isolate.h"
	"details/context/isolate.cpp")


addsrc(
	NAME  nysrc_details_reporting
	GROUP "details/reporting"
	FILES
	"details/reporting/fwd.h"
	"details/reporting/message.h"
	"details/reporting/message.cpp"
	"details/reporting/report.cpp"
	"details/reporting/report.h"
	"details/reporting/report.hxx"
	"details/reporting/levels.h")


addsrc(
	NAME  nysrc_c_api
	GROUP "c-api"
	FILES
	"c-api/context.cpp"
	"c-api/print-ast.cpp"
	"c-api/queueservice.cpp"
	"c-api/report.cpp"
	"c-api/sandbox.cpp"
	"c-api/intrinsics.cpp"
	"c-api/source.cpp"
	"c-api/utilities.cpp"
	"c-api/validator.cpp"
	"c-api/bugreport.cpp"
	"c-api/visibility.cpp"
	"c-api/version.c"
	"c-api/common-debuginfo.hxx")





set(mode_libnany "SHARED")
if ("${NANY_STATIC}")
	set(mode_libnany "STATIC")
endif()


add_library(libnany ${mode_libnany}
	# public headers
	nany/nany.h
	nany/nany.hxx

	${nysrc_grammar}
	${nysrc_details_grammar}

	${nysrc_details_pass_build_ast_from_source}
	${nysrc_details_pass_build_ast_normalize}
	${nysrc_details_pass_build_ast_to_ir}
	${nysrc_details_pass_build_attach}
	${nysrc_details_pass_build_instanciate}

	${nysrc_details_vm}

	${nysrc_details_utils}
	${nysrc_details_ast}
	${nysrc_details_ir}
	${nysrc_details_atom_classdef}
	${nysrc_details_atom}
	${nysrc_details_intrinsic}
	${nysrc_details_context}
	${nysrc_details_reporting}

	${nysrc_c_api}

	# Type
	details/type/builtin.h
	details/type/builtin.cpp
	# Semantic Analysis
	details/sema/metadata.cpp
	details/sema/metadata.h
	# implementation - misc
	details/fwd.h

	# misc
	"${libnany_config_folder}/libnany-config.h")



set_target_properties(libnany PROPERTIES
	VERSION   "${nany_version_major}.${nany_version_minor}.${nany_version_patch}"
	SOVERSION "${nany_version_major}")

target_link_libraries(libnany PRIVATE yuni-static-core)
set_property(TARGET libnany PROPERTY MACOSX_RPATH true)
set_property(TARGET libnany PROPERTY LIBRARY_OUTPUT_NAME "nanyc")
set_property(TARGET libnany PROPERTY ARCHIVE_OUTPUT_NAME "nanyc")
set_property(TARGET libnany PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../")
set_property(TARGET libnany PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../")

target_compile_definitions(libnany PRIVATE "LIBNANY_DLL_EXPORT=1")


target_include_directories(libnany PRIVATE "${libnany_config_folder}")
target_include_directories(libnany PUBLIC  ".")

add_dependencies(libnany nany-grammar-cpp)

install(TARGETS libnany
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	COMPONENT "libnany")



# The end !
