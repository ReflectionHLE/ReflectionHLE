/* This header must be included *only* from be_gamedef.h */

#ifdef REFKEEN_HAS_VER_CATABYSS
#include "be_gamedef_catabyss.h"
#endif
#ifdef REFKEEN_HAS_VER_CATARM
#include "be_gamedef_catarm.h"
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
#include "be_gamedef_catapoc.h"
#endif

// These MUST have the same order as in the BE_GameVer_T enum
void (*refkeen_game_exe_main_funcs[BE_GAMEVER_LAST])(void) = {
#ifdef REFKEEN_HAS_VER_KDREAMS
#error "Undefined for Keen Dreams"
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
	&REFKEEN_NS_ENCLOSE(cat3d, cat3d_exe_main),
	&REFKEEN_NS_ENCLOSE(cat3d, cat3d_exe_main),
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	&REFKEEN_NS_ENCLOSE(catabyss, abysgame_exe_main),
	&REFKEEN_NS_ENCLOSE(catabyss, abysgame_exe_main),
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	&REFKEEN_NS_ENCLOSE(catarm, armgame_exe_main),
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	&REFKEEN_NS_ENCLOSE(catapoc, apocgame_exe_main),
#endif
};

void (*refkeen_slidecat_exe_main_funcs[BE_GAMEVER_LAST])(void) = {
#ifdef REFKEEN_HAS_VER_KDREAMS
#error "Undefined for Keen Dreams"
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
	NULL,
	NULL,
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	&REFKEEN_NS_ENCLOSE(catabyss, slidecat_exe_main),
	&REFKEEN_NS_ENCLOSE(catabyss, slidecat_exe_main),
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	&REFKEEN_NS_ENCLOSE(catarm, slidecat_exe_main),
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	&REFKEEN_NS_ENCLOSE(catapoc, slidecat_exe_main),
#endif
};
