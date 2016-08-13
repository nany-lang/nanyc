macro(create_rules_for_packages output)
	if (NOT WIN32 AND NOT APPLE)
		set(destdir "/tmp/__nany-install-${nany_version}/")
		set(dev "dev")
		if ("${output}" STREQUAL "rpm")
			set(dev "devel")
		endif()

		add_custom_target(package-${output}
			DEPENDS libnanyc nany nanyc-check-syntax nanyc-dump-ast
			COMMAND rm -rf "${destdir}"

			COMMAND "${CMAKE_COMMAND}" "-E" echo ""
			COMMAND "${CMAKE_COMMAND}" "-E" echo "notes: to install 'fpm' on debian:"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           [sudo] apt-get install ruby-dev gcc make"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           [sudo] gem install fpm"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "       to install 'fpm' on redhat/centos:"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           [sudo] yum install ruby-devel gcc make"
			COMMAND "${CMAKE_COMMAND}" "-E" echo "           [sudo] gem install fpm"
			COMMAND "${CMAKE_COMMAND}" "-E" echo ""

			COMMAND "DESTDIR=${destdir}/libnanyc" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnanyc"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				--name libnanyc
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor "Nany Team"
				--description "Nany Compiler Library"
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--verbose
				-p "../../distrib/"
				-C "${destdir}/libnanyc"

			COMMAND "DESTDIR=${destdir}/nany" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=nanyc"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				--name nanyc
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor "Nany Team"
				--description "Nany Programming Language"
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--depends libnanyc
				--verbose
				-p "../../distrib/"
				-C "${destdir}/nany"

			COMMAND "DESTDIR=${destdir}/dev" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnanyc-dev"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				-a all 
				--name libnanyc-${dev}
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor "Nany Team"
				--description "Nany Library Headers"
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--depends libnanyc
				--verbose
				-p "../../distrib/"
				-C "${destdir}/dev"

			COMMAND "DESTDIR=${destdir}/dev-tools" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=libnanyc-dev-tools"
				-P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
			COMMAND fpm -f -s dir -t ${output}
				-a all 
				--name libnanyc-${dev}-tools
				--version "\"${nany_version}\""
				--url "\"${nany_website}\""
				--vendor "Nany Team"
				--description "Nany Compiler Dev Tools"
				--license "${nany_license}"
				--maintainer "${nany_contact}"
				--depends libnanyc
				--verbose
				-p "../../distrib/"
				-C "${destdir}/dev-tools"

			COMMAND rm -rf "${destdir}")
	endif()
endmacro()


