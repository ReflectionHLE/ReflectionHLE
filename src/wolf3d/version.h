// *** VERSIONS RESTORATION ***

// Note: UPLOAD is defined for v1.0, as well as the alpha, although
// there are good chances this macro didn't even exist back then.
// So at the least, it's used for the sake of convenience.

// GAMEVER_WOLFREV definitions

#define GV_WR_WL920312 19920312L
#define GV_WR_WL1AP10 19920505L
#define GV_WR_WL1AP11 19920601L
#define GV_WR_WL6AP11 19920610L
// WL1AP12 might be later, but it looks like the same code rev.
#define GV_WR_WL1AP12 19920610L
#define GV_WR_SDMFG10 19920921L
#define GV_WR_SODFG10 19920928L
#define GV_WR_WL1AP14 19921101L
#define GV_WR_WL6AP14 19921101L
#define GV_WR_SODFG14A 19921111L
// Only the sign-on screen and project settings really differed
#define GV_WR_SODFG14B 19921111L
#define GV_WR_WJ6IM14 19930304L
// GT might be later than ID, but only the sign-on screen differs
#define GV_WR_WL6GT14A 19930902L
#define GV_WR_WL6ID14 19930902L
// Like GT14B and later, N3D has the verification block from WL_INTER.C removed,
// but it lacks a few minor GT14B changes to WL_TEXT.C, and an end-screen is still
// shown on quit (which isn't the case in the later 1998 Activision release).
// Since N3D was released on 1994, let's assume the first day of January.
#define GV_WR_N3DWT10 19940101L
#define GV_WR_WL6GT14B 19940413L
#define GV_WR_WL6AC14 19980127L
#define GV_WR_SODAC14 19980127L

// Now define GAMEVER_WOLFREV to one of the above,
// based on the definition of GAMEVER_EXEDEF

#define GAMEVER_CONCAT1(x,y) x ## y
#define GAMEVER_CONCAT2(x,y) GAMEVER_CONCAT1(x,y)
#define GAMEVER_WOLFREV GAMEVER_CONCAT2(GV_WR_,GAMEVER_EXEDEF)

// Now defining the usual macros, plus a few more. Note that
// these use a *separate* GAMEVER_EXEDEF_*** macro

#if (defined GAMEVER_EXEDEF_WL920312) || (defined GAMEVER_EXEDEF_WL1AP10) || (defined GAMEVER_EXEDEF_WL1AP11) || (defined GAMEVER_EXEDEF_WL1AP12) || (defined GAMEVER_EXEDEF_WL1AP14)
#define UPLOAD
#elif (defined GAMEVER_EXEDEF_WL6ID14) || (defined GAMEVER_EXEDEF_WL6GT14A) || (defined GAMEVER_EXEDEF_WL6GT14B) || (defined GAMEVER_EXEDEF_WL6AC14)
#define GOODTIMES
#elif (defined GAMEVER_EXEDEF_WJ6IM14)
#define JAPAN
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

#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#define ARTSEXTERN
#define DEMOSEXTERN
#endif

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#define CARMACIZED
#endif

//#define MYPROFILE
//#define DEBCHECK

/*** Additional convenience macros ***/

// This doesn't cover the SOD case of the debug keys, just Wolf3D v1.0/earlier and S3DNA vs the later releases
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)

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
