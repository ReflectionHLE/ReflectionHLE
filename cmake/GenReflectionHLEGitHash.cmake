execute_process(COMMAND git describe --tags
                RESULT_VARIABLE result
                OUTPUT_VARIABLE git_desc
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
if ((result AND NOT result EQUAL 0) OR
    NOT (${git_desc} MATCHES "^release[-][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]"))
	set(git_id "<UnknownRev>")
else ()
	string(LENGTH "${git_desc}" len)
	# If length == 16, assume release-YYYYMMDD and don't
	# define macro. We'll use an internal version number instead.
	if (len EQUAL 16)
		set(git_id "")
	# Otherwise, assume release-YYYYMMDD-COUNT-gHASH
	# and extract parts of the string.
	else ()
		string(REGEX MATCH "[-][^-]*[-][^-]*$" git_id "${git_desc}")
		string(SUBSTRING "${git_id}" 1 -1 git_id)
	endif ()
endif ()

set(REFKEEN_GIT_ID ${git_id})
configure_file(${IN_FILE} ${OUT_FILE})
