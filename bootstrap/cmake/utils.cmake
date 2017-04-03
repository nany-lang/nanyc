macro(system varname)
	set(options)
	set(oneValueArgs)
	set(multiValueArgs COMMAND)
	cmake_parse_arguments(opts "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	execute_process(COMMAND ${opts_COMMAND} OUTPUT_VARIABLE ${varname}
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro()
