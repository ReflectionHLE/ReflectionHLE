cmake_minimum_required(VERSION 2.8)
project(Reflection-Keen)

function(add_refkeen_separate_target
	name app_rc targets libraries sources shared_sources
	include_dirs target_cc_flags common_cc_flags is_lib
)
	foreach(MACRO ${target_cc_flags})
		list(APPEND all_cc_flags ${MACRO})
		string(REPLACE "_VER" "_HAS_VER" MACRO ${MACRO})
		list(APPEND all_cc_flags ${MACRO})
	endforeach ()
	if (is_lib)
		set(all_cc_flags ${common_cc_flags} ${all_cc_flags})
		list(REMOVE_DUPLICATES "all_cc_flags")
		string(REPLACE ";" " " all_cc_flags "${all_cc_flags}")
		add_library(${name} STATIC ${sources})
		set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${all_cc_flags}")
	else ()
		if (WIN32)
			set(app_rc "rsrc/reflection-${app_rc}.rc")
		else ()
			set(app_rc)
		endif ()
		string(REPLACE ";" " " all_cc_flags "${all_cc_flags}")
		add_executable(${name} ${sources} ${shared_sources} ${app_rc})
		set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${all_cc_flags}")
		target_link_libraries(${name} ${libraries})
		set(${targets} "${targets} ${name}" PARENT_SCOPE)
	endif ()
	target_include_directories(${name} PUBLIC ${include_dirs})
endfunction()

function(add_refkeen_unified_target name app_rc targets shared_sources cc_flags libraries)
	if (WIN32)
		set(app_rc "rsrc/reflection-${app_rc}.rc")
	else ()
		set(app_rc)
	endif ()
	add_executable(${name} ${shared_sources} ${app_rc})
	set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${cc_flags}")
	target_link_libraries(${name} ${libraries})
	set(${targets} "${targets} ${name}" PARENT_SCOPE)
endfunction()

include(CMakeDependentOption)

