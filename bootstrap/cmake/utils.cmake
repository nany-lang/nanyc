macro(system varname)
	execute_process(COMMAND ${ARGN} OUTPUT_VARIABLE ${varname}
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro()
