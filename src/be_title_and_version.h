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
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif

// Note that the "vX.Y.Z" format is important, since a Makefile command parses
// the string to fill the CFBundleVersion string in Info.plist (OS X/macOS specific)

//#define REFKEEN_VERSION_STRING "Dev. Build"
#define REFKEEN_VERSION_STRING "v0.18.0"

#define REFKEEN_TITLE_AND_VER_STRING REFKEEN_TITLE_STRING " " REFKEEN_VERSION_STRING

#endif // BE_TITLE_AND_VERSION_H
