set(__pkg_deb_tmp "${CMAKE_CURRENT_BINARY_DIR}")

execute_process(
	COMMAND "dpkg" "--print-architecture"
	OUTPUT_VARIABLE deb_arch
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(PACKAGE_VERSION "${nany_version}")

set(PACKAGE_NAME "libnanyc")
set(PACKAGE_DESCRIPTION "Nanyc common runtime library")
set(PACKAGE_ARCH "${deb_arch}")
set(PACKAGE_SECTION "libs")
set(PACKAGE_DEBS "")
configure_file("../distrib/deb-control-debian.template" "${__pkg_deb_tmp}/deb-control-libnanyc")
install(
	FILES "${__pkg_deb_tmp}/deb-control-libnanyc"
	DESTINATION "DEBIAN"
	RENAME "control"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	COMPONENT "libnanyc"
)
