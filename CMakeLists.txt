cmake_minimum_required(VERSION 2.8)
project(Reflection-Keen)

function(add_refkeen_separate_target
	name targets libraries sources shared_sources
	include_dirs target_cc_flags common_cc_flags is_lib
)
	set(all_cc_flags ${target_cc_flags})
	list(TRANSFORM all_cc_flags REPLACE "_VER" "_HAS_VER")
	set(all_cc_flags ${target_cc_flags} ${all_cc_flags})
	if (is_lib)
		set(all_cc_flags ${common_cc_flags} ${all_cc_flags})
		list(REMOVE_DUPLICATES "all_cc_flags")
		string(REPLACE ";" " " all_cc_flags "${all_cc_flags}")
		add_library(${name} STATIC ${sources})
		set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${all_cc_flags}")
	else ()
		string(REPLACE ";" " " all_cc_flags "${all_cc_flags}")
		add_executable(${name} ${sources} ${shared_sources})
		set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${all_cc_flags}")
		target_link_libraries(${name} ${libraries})
		set(${targets} "${targets} ${name}" PARENT_SCOPE)
	endif ()
	target_include_directories(${name} PUBLIC ${include_dirs})
endfunction()

function(add_refkeen_unified_target name targets shared_sources cc_flags libraries)
	add_executable(${name} ${shared_sources})
	set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${cc_flags}")
	target_link_libraries(${name} ${libraries})
	set(${targets} "${targets} ${name}" PARENT_SCOPE)
endfunction()

include(CMakeDependentOption)

option(BUILD_AS_CPP "Build Reflection Keen as C++ code" OFF)
CMAKE_DEPENDENT_OPTION(
	UNIFIED "Build aggregated C++ EXEs covering multiple games/versions" ON
	"BUILD_AS_CPP" OFF
)

if (NOT BUILD_AS_CPP)
	if (CMAKE_VERSION VERSION_LESS "3.1")
		if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
			set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
		endif ()
	else ()
		set(CMAKE_C_EXTENSIONS OFF)
		set(CMAKE_C_STANDARD 99)
		set(CMAKE_C_STANDARD_REQUIRED ON)
	endif ()
endif ()

if (UNIFIED)
	set(REFKEEN_USE_NS "1")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
find_package(SDL2 REQUIRED)
find_package(SpeexDSP)
include_directories(${SDL2_INCLUDE_DIRS} ${SPEEXDSP_INCLUDE_DIRS})

if (SPEEXDSP_FOUND)
	set(REFKEEN_RESAMPLER_LIBSPEEXDSP "1")
else ()
	set(REFKEEN_RESAMPLER_NONE "1")
endif ()

option(REFKEEN_ENABLE_LAUNCHER "Enable the Reflection Keen launcher" ON)

include(src/CMakeLists.txt)
include(src/kdreams/CMakeLists.txt)
include(src/id91_11/cat_all/cat3d/CMakeLists.txt)
include(src/id91_11/cat_all/catabyss/CMakeLists.txt)
include(src/id91_11/cat_all/catarm/CMakeLists.txt)
include(src/id91_11/cat_all/catapoc/CMakeLists.txt)

set(all_src ${be_src} ${kdreams_src} ${cat3d_src}
    ${catabyss_src} ${catarm_src} ${catapoc_src})
list(REMOVE_DUPLICATES "all_src")

if (BUILD_AS_CPP)
	set_source_files_properties(${all_src} PROPERTIES LANGUAGE CXX)
endif ()

configure_file(refkeen_config.h.in refkeen_config.h)

set(LIBRARIES ${SDL2_LIBRARIES} ${SPEEXDSP_LIBRARIES} m)

include_directories(
	PUBLIC src src/crc32 src/depklite src/opl src/unlzexe
	rsrc "${PROJECT_BINARY_DIR}"
)

add_refkeen_separate_target(
	reflection-kdreams "${targets}" "${LIBRARIES}"
	"${kdreams_src}" "${be_src}"
	"src/kdreams;src/kdreams/lscr"
	"-DREFKEEN_VER_KDREAMS=1" "" 0
)

set(UNIFIED_CAT_GAME_PARAMS
	-DREFKEEN_HAS_VER_CAT3D=1 -DREFKEEN_HAS_VER_CATADVENTURES=1
	-DREFKEEN_HAS_VER_CATABYSS=1 -DREFKEEN_HAS_VER_CATARM=1
	-DREFKEEN_HAS_VER_CATAPOC=1 -DREFKEEN_HAS_VER_CATACOMB_ALL=1
)

add_refkeen_separate_target(
	reflection-cat3d "${targets}" "${LIBRARIES}"
	"${cat3d_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/cat3d"
	"-DREFKEEN_VER_CAT3D=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${UNIFIED}"
)

add_refkeen_separate_target(
	reflection-catabyss "${targets}" "${LIBRARIES}"
	"${catabyss_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/catabyss"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATABYSS=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${UNIFIED}"
)

add_refkeen_separate_target(
	reflection-catarm "${targets}" "${LIBRARIES}"
	"${catarm_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/cat_late;src/id91_11/cat_all/catarm"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATARM=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${UNIFIED}"
)

add_refkeen_separate_target(
	reflection-catapoc "${targets}" "${LIBRARIES}"
	"${catapoc_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/cat_late;src/id91_11/cat_all/catapoc"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATAPOC=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${UNIFIED}"
)

if (UNIFIED)
	string(REPLACE ";" " " UNIFIED_CAT_GAME_PARAMS "${UNIFIED_CAT_GAME_PARAMS}")
	add_refkeen_unified_target(
		reflection-catacomb "${targets}"
		"${be_src}" "${UNIFIED_CAT_GAME_PARAMS}"
		"${LIBRARIES};reflection-cat3d;reflection-catabyss;reflection-catarm;reflection-catapoc"
	)
endif ()

install(TARGETS ${targets})
install(FILES "${PROJECT_BINARY_DIR}/refkeen_config.h"
	DESTINATION include
)
