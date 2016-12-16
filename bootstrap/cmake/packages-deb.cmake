macro (_generate_deb_package pkgname)
	set(debfile "${pkgname}_${nany_version}-0_${nany_arch}")
	set(pkgname "${pkgname}")
	set(builddir "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/deb/${debfile}")
	file(MAKE_DIRECTORY "${builddir}/DEBIAN")
	configure_file("../distrib/deb/${pkgname}.control.template" "${builddir}/DEBIAN/control" @ONLY)
endmacro()


function (generate_rules_dpk_deb)
	message(STATUS "generating rules for DEB packages")
	set(ver "${nany_version_major}.${nany_version_minor}")
	_generate_deb_package(libnanyc)
	file(MAKE_DIRECTORY "${builddir}/usr/lib")
	add_custom_target(package-deb-${pkgname}
		DEPENDS libnanyc nanyc nanyc-check-syntax nanyc-dump-ast
		COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:libnanyc>" "${builddir}/usr/lib"
		COMMAND dpkg-deb --build "${debfile}" "${CMAKE_BINARY_DIR}/../distrib"
		VERBATIM WORKING_DIRECTORY "${builddir}/..")

	_generate_deb_package(libnanyc-dev)
	file(MAKE_DIRECTORY "${builddir}/usr/include/nanyc/${ver}/nany")
	file(COPY "${CMAKE_CURRENT_LIST_DIR}/../libnanyc/include/nany/nany.h"
		DESTINATION "${builddir}/usr/include/nanyc/${ver}/nany")
	add_custom_target(package-deb-${pkgname}
		DEPENDS libnanyc nanyc nanyc-check-syntax nanyc-dump-ast
		COMMAND dpkg-deb --build "${debfile}" "${CMAKE_BINARY_DIR}/../distrib"
		VERBATIM WORKING_DIRECTORY "${builddir}/..")

	_generate_deb_package(nanyc)
	file(MAKE_DIRECTORY "${builddir}/usr/bin")
	add_custom_target(package-deb-${pkgname}
		DEPENDS libnanyc nanyc nanyc-check-syntax nanyc-dump-ast
		COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:nanyc>" "${builddir}/usr/bin"
		COMMAND dpkg-deb --build "${debfile}" "${CMAKE_BINARY_DIR}/../distrib"
		VERBATIM WORKING_DIRECTORY "${builddir}/..")

	_generate_deb_package(nanyc-extras)
	file(MAKE_DIRECTORY "${builddir}/usr/bin")
	add_custom_target(package-deb-${pkgname}
		DEPENDS libnanyc nanyc nanyc-check-syntax nanyc-dump-ast nanyc-unittest
		COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:nanyc-check-syntax>" "${builddir}/usr/lib"
		COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:nanyc-dump-ast>" "${builddir}/usr/lib"
		COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:nanyc-unittest>" "${builddir}/usr/lib"
		COMMAND dpkg-deb --build "${debfile}" "${CMAKE_BINARY_DIR}/../distrib"
		VERBATIM WORKING_DIRECTORY "${builddir}/..")

	add_custom_target(packages-deb DEPENDS
		package-deb-libnanyc
		package-deb-nanyc
		package-deb-nanyc-extras
		package-deb-libnanyc-dev)
endfunction()


generate_rules_dpk_deb()
