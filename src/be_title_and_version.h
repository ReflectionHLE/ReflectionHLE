#ifndef BE_TITLE_AND_VERSION_H
#define BE_TITLE_AND_VERSION_H

#ifdef REFKEEN_UNIFIED_BUILD
#define REFKEEN_TITLE_STRING "ReflectionHLE"
#elif (defined REFKEEN_VER_KDREAMS)
#define REFKEEN_TITLE_STRING "Reflection KDreams"
#elif (defined REFKEEN_VER_CAT3D)
#define REFKEEN_TITLE_STRING "Reflection Cat3D"
#elif (defined REFKEEN_VER_CATABYSS)
#define REFKEEN_TITLE_STRING "Reflection CatAbyss"
#elif (defined REFKEEN_VER_CATARM)
#define REFKEEN_TITLE_STRING "Reflection CatArm"
#elif (defined REFKEEN_VER_CATAPOC)
#define REFKEEN_TITLE_STRING "Reflection CatApoc"
#elif (defined REFKEEN_HAS_VER_CATACOMB_ALL)
#define REFKEEN_TITLE_STRING "Reflection Cat3DAll"
#elif (defined REFKEEN_HAS_VER_WOLF3D_ALL) // Even if just a specific version
#define REFKEEN_TITLE_STRING "Reflection Wolf3D"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

// Note that the "vX.Y.Z" format is important, since a makefile command parses
// the string to fill the CFBundleVersion string in Info.plist (OS X/macOS specific)

#define REFKEEN_VERSION_STRING "v0.40.1"

#define REFKEEN_TITLE_AND_VER_STRING REFKEEN_TITLE_STRING " " REFKEEN_VERSION_STRING

#endif // BE_TITLE_AND_VERSION_H
