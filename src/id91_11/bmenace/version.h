#ifndef __VERSION_H__
#define __VERSION_H__

#ifdef REFKEEN_VER_BH071592
#define BETA
#endif

#if (defined REFKEEN_VER_BM1V10S) || (defined REFKEEN_VER_BM1V11S)
#define SHAREWARE 1
#else
#define SHAREWARE 0
#endif

#ifdef REFKEEN_VER_BM1V11GM
#define GMS_VERSION
#endif

#if (defined REFKEEN_VER_BM1V11F) || (defined REFKEEN_VER_BM2V11F) || (defined REFKEEN_VER_BM3V11F)
#define VERSION_1_1_FREEWARE
#endif

#if (defined REFKEEN_VER_BM1V10S) || (defined REFKEEN_VER_BM1V10R) || \
    (defined REFKEEN_VER_BM2V10R) || (defined REFKEEN_VER_BM3V10R)
#define VERSION_1_0
#endif

#if (defined REFKEEN_VER_BM1V11S) || (defined REFKEEN_VER_BM1V11R) || \
    (defined REFKEEN_VER_BM2V11R) || (defined REFKEEN_VER_BM3V11R) || \
    (defined GMS_VERSION) || (defined VERSION_1_1_FREEWARE)
#define VERSION_1_1
#endif

#if (defined REFKEEN_VER_BM1V10S) || (defined REFKEEN_VER_BM1V10R) || \
    (defined REFKEEN_VER_BM1V11S) || (defined REFKEEN_VER_BM1V11R) || \
    (defined REFKEEN_VER_BM1V11GM) || (defined REFKEEN_VER_BM1V11F)
#define EPISODE 1
#endif
#if (defined REFKEEN_VER_BH071592) || (defined REFKEEN_VER_BM2V10R) || \
    (defined REFKEEN_VER_BM2V11R) || (defined REFKEEN_VER_BM2V11F)
#define EPISODE 2
#endif
#if (defined REFKEEN_VER_BM3V10R) || \
    (defined REFKEEN_VER_BM3V11R) || (defined REFKEEN_VER_BM3V11F)
#define EPISODE 3
#endif

//#define FIX_MUSIC_MEMORY_ISSUES       //make sure the music does not cause "out of memory" issues

#ifdef BETA
#define EXTENSION "BH1"
#elif (EPISODE == 1)
#define EXTENSION "BM1"
#elif (EPISODE == 2)
#define EXTENSION "BM2"
#elif (EPISODE == 3)
#define EXTENSION "BM3"
#else
#error Unhandled Episode!
#endif

#if (defined VERSION_1_0) && (defined VERSION_1_1)
#error You cannot use multiple game versions in one build!
#endif

#endif
