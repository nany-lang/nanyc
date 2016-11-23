macro(create_rules_for_package_msi)
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
		set(wix_config_mode "Debug")
	else()
		set(wix_config_mode "Release")
	endif()

	if (RTC_ARCH_ARM)
		set(wix_arch "arm")
	elseif (RTC_ARCH_X64)
		set(wix_arch "x64")
	elseif (RTC_ARCH_X86)
		set(wix_arch "x86")
	else()
		set(wix_arch "unknown")
	endif()

	set(wix_ver "3.10")
	set(wix_wxs "nanyc-${nany_version}-${wix_arch}")
	configure_file("../distrib/wix/cmake-wix-template.cmake" "../distrib/${wix_wxs}.wxs" @ONLY)

	add_custom_target(package-msi
		DEPENDS libnanyc nany nanyc-check-syntax nanyc-dump-ast
		WORKING_DIRECTORY "../distrib"
		COMMAND "${CMAKE_COMMAND}" "-E" echo "-- wix candle ${wix_wxs}.wxs" 
		COMMAND "C:\\Program Files (x86)\\WiX Toolset v${wix_ver}\\bin\\candle.exe" "${wix_wxs}.wxs"

		COMMAND "${CMAKE_COMMAND}" "-E" echo "-- wix light ${wix_wxs}.wixobj" 
		COMMAND "C:\\Program Files (x86)\\WiX Toolset v${wix_ver}\\bin\\light.exe"
		"-out" "nanyc-${nany_version}-${wix_arch}.msi" -ext WixUIExtension "${wix_wxs}.wixobj")
endmacro()
