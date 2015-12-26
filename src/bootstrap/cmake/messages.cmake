
set(nany-bootstrap_error_has_occured   false)


macro(nmessage msg)
	if(UNIX)
		message(STATUS "[1;30m{nany-bootstrap}[0m  ${msg}")
	else()
		message(STATUS "{nany-bootstrap}  ${msg}")
	endif()

endmacro()


macro(nwarning msg)
	if(UNIX)
		message(STATUS "[1;33m{nany-bootstrap}  [warning][0m ${msg}")
	else()
		message(STATUS "{nany-bootstrap}  [WARNING] ${msg}")
	endif()
endmacro()


macro(nerror msg)
	if(UNIX)
		message(STATUS "[1;31m{nany-bootstrap}  [error][0m ${msg}")
	else()
		message(STATUS "{nany-bootstrap}  [ERROR] ${msg}")
	endif()
	set(nany-bootstrap_error_has_occured  true)
endmacro()



