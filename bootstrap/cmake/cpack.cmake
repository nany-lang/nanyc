set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_VERSION_MAJOR "${nany_version_major}")
set(CPACK_PACKAGE_VERSION_MINOR "${nany_version_minor}")
set(CPACK_PACKAGE_VERSION_PATCH "${nany_version_patch}")
set(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)

set(CPACK_PACKAGE_VENDOR   "Nany Team")
set(CPACK_PACKAGE_CONTACT  "${nany_contact}")

include(CPack)

cpack_add_install_type(insttype_default DISPLAY_NAME "Default")
cpack_add_install_type(insttype_full DISPLAY_NAME "Full")

cpack_add_component_group("libnany" DISPLAY_NAME "Nany Compiler Core")
cpack_add_component_group("nany" DISPLAY_NAME "Nany Compiler" EXPANDED BOLD_TITLE)
cpack_add_component_group("nany-dev" DISPLAY_NAME "Nany Compiler Headers")

cpack_add_component("libnany"
	REQUIRED
	GROUP "libnany"
	INSTALL_TYPES insttype_full insttype_default
)

cpack_add_component("libnany-dev"
	DEPENDS "libnany"
	GROUP "nany-dev"
	INSTALL_TYPES insttype_full
)

cpack_add_component("nany"
	DEPENDS "libnany"
	GROUP "nany"
	INSTALL_TYPES insttype_full insttype_default
)
