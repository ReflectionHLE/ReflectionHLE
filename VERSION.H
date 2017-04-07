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
#elif (defined GAMEVER_EXEDEF_SODFG10) || (defined GAMEVER_EXEDEF_SODFG14B)
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
#elif (defined GAMEVER_EXEDEF_SODFO10)
#define GAMEVER_WOLFREV 19920928L
#elif (defined GAMEVER_EXEDEF_SDMFO10)
#define GAMEVER_WOLFREV 19921022L
#elif (defined GAMEVER_EXEDEF_WL1AP14) || (defined GAMEVER_EXEDEF_WL6AP14)
#define GAMEVER_WOLFREV 19921101L
#elif (defined GAMEVER_EXEDEF_SDMFO14A) || (defined GAMEVER_EXEDEF_SDMFO14B)
// Only the sign-on screen and project settings really differed
#define GAMEVER_WOLFREV 19921112L
#elif (defined GAMEVER_EXEDEF_WL6GT14A) || (defined GAMEVER_EXEDEF_WL6ID14) || (defined GAMEVER_EXEDEF_N3DWT10)
// GT might be later than ID, but only the sign-on screen differs.
// Also, N3D is missing later changes, essentially residing in a separate "branch".
#define GAMEVER_WOLFREV 19930903L
#elif (defined GAMEVER_EXEDEF_WL6GT14B)
#define GAMEVER_WOLFREV 19940413L
#elif (defined GAMEVER_EXEDEF_WL6AC14) || (defined GAMEVER_EXEDEF_SODAC14)
#define GAMEVER_WOLFREV 19980313L
#else
#error "No proper GAMEVER_EXEDEF macro is defined!"
#endif

// Additional convenience macros
#if (defined GAMEVER_EXEDEF_WL1AP10)
#define GAMEVER_RESTORATION_ANY_APO10
#endif
#if (defined GAMEVER_EXEDEF_WL1AP11) || (defined GAMEVER_RESTORATION_WL6_APO11)
#define GAMEVER_RESTORATION_ANY_APO11
#endif
#if (defined GAMEVER_EXEDEF_WL1AP12) || (defined GAMEVER_EXEDEF_WL6AP11)
#define GAMEVER_RESTORATION_ANY_APO12
#endif
#if (defined GAMEVER_EXEDEF_WL1AP14) || (defined GAMEVER_EXEDEF_WL6AP14)
#define GAMEVER_RESTORATION_ANY_APO14
#endif

#if (defined GAMEVER_RESTORATION_ANY_APO10) || (defined GAMEVER_RESTORATION_ANY_APO11) || (defined GAMEVER_RESTORATION_ANY_APO12)
#define GAMEVER_RESTORATION_ANY_APO_PRE14
#endif

#if (defined GAMEVER_EXEDEF_SDMFG10) || (defined GAMEVER_EXEDEF_SODFG10)
#define GAMEVER_RESTORATION_ANY_FOR10
#endif

#if (defined GAMEVER_RESTORATION_ANY_APO_PRE14) || (defined GAMEVER_RESTORATION_ANY_FOR10)
#define GAMEVER_RESTORATION_ANY_ALL_PRE14
#endif

#if (defined GAMEVER_EXEDEF_WL6AC14) || (defined GAMEVER_EXEDEF_SODAC14)
#define GAMEVER_RESTORATION_ANY_ACT14
#endif

#if (defined GAMEVER_EXEDEF_WL6GT14B) || (defined GAMEVER_EXEDEF_WL6AC14) || (defined GAMEVER_EXEDEF_SODAC14)
#define GAMEVER_RESTORATION_ANY_FROM_GT214
#endif

#if (defined GAMEVER_RESTORATION_ANY_FROM_GT214) || (defined GAMEVER_EXEDEF_N3DWT10)
#define GAMEVER_RESTORATION_ANY_POST_GT114
#endif

#if (defined GAMEVER_RESTORATION_ANY_ALL_PRE14) || (defined GAMEVER_RESTORATION_ANY_APO14) || (defined GAMEVER_EXEDEF_SODFG14B)
#define GAMEVER_RESTORATION_ANY_PRE_GT
#endif

// This doesn't cover the SOD case of the debug keys, just Wolf3D v1.0 and S3DNA vs the later releases
#ifdef GAMEVER_EXEDEF_WL1AP10

#define GAMEVER_RESTORATION_W3D_DEBUGPARM "next"
#define GAMEVER_RESTORATION_CONDFARPTR
#define GAMEVER_RESTORATION_CONDFARSTRCPY strcpy

#else

#ifdef GAMEVER_EXEDEF_N3DWT10
#define GAMEVER_RESTORATION_W3D_DEBUGPARM "debug"
#else
#define GAMEVER_RESTORATION_W3D_DEBUGPARM "goobers"
#endif
#define GAMEVER_RESTORATION_CONDFARPTR far
#define GAMEVER_RESTORATION_CONDFARSTRCPY _fstrcpy

#endif

//#define MYPROFILE
//#define DEBCHECK
