/* This header must be included *only* from be_gamedef.h */

#ifdef REFKEEN_VER_CATABYSS
#include "be_gamedef_catabyss.h"
#endif
#ifdef REFKEEN_VER_CATARM
#include "be_gamedef_catarm.h"
#endif
#ifdef REFKEEN_VER_CATAPOC
#include "be_gamedef_catapoc.h"
#endif

// These MUST have the same order as in the BE_GameVer_T enum
void (*refkeen_game_exe_main_funcs[BE_GAMEVER_LAST])(void) = {
#ifdef REFKEEN_HAS_VER_KDREAMS
#error "Undefined for Keen Dreams"
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
	&cat3d_exe_main,
	&cat3d_exe_main,
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	&abysgame_exe_main,
	&abysgame_exe_main,
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	&armgame_exe_main,
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	&apocgame_exe_main,
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
	&slidecat_exe_main,
	&slidecat_exe_main,
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	&slidecat_exe_main,
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	&slidecat_exe_main,
#endif
};
