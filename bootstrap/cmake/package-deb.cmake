set(__pkg_deb_tmp "${CMAKE_CURRENT_BINARY_DIR}")

execute_process(
	COMMAND "dpkg" "--print-architecture"
	OUTPUT_VARIABLE deb_arch
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(PACKAGE_VERSION "${nany_version}")

function (make_debian_control)
	set(options)
	set(oneValueArgs ARCH COMPONENT DESCRIPTION SECTION)
	set(multiValueArgs)
	cmake_parse_arguments(opts "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	set(PACKAGE_NAME "${opts_COMPONENT}")
	set(PACKAGE_DESCRIPTION "${opts_DESCRIPTION}")
	set(PACKAGE_ARCH "${opts_ARCH}")
	set(PACKAGE_SECTION "${opts_SECTION}")
	set(PACKAGE_DEBS "")
	set(output_file "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${__pkg_deb_tmp}/deb-control-${opts_COMPONENT}")
	configure_file("../distrib/deb-control-debian.template" "${output_file}")
	install(
		FILES "${output_file}"
		DESTINATION "DEBIAN"
		RENAME "control"
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
		COMPONENT "${opts_COMPONENT}"
	)
endfunction()

make_debian_control(
	COMPONENT "libnanyc"
	DESCRIPTION "Nanyc common runtime library"
	SECTION "libs"
	ARCH "${deb_arch}"
)

make_debian_control(
	COMPONENT "nanyc"
	DESCRIPTION "Nanyc interpretor"
	SECTION "devel"
	ARCH "${deb_arch}"
)
