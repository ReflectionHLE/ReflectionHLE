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

/* Disable mouse button overrides for UI-accessible buttons for now.
   That includes function keys and Catacomb 3-D/Abyss' scrolls.
   The cfg settings are still there, but they'll be unused and won't
   appear in the launcher. Problem is that due to the implementation in use,
   if the feature is enabled and matching mouse button is pressed, the UI in
   question will not appear without having a detected game controller in use. */

/* #undef BE_ST_ENABLE_SHORTCUTS_MOUSE_BINDS */

#endif // BE_FEATURES_H
