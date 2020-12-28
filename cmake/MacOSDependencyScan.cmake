# Public domain

if (CMAKE_VERSION VERSION_LESS 3.3)
	function (macos_dependency_scan output)
		message(WARNING "CMake 3.3 or newer required for macos_dependency_scan to work")
	endfunction ()
	return()
endif ()

# Enable IN_LIST
cmake_policy(SET CMP0057 NEW)

# Takes targets to scan as additional arguments. The linked libraries on these
# targets will be scanned for macOS frameworks. The directory that contains
# these frameworks will be placed in output as well as any frameworks contained
# within. The output of this function can then be fed into fixup_bundle.
#
# This should probably be recursive among detecting other possible ways
# frameworks could be referenced in the link line, but this should cover the
# basic use cases and otherwise be harmless.
function(macos_dependency_scan output)
	list(POP_FRONT ARGV)

	foreach (target IN LISTS ARGV)
		get_property(target_links TARGET ${target} PROPERTY LINK_LIBRARIES)
		foreach (link IN LISTS target_links)
			if (link MATCHES "^((.*/)[^/]+\\.framework)")
				set(subframeworks "${CMAKE_MATCH_1}/Versions/Current/Frameworks")
				set(framework_dir ${CMAKE_MATCH_2})
				if (IS_DIRECTORY "${subframeworks}" AND NOT subframeworks IN_LIST bundle_dependency_dirs)
					list(APPEND bundle_dependency_dirs ${subframeworks})
				endif ()
				if (NOT framework_dir IN_LIST bundle_dependency_dirs)
					list(APPEND bundle_dependency_dirs ${framework_dir})
				endif ()
			endif ()
		endforeach()
	endforeach ()

	set("${output}" "${bundle_dependency_dirs}" PARENT_SCOPE)
endfunction()
