# Runs at install time to copy in frameworks from the host system if needed.
# bundle_dependency_dirs should be set by a install(CODE) fragment

if (CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 3.3)
	cmake_minimum_required(VERSION 3.3)
endif ()

include(BundleUtilities)

# Pull list of bundles to fixup from the installed files list
set(bundles "")
foreach (file IN LISTS CMAKE_INSTALL_MANIFEST_FILES)
	if (file MATCHES "(.+\\.app)/")
		if (NOT CMAKE_MATCH_1 IN_LIST bundles)
			list(APPEND bundles ${CMAKE_MATCH_1})
		endif ()
	endif ()
endforeach ()

foreach (bundle IN LISTS bundles)
	fixup_bundle($ENV{DESTDIR}${bundle} "" "${bundle_dependency_dirs}")
endforeach ()
