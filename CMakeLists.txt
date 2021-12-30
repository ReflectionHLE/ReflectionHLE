cmake_minimum_required(VERSION 2.8...3.19)

if (CMAKE_VERSION VERSION_LESS 3.12)
	project(ReflectionHLE)
else ()
	project(ReflectionHLE VERSION 0.40.0)
endif ()

set(bundle_identifier_prefix "com.reflectionhle")
set(bundle_copyright "See README.md for copyright details")

function(add_refhle_separate_target
	name app_rc targets libraries sources shared_sources
	include_dirs common_cc_flags target_cc_flags unified_target_cc_flags is_lib
)
	string(REGEX MATCH "-.+$" name_suffix ${name})
	string(SUBSTRING ${name_suffix} 1 -1 name_suffix)

	set(all_cc_flags ${common_cc_flags})
	foreach(MACRO ${target_cc_flags})
		list(APPEND all_cc_flags ${MACRO})
		string(REPLACE "_VER" "_HAS_VER" MACRO ${MACRO})
		list(APPEND all_cc_flags ${MACRO})
	endforeach ()
	if (is_lib)
		set(all_cc_flags ${unified_target_cc_flags} ${all_cc_flags})
		list(REMOVE_DUPLICATES "all_cc_flags")
		string(REPLACE ";" " " all_cc_flags "${all_cc_flags}")
		add_library(${name} STATIC ${sources})
		set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${all_cc_flags}")
	else ()
		set(icns_file "reflection-${app_rc}.icns")
		if (WIN32)
			set(app_rc "rsrc/reflection-${app_rc}.rc")
		elseif (APPLE)
			set(app_rc "rsrc/${icns_file}")
			set_source_files_properties(${app_rc} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
		else ()
			set(app_rc)
		endif ()
		string(REPLACE ";" " " all_cc_flags "${all_cc_flags}")
		if (CMAKE_SYSTEM_NAME STREQUAL "Android")
			add_library(${name} SHARED ${sources} ${shared_sources})
		else ()
			add_executable(${name} MACOSX_BUNDLE ${sources} ${shared_sources} ${app_rc})
		endif ()
		set_target_properties(${name} PROPERTIES
			COMPILE_FLAGS "${all_cc_flags}"
			MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/rsrc/Info.plist"
			MACOSX_BUNDLE_BUNDLE_NAME "ref${name_suffix}"
			MACOSX_BUNDLE_ICON_FILE "${icns_file}"
			MACOSX_BUNDLE_GUI_IDENTIFIER "${bundle_identifier_prefix}.${name}"
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
			MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
			MACOSX_BUNDLE_COPYRIGHT "${bundle_copyright}"
		)
		target_link_libraries(${name} ${libraries})
		list(APPEND targets ${name})
		set(targets "${targets}" PARENT_SCOPE)
	endif ()
	target_include_directories(${name} PUBLIC ${include_dirs})
endfunction()

function(add_refhle_unified_target name targets shared_sources common_cc_flags cc_flags libraries)
	set(icns_file "${name}.icns")
	if (WIN32)
		set(app_rc "rsrc/${name}.rc")
	elseif (APPLE)
		set(app_rc "rsrc/${icns_file}")
		set_source_files_properties(${app_rc} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	else ()
		set(app_rc)
	endif ()
	if (CMAKE_SYSTEM_NAME STREQUAL "Android")
		add_library(${name} SHARED ${shared_sources})
	else ()
		add_executable(${name} MACOSX_BUNDLE ${shared_sources} ${app_rc})
	endif ()
	set_target_properties(${name} PROPERTIES
		COMPILE_FLAGS "${common_cc_flags} ${cc_flags}"
		MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/rsrc/Info.plist"
		MACOSX_BUNDLE_BUNDLE_NAME "${name}"
		MACOSX_BUNDLE_ICON_FILE "${icns_file}"
		MACOSX_BUNDLE_GUI_IDENTIFIER "${bundle_identifier_prefix}.${name}"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
		MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
		MACOSX_BUNDLE_COPYRIGHT "${bundle_copyright}"
	)
	target_link_libraries(${name} ${libraries})
	list(APPEND targets ${name})
	set(targets "${targets}" PARENT_SCOPE)
endfunction()

include(CMakeDependentOption)

option(BUILD_AS_CPP "Build ReflectionHLE as C++ code" ON)
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
	set(REFKEEN_UNIFIED_BUILD "1")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# We build our own copy of SDL2 for Android
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	find_library(SDL2 SDL2)
	include_directories(src/android-project/reflectionhle/jni/SDL/include)
else ()
	find_package(SDL2 REQUIRED)
	find_package(SpeexDSP)
	include_directories(${SDL2_INCLUDE_DIR} ${SPEEXDSP_INCLUDE_DIRS})
endif ()

if (SPEEXDSP_FOUND)
	set(REFKEEN_RESAMPLER_LIBSPEEXDSP "1")
else ()
	set(REFKEEN_RESAMPLER_NONE "1")
endif ()

option(REFKEEN_ENABLE_LAUNCHER "Enable the ReflectionHLE launcher" ON)

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
	set(common_cc_flags "-x c++")
endif ()

configure_file(refkeen_config.h.in refkeen_config.h)

include(CheckLibraryExists)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	set(LIBRARIES log SDL2 SDL2main)
else ()
	set(LIBRARIES ${SDL2_LIBRARY} ${SPEEXDSP_LIBRARIES})
endif ()

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

set(UNIFIED_GAME_PARAMS
	-DREFKEEN_HAS_VER_KDREAMS=1

	-DREFKEEN_HAS_VER_CAT3D=1 -DREFKEEN_HAS_VER_CATADVENTURES=1
	-DREFKEEN_HAS_VER_CATABYSS=1 -DREFKEEN_HAS_VER_CATARM=1
	-DREFKEEN_HAS_VER_CATAPOC=1 -DREFKEEN_HAS_VER_CATACOMB_ALL=1

	-DREFKEEN_HAS_VER_WL920312=1
	-DREFKEEN_HAS_VER_WL1AP10=1
	-DREFKEEN_HAS_VER_WL1AP11=1
	-DREFKEEN_HAS_VER_WL6AP11=1
	-DREFKEEN_HAS_VER_WL1AP14=1
	-DREFKEEN_HAS_VER_WL6GT14A=1
	-DREFKEEN_HAS_VER_WJ6IM14=1
	-DREFKEEN_HAS_VER_WL6GT14B=1
	-DREFKEEN_HAS_VER_WL6AC14=1
	-DREFKEEN_HAS_VER_SDMFG10=1
	-DREFKEEN_HAS_VER_SODFG10=1
	-DREFKEEN_HAS_VER_SODFG14=1
	-DREFKEEN_HAS_VER_SODAC14=1
	-DREFKEEN_HAS_VER_N3DWT10=1
	-DREFKEEN_HAS_VER_WOLF3D_ALL=1
)

add_refhle_separate_target(
	reflection-kdreams kdreams "${targets}" "${LIBRARIES}"
	"${kdreams_src}" "${be_src}"
	"src/kdreams;src/kdreams/lscr"
	"${common_cc_flags}"
	"-DREFKEEN_VER_KDREAMS=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-cat3d cat3d "${targets}" "${LIBRARIES}"
	"${cat3d_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/cat3d"
	"${common_cc_flags}"
	"-DREFKEEN_VER_CAT3D=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-catabyss catabyss "${targets}" "${LIBRARIES}"
	"${catabyss_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/catabyss"
	"${common_cc_flags}"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATABYSS=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-catarm catarm "${targets}" "${LIBRARIES}"
	"${catarm_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/cat_late;src/id91_11/cat_all/catarm"
	"${common_cc_flags}"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATARM=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-catapoc catapoc "${targets}" "${LIBRARIES}"
	"${catapoc_src}" "${be_src}"
	"src/id91_11;src/id91_11/cat_all;src/id91_11/cat_all/external_restored;src/id91_11/cat_all/cat_late;src/id91_11/cat_all/catapoc"
	"${common_cc_flags}"
	"-DREFKEEN_VER_CATADVENTURES=1;-DREFKEEN_VER_CATAPOC=1;-DREFKEEN_VER_CATACOMB_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl920312 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL920312;-DGAMEVER_EXEDEF_WL920312=1;-DREFKEEN_VER_WL920312=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl1ap10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL1AP10;-DGAMEVER_EXEDEF_WL1AP10=1;-DREFKEEN_VER_WL1AP10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl1ap11 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL1AP11;-DGAMEVER_EXEDEF_WL1AP11=1;-DREFKEEN_VER_WL1AP11=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl6ap11 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL6AP11;-DGAMEVER_EXEDEF_WL6AP11=1;-DREFKEEN_VER_WL6AP11=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl1ap14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL1AP14;-DGAMEVER_EXEDEF_WL1AP14=1;-DREFKEEN_VER_WL1AP14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl6gt14a wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL6GT14A;-DGAMEVER_EXEDEF_WL6GT14A=1;-DREFKEEN_VER_WL6GT14A=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wj6im14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WJ6IM14;-DGAMEVER_EXEDEF_WJ6IM14=1;-DREFKEEN_VER_WJ6IM14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl6gt14b wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL6GT14B;-DGAMEVER_EXEDEF_WL6GT14B=1;-DREFKEEN_VER_WL6GT14B=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-wl6ac14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=WL6AC14;-DGAMEVER_EXEDEF_WL6AC14=1;-DREFKEEN_VER_WL6AC14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-sdmfg10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=SDMFG10;-DGAMEVER_EXEDEF_SDMFG10=1;-DREFKEEN_VER_SDMFG10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-sodfg10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=SODFG10;-DGAMEVER_EXEDEF_SODFG10=1;-DREFKEEN_VER_SODFG10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-sodfg14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=SODFG14A;-DGAMEVER_EXEDEF_SODFG14A=1;-DREFKEEN_VER_SODFG14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-sodac14 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=SODAC14;-DGAMEVER_EXEDEF_SODAC14=1;-DREFKEEN_VER_SODAC14=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

add_refhle_separate_target(
	reflection-n3dwt10 wolf3d "${targets}" "${LIBRARIES}"
	"${wolf3d_src}" "${be_src}"
	"src/wolf3d"
	"${common_cc_flags}"
	"-DGAMEVER_EXEDEF=N3DWT10;-DGAMEVER_EXEDEF_N3DWT10=1;-DREFKEEN_VER_N3DWT10=1;-DREFKEEN_VER_WOLF3D_ALL=1"
	"${UNIFIED_GAME_PARAMS}" "${BUILD_UNIFIED}"
)

if (BUILD_UNIFIED)
	string(REPLACE ";" " " UNIFIED_GAME_PARAMS "${UNIFIED_GAME_PARAMS}")
	add_refhle_unified_target(
		reflectionhle "${targets}"
		"${be_src}" "${common_cc_flags}" "${UNIFIED_GAME_PARAMS}"
		"${LIBRARIES};reflection-kdreams;reflection-cat3d;reflection-catabyss;reflection-catarm;reflection-catapoc;reflection-wl920312;reflection-wl1ap10;reflection-wl1ap11;reflection-wl6ap11;reflection-wl1ap14;reflection-wl6gt14a;reflection-wj6im14;reflection-wl6gt14b;reflection-wl6ac14;reflection-sdmfg10;reflection-sodfg10;reflection-sodfg14;reflection-sodac14;reflection-n3dwt10"
	)
endif ()

include(GNUInstallDirs)

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Android"))
	install(TARGETS ${targets}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		BUNDLE DESTINATION Applications
	)
endif ()

if (APPLE)
	include(cmake/MacOSDependencyScan.cmake)
	set(bundle_dependency_dirs "")
	macos_dependency_scan(bundle_dependency_dirs ${targets})

	install(CODE "set(bundle_dependency_dirs \"${bundle_dependency_dirs}\")" SCRIPT cmake/MacOSBundle.cmake)
endif ()
