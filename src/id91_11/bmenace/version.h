#ifndef __VERSION_H__
#define __VERSION_H__

#if (defined REFKEEN_VER_BM1V11F) || (defined REFKEEN_VER_BM2V11F) || (defined REFKEEN_VER_BM3V11F)
#define SHAREWARE 0
#define VERSION_1_1
#define VERSION_1_1_FREEWARE
#endif

#ifdef REFKEEN_VER_BM1V11F
#define EPISODE 1
#endif
#ifdef REFKEEN_VER_BM2V11F
#define EPISODE 2
#endif
#ifdef REFKEEN_VER_BM3V11F
#define EPISODE 3
#endif

//#define FIX_MUSIC_MEMORY_ISSUES       //make sure the music does not cause "out of memory" issues

#if (EPISODE == 1)
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
