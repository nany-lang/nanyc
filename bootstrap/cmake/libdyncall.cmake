if (MSVC)
	# DynCALL does not like SAFESEH on Windows x86
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
endif()

# -- dyncall - (Generic Dynamic FFI package / http://www.dyncall.org/)
add_subdirectory("ext/dyncall")
