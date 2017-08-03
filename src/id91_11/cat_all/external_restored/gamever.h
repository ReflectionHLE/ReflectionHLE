// *** VERSIONS RESTORATION ***

// Not defining GAMEVER_SHAREWARE on purpose,
// supporting two different versions from a single catabyss EXE
#ifdef REFKEEN_VER_CATABYSS
#define GAMEVER_CATABYSS
#elif (defined REFKEEN_VER_CATARM)
#define GAMEVER_CATARM
#elif (defined REFKEEN_VER_CATAPOC)
#define GAMEVER_CATAPOC
#endif

// HUGE FIXME!!! - This assumes catabyss *AND* slidecat!
//#ifdef GAMEVER_SLIDECAT
//#ifdef GAMEVER_CATABYSS
#define GAMEVER_SLIDECAT_ERR_STR "CATALOG ERROR"
//#else
//#define GAMEVER_SLIDECAT_ERR_STR "ERROR"
//#endif
//#endif
