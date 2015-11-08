#ifndef _BE_TITLE_AND_VERSION_H_
#define _BE_TITLE_AND_VERSION_H_

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

#define REFKEEN_VERSION_STRING "0.13.0"

#define REFKEEN_TITLE_AND_VER_STRING REFKEEN_TITLE_STRING " v" REFKEEN_VERSION_STRING

#endif // _BE_TITLE_AND_VERSION_H_
