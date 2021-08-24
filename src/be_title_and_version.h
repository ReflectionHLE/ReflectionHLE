#ifndef BE_TITLE_AND_VERSION_H
#define BE_TITLE_AND_VERSION_H

#ifdef REFKEEN_VER_KDREAMS
#define REFKEEN_TITLE_STRING "Reflection Keen Dreams"
#elif (defined REFKEEN_VER_CAT3D)
#define REFKEEN_TITLE_STRING "Reflection Catacomb 3-D"
#elif (defined REFKEEN_VER_CATABYSS)
#define REFKEEN_TITLE_STRING "Reflection Catacomb Abyss"
#elif (defined REFKEEN_VER_CATARM)
#define REFKEEN_TITLE_STRING "Reflection Catacomb Armageddon"
#elif (defined REFKEEN_VER_CATAPOC)
#define REFKEEN_TITLE_STRING "Reflection Catacomb Apocalypse"
#elif (defined REFKEEN_HAS_VER_CATACOMB_ALL)
#define REFKEEN_TITLE_STRING "Reflection Catacomb"
#elif (defined REFKEEN_HAS_VER_WOLF3D_ALL) // Even if just a specific version
#define REFKEEN_TITLE_STRING "Reflection Wolfenstein 3D"
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

// Note that the "vX.Y.Z" format is important, since a makefile command parses
// the string to fill the CFBundleVersion string in Info.plist (OS X/macOS specific)

#define REFKEEN_VERSION_STRING "v0.33.1: Loud Edition"

#define REFKEEN_TITLE_AND_VER_STRING REFKEEN_TITLE_STRING " " REFKEEN_VERSION_STRING

#endif // BE_TITLE_AND_VERSION_H
