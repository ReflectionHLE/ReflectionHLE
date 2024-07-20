/* This header must be included *only* from be_gamedefs.h */

#ifdef REFKEEN_HAS_VER_CAT3D
#include "be_gamedefs_cat3d.h"
#endif
#ifdef REFKEEN_HAS_VER_CATADVENTURES
#include "be_gamedefs_catadventures.h"
#endif

static const BE_GameVerDetails_T *g_be_gamever_ptrs_catacomb_all[] = {
#ifdef REFKEEN_HAS_VER_CAT3D
	&g_be_gamever_cat3d100,
	&g_be_gamever_cat3d122,
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	&g_be_gamever_catabyss113,
	&g_be_gamever_catabyss124,
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	&g_be_gamever_catarm102,
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	&g_be_gamever_catapoc101,
#endif
};

static const BE_GameDetails_T g_be_game_catacomb_all = {
	"Catacomb 3-D and later",
	g_be_gamever_ptrs_catacomb_all,
	BE_Cross_ArrayLen(g_be_gamever_ptrs_catacomb_all),
	BE_GAME_CATACOMB_ALL
};