option(BUILD_AS_CPP "Build Reflection Keen as C++ code" ON)
CMAKE_DEPENDENT_OPTION(
	BUILD_UNIFIED "Build aggregated C++ EXEs covering multiple games/versions" ON
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

if (BUILD_UNIFIED)
	set(REFKEEN_USE_NS "1")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
find_package(SDL2 REQUIRED)
find_package(SpeexDSP)
include_directories(${SDL2_INCLUDE_DIR} ${SPEEXDSP_INCLUDE_DIRS})

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
include(src/wolf3d/CMakeLists.txt)

set(all_src ${be_src} ${kdreams_src} ${cat3d_src}
    ${catabyss_src} ${catarm_src} ${catapoc_src} ${wolf3d_src})
list(REMOVE_DUPLICATES "all_src")

if (BUILD_AS_CPP)
	set_source_files_properties(${all_src} PROPERTIES LANGUAGE CXX)
endif ()

configure_file(refkeen_config.h.in refkeen_config.h)

set(LIBRARIES ${SDL2_LIBRARY} ${SPEEXDSP_LIBRARIES})
check_library_exists(m pow "" LIBM)
if (LIBM)
	set(LIBRARIES ${LIBRARIES} m)
endif ()
if (WIN32)
	set(LIBRARIES ${LIBRARIES} shlwapi)
endif ()

include_directories(
	PUBLIC src src/crc32 src/depklite src/opl src/unlzexe
	rsrc "${PROJECT_BINARY_DIR}"
)

add_refkeen_separate_target(
	reflection-kdreams kdreams "${targets}" "${LIBRARIES}"
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
	reflection-cat3d cat3d "${targets}" "${LIBRARIES}"
	"${cat3d_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/cat3d"
	"-DREFKEEN_VER_CAT3D=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-catabyss catabyss "${targets}" "${LIBRARIES}"
	"${catabyss_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/catabyss"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATABYSS=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-catarm catarm "${targets}" "${LIBRARIES}"
	"${catarm_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/cat_late;src/id91_11/cat_all/catarm"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATARM=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-catapoc catapoc "${targets}" "${LIBRARIES}"
	"${catapoc_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/cat_late;src/id91_11/cat_all/catapoc"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATAPOC=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_CAT_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

set(UNIFIED_WOLF3D_GAME_PARAMS
	-DREFKEEN_HAS_VER_WL1AP10=1
	-DREFKEEN_HAS_VER_WL1AP11=1
	-DREFKEEN_HAS_VER_WL6AP11=1
	-DREFKEEN_HAS_VER_WL1AP14=1
	-DREFKEEN_HAS_VER_WL6GT14A=1
	-DREFKEEN_HAS_VER_WL6GT14B=1
	-DREFKEEN_HAS_VER_WL6AC14=1
	-DREFKEEN_HAS_VER_SDMFG10=1
	-DREFKEEN_HAS_VER_SODAC14=1
	-DREFKEEN_HAS_VER_N3DWT10=1
	-DREFKEEN_HAS_VER_WOLF3D_ALL=1
)

add_refkeen_separate_target(
	reflection-wl1ap10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL1AP10;-DGAMEVER_EXEDEF_WL1AP10=1;-DREFKEEN_VER_WL1AP10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-wl1ap11 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL1AP11;-DGAMEVER_EXEDEF_WL1AP11=1;-DREFKEEN_VER_WL1AP11=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-wl1ap12 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL1AP12;-DGAMEVER_EXEDEF_WL1AP12=1;-DREFKEEN_VER_WL6AP11=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-wl1ap14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL1AP14;-DGAMEVER_EXEDEF_WL1AP14=1;-DREFKEEN_VER_WL1AP14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-wl6gt14a wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL6GT14A;-DGAMEVER_EXEDEF_WL6GT14A=1;-DREFKEEN_VER_WL6GT14A=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-wl6gt14b wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL6GT14B;-DGAMEVER_EXEDEF_WL6GT14B=1;-DREFKEEN_VER_WL6GT14B=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-wl6ac14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=WL6AC14;-DGAMEVER_EXEDEF_WL6AC14=1;-DREFKEEN_VER_WL6AC14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-sdmfg10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=SDMFG10;-DGAMEVER_EXEDEF_SDMFG10=1;-DREFKEEN_VER_SDMFG10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-sodac14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=SODAC14;-DGAMEVER_EXEDEF_SODAC14=1;-DREFKEEN_VER_SODAC14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refkeen_separate_target(
	reflection-n3dwt10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"-DGAMEVER_EXEDEF=N3DWT10;-DGAMEVER_EXEDEF_N3DWT10=1;-DREFKEEN_VER_N3DWT10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_WOLF3D_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

if (BUILD_UNIFIED)
	string(REPLACE ";" " " UNIFIED_CAT_GAME_PARAMS "${UNIFIED_CAT_GAME_PARAMS}")
	add_refkeen_unified_target(
		reflection-catacomb cat3d "${targets}"
		"${be_src}" "${UNIFIED_CAT_GAME_PARAMS}"
		"${LIBRARIES};reflection-cat3d;reflection-catabyss;reflection-catarm;reflection-catapoc"
	)
	string(REPLACE ";" " " UNIFIED_WOLF3D_GAME_PARAMS "${UNIFIED_WOLF3D_GAME_PARAMS}")
	add_refkeen_unified_target(
		reflection-wolf3d wolf3d "${targets}"
		"${be_src}" "${UNIFIED_WOLF3D_GAME_PARAMS}"
		"${LIBRARIES};reflection-wl1ap10;reflection-wl1ap11;reflection-wl1ap12;reflection-wl1ap14;reflection-wl6gt14a;reflection-wl6gt14b;reflection-wl6ac14;reflection-sdmfg10;reflection-sodac14;reflection-n3dwt10"
	)
endif ()

install(TARGETS ${targets})
install(FILES "${PROJECT_BINARY_DIR}/refkeen_config.h"
	DESTINATION include
)
