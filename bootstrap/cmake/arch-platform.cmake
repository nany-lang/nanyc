# Architecture
include(CheckSymbolExists)
if(WIN32)
	check_symbol_exists("_M_AMD64" "" RTC_ARCH_X64)
	if(NOT RTC_ARCH_X64)
		check_symbol_exists("_M_IX86" "" RTC_ARCH_X86)
	endif(NOT RTC_ARCH_X64)
	# add check for arm here
	# see http://msdn.microsoft.com/en-us/library/b0084kay.aspx
else(WIN32)
	check_symbol_exists("__i386__" "" RTC_ARCH_X86)
	check_symbol_exists("__x86_64__" "" RTC_ARCH_X64)
	check_symbol_exists("__arm__" "" RTC_ARCH_ARM)
endif(WIN32)


# Underlying platform
set(nany_os_debian 0)
if (NOT WIN32)
	set(nany_os_windows 0)
	if (EXISTS "/etc/debian_version")
		set(nany_os_debian 1)
		system(nany_arch COMMAND dpkg --print-architecture)
	endif()
else()
	set(nany_os_windows 1)
endif()
