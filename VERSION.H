// ***VERSIONS RESTORATION***
#define ARTSEXTERN
#define DEMOSEXTERN
#define CARMACIZED

#ifdef GAMEVER_RESTORATION_WL1_APO14
#define UPLOAD
#elif defined GAMEVER_RESTORATION_WL6_APO14
// Nothing to add in such a case
#elif (defined GAMEVER_RESTORATION_WL6_ID14) || (defined GAMEVER_RESTORATION_WL6_GT114) || (defined GAMEVER_RESTORATION_WL6_GT214) || (defined GAMEVER_RESTORATION_WL6_ACT14)
#define GOODTIMES
#elif defined GAMEVER_RESTORATION_SOD_ACT14
#define SPEAR
#define GOODTIMES
#elif defined GAMEVER_RESTORATION_SDM_FOR10
#define SPEAR
#define SPEARDEMO
#else
#error "No proper game version restoration macro is defined!"
#endif

//#define MYPROFILE
//#define DEBCHECK
