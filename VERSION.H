// *** VERSIONS RESTORATION ***
#define ARTSEXTERN
#define DEMOSEXTERN

#ifndef GAMEVER_EXEDEF_WL1AP10
#define CARMACIZED
#endif

#if (defined GAMEVER_EXEDEF_WL1AP10) || (defined GAMEVER_EXEDEF_WL1AP11) || (defined GAMEVER_EXEDEF_WL1AP12) || (defined GAMEVER_EXEDEF_WL1AP14)
#define UPLOAD
#elif (defined GAMEVER_EXEDEF_WL6ID14) || (defined GAMEVER_EXEDEF_WL6GT14A) || (defined GAMEVER_EXEDEF_WL6GT14B) || (defined GAMEVER_EXEDEF_WL6AC14)
#define GOODTIMES
#elif defined GAMEVER_EXEDEF_SDMFG10
#define SPEAR
#define SPEARDEMO
#elif (defined GAMEVER_EXEDEF_SODFG10) || (defined GAMEVER_EXEDEF_SODFG14A) || (defined GAMEVER_EXEDEF_SODFG14B)
#define SPEAR
#elif defined GAMEVER_EXEDEF_SODAC14
#define SPEAR
#define GOODTIMES
#elif (defined GAMEVER_EXEDEF_N3DWT10)
// NEW macro, used so EXEDEF isn't redirectly mentioned outside VERSION.*
// (and since we can't use just GAMEVER_WOLFREV)
#define GAMEVER_NOAH3D
#endif

// A new GAMEVER_WOLFREV macro, usually based on some known or guessed
// release/build date, but does not have to be.

#ifdef GAMEVER_EXEDEF_WL1AP10
#define GAMEVER_WOLFREV 19920505L
#elif (defined GAMEVER_EXEDEF_WL1AP11)
#define GAMEVER_WOLFREV 19920601L
#elif (defined GAMEVER_EXEDEF_WL1AP12) || (defined GAMEVER_EXEDEF_WL6AP11)
// WL1AP12 might be later, but it looks like the same code rev.
#define GAMEVER_WOLFREV 19920614L
#elif (defined GAMEVER_EXEDEF_SDMFG10)
#define GAMEVER_WOLFREV 19920921L
#elif (defined GAMEVER_EXEDEF_SODFG10)
#define GAMEVER_WOLFREV 19920928L
#elif (defined GAMEVER_EXEDEF_WL1AP14) || (defined GAMEVER_EXEDEF_WL6AP14)
#define GAMEVER_WOLFREV 19921101L
#elif (defined GAMEVER_EXEDEF_SODFG14A) || (defined GAMEVER_EXEDEF_SODFG14B)
// Only the sign-on screen and project settings really differed
#define GAMEVER_WOLFREV 19921112L
#elif (defined GAMEVER_EXEDEF_WL6GT14A) || (defined GAMEVER_EXEDEF_WL6ID14)
// GT might be later than ID, but only the sign-on screen differs
#define GAMEVER_WOLFREV 19930903L
#elif (defined GAMEVER_EXEDEF_N3DWT10)
// Like GT14B and later, N3D has the verification block from WL_INTER.C removed,
// but it lacks a few minor GT14B changes to WL_TEXT.C, and an end-screen is still
// shown on quit (which isn't the case in the later 1998 Activision release).
// Since N3D was released on 1994, let's assume the first day of January.
#define GAMEVER_WOLFREV 19940101L
#elif (defined GAMEVER_EXEDEF_WL6GT14B)
#define GAMEVER_WOLFREV 19940413L
#elif (defined GAMEVER_EXEDEF_WL6AC14) || (defined GAMEVER_EXEDEF_SODAC14)
#define GAMEVER_WOLFREV 19980313L
#else
#error "No proper GAMEVER_EXEDEF macro is defined!"
#endif

/*** Additional convenience macros ***/

// This doesn't cover the SOD case of the debug keys, just Wolf3D v1.0 and S3DNA vs the later releases
#ifdef GAMEVER_EXEDEF_WL1AP10

#define GAMEVER_WOLF3D_DEBUGPARM "next"
#define GAMEVER_COND_FARPTR
#define GAMEVER_COND_FSTRCPY strcpy

#else

#ifdef GAMEVER_EXEDEF_N3DWT10
#define GAMEVER_WOLF3D_DEBUGPARM "debug"
#else
#define GAMEVER_WOLF3D_DEBUGPARM "goobers"
#endif
#define GAMEVER_COND_FARPTR far
#define GAMEVER_COND_FSTRCPY _fstrcpy

#endif

//#define MYPROFILE
//#define DEBCHECK
