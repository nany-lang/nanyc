
macro(string_trim var str)
	set(${var} "")
	if (NOT "${ARGN}" STREQUAL "NOUNQUOTE")
		# Need not trim a quoted string.
		string_unquote(_var "${str}")
		if (NOT _var STREQUAL "")
			# String is quoted
			set(${var} "${_var}")
		endif()
	endif()

	if (${var} STREQUAL "")
		set(_var_1 "${str}")
		string(REGEX REPLACE  "^[ \t\r\n]+" "" _var_2 "${str}" )
		string(REGEX REPLACE  "[ \t\r\n]+$" "" _var_3 "${_var_2}" )
		set(${var} "${_var_3}")
	endif()
endmacro()


# Internal macro
# Variable cannot be escaped here, as variable is already substituted
# at the time it passes to this macro.
macro(string_escape var str)
	# ';' and '\' are tricky, need to be encoded.
	# '#' => '#H'
	# '\' => '#B'
	# ';' => '#S'
	set(_NOESCAPE_SEMICOLON "")
	set(_NOESCAPE_HASH "")

	foreach(_arg ${ARGN})
		if (${_arg} STREQUAL "NOESCAPE_SEMICOLON")
			set(_NOESCAPE_SEMICOLON "NOESCAPE_SEMICOLON")
		elseif (${_arg} STREQUAL "NOESCAPE_HASH")
			set(_NOESCAPE_HASH "NOESCAPE_HASH")
		endif()
	endforeach()

	if (_NOESCAPE_HASH STREQUAL "")
		string(REGEX REPLACE "#" "#H" _ret "${str}")
	else(_NOESCAPE_HASH STREQUAL "")
		set(_ret "${str}")
	endif()

	string(REGEX REPLACE "\\\\" "#B" _ret "${_ret}")
	if (_NOESCAPE_SEMICOLON STREQUAL "")
		string(REGEX REPLACE ";" "#S" _ret "${_ret}")
	endif()

	set(${var} "${_ret}")
endmacro()



macro(string_unescape var str)
	# '#B' => '\'
	# '#H' => '#'
	# '#D' => '$'
	# '#S' => ';'
	set(_ESCAPE_VARIABLE "")
	set(_NOESCAPE_SEMICOLON "")
	set(_ret "${str}")
	foreach(_arg ${ARGN})
		if (${_arg} STREQUAL "NOESCAPE_SEMICOLON")
			set(_NOESCAPE_SEMICOLON "NOESCAPE_SEMICOLON")
		elseif (${_arg} STREQUAL "ESCAPE_VARIABLE")
			set(_ESCAPE_VARIABLE "ESCAPE_VARIABLE")
			string(REGEX REPLACE "#D" "$" _ret "${_ret}")
		endif()
	endforeach()

	string(REGEX REPLACE "#B" "\\\\" _ret "${_ret}")
	if (_NOESCAPE_SEMICOLON STREQUAL "")
		# ';' => '#S'
		string(REGEX REPLACE "#S" "\\\\;" _ret "${_ret}")
	else(_NOESCAPE_SEMICOLON STREQUAL "")
		string(REGEX REPLACE "#S" ";" _ret "${_ret}")
	endif()

	if (NOT _ESCAPE_VARIABLE STREQUAL "")
		# '#D' => '$'
		string(REGEX REPLACE "#D" "$" _ret "${_ret}")
	endif()
	string(REGEX REPLACE "#H" "#" _ret "${_ret}")
	set(${var} "${_ret}")
endmacro()




macro(string_unquote var str)
	string_escape(_ret "${str}" ${ARGN})
	if (_ret MATCHES "^[ \t\r\n]+")
		string(REGEX REPLACE "^[ \t\r\n]+" "" _ret "${_ret}")
	endif()
	if (_ret MATCHES "^\"")
		# Double quote
		string(REGEX REPLACE "\"\(.*\)\"[ \t\r\n]*$" "\\1" _ret "${_ret}")
	elseif (_ret MATCHES "^'")
		# Single quote
		string(REGEX REPLACE "'\(.*\)'[ \t\r\n]*$" "\\1" _ret "${_ret}")
	else(_ret MATCHES "^\"")
		set(_ret "")
	endif()

	# Unencoding
	string_unescape(${var} "${_ret}" ${ARGN})

	#	string(REGEX REPLACE "#B" "\\\\" _ret "${_ret}")
	#string(REGEX REPLACE "#S" "\\\\;" ${var} "${_ret}")
	#string(REGEX REPLACE "#H" "#" _ret "${_ret}")
endmacro()




macro(string_split var delimiter str)
	set(_max_tokens "")
	set(_NOESCAPE_SEMICOLON "")
	set(_ESCAPE_VARIABLE "")
	foreach(_arg ${ARGN})
		if (${_arg} STREQUAL "NOESCAPE_SEMICOLON")
			set(_NOESCAPE_SEMICOLON "NOESCAPE_SEMICOLON")
		elseif (${_arg} STREQUAL "ESCAPE_VARIABLE")
			set(_ESCAPE_VARIABLE "ESCAPE_VARIABLE")
		else(${_arg} STREQUAL "NOESCAPE_SEMICOLON")
			set(_max_tokens ${_arg})
		endif()
	endforeach()

	if (NOT _max_tokens)
		set(_max_tokens -1)
	endif()

	string_escape(_str "${str}" ${_NOESCAPE_SEMICOLON} ${_ESCAPE_VARIABLE})
	string_escape(_delimiter "${delimiter}" ${_NOESCAPE_SEMICOLON} ${_ESCAPE_VARIABLE})

	set(_str_list "")
	set(_token_count 0)
	string(LENGTH "${_delimiter}" _de_len)

	while (NOT _token_count EQUAL _max_tokens)
		math(EXPR _token_count ${_token_count}+1)
		if (_token_count EQUAL _max_tokens)
			# Last token, no need splitting
			set(_str_list ${_str_list} "${_str}")
		else(_token_count EQUAL _max_tokens)
			# in case encoded characters are delimiters
			string(LENGTH "${_str}" _str_len)
			set(_index 0)
			set(_token "")
			set(_str_remain "")
			math(EXPR _str_end ${_str_len}-${_de_len}+1)
			set(_bound "k")
			while (_index LESS _str_end)
				string(SUBSTRING "${_str}" ${_index} ${_de_len} _str_cursor)
				if (_str_cursor STREQUAL _delimiter)
					# Get the token
					string(SUBSTRING "${_str}" 0 ${_index} _token)
					# Get the rest
					math(EXPR _rest_index ${_index}+${_de_len})
					math(EXPR _rest_len ${_str_len}-${_index}-${_de_len})
					string(SUBSTRING "${_str}" ${_rest_index} ${_rest_len} _str_remain)
					set(_index ${_str_end})
				else(_str_cursor STREQUAL _delimiter)
					math(EXPR _index ${_index}+1)
				endif()
			endwhile(_index LESS _str_end)

			if (_str_remain STREQUAL "")
				# Meaning: end of string
				list(APPEND _str_list "${_str}")
				set(_max_tokens ${_token_count})
			else(_str_remain STREQUAL "")
				list(APPEND _str_list "${_token}")
				set(_str "${_str_remain}")
			endif()
		endif()
	endwhile()

	# Unencoding
	string_unescape(${var} "${_str_list}" ${_NOESCAPE_SEMICOLON} ${_ESCAPE_VARIABLE})
endmacro()


macro(system varname)
	execute_process(COMMAND ${ARGN} OUTPUT_VARIABLE ${varname}
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro()
