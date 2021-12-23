#ifndef BE_FEATURES_H
#define BE_FEATURES_H

/* This header is used to define features, based on the EXE being built */

#if (defined REFKEEN_HAS_VER_CATARM) || (defined REFKEEN_HAS_VER_CATAPOC)
#define BE_CROSS_ENABLE_FARPTR_CFG 1
#endif

#ifdef REFKEEN_HAS_VER_CATADVENTURES
#define BE_CROSS_ENABLE_SORTED_FILENAMES_FUNC 1
#endif

#ifdef REFKEEN_HAS_VER_KDREAMS
#define BE_ST_ADAPT_VSYNC_DEFAULT_TO_KDREAMS 1
#define ENABLE_PKLITE 1
#endif

#if (defined REFKEEN_HAS_VER_KDREAMS) || (defined REFKEEN_HAS_VER_WOLF3D_ALL)
#define BE_ST_ENABLE_SETTING_DIGIVOL 1
#endif

#if (defined REFKEEN_HAS_VER_WOLF3D_ALL)
#define BE_ST_ENABLE_SETTING_SB 1
#endif

#endif // BE_FEATURES_H
