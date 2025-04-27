#ifndef __VERSION_H__
#define __VERSION_H__

// TODO: This covers just BM1 freeware v1.1 for now

//IMPORTANT: Always make sure to use the same settings in VERSION.EQU !!

#define SHAREWARE 0
#define EPISODE 1

//#define VERSION_1_0
#define VERSION_1_1
#define VERSION_1_1_FREEWARE

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
