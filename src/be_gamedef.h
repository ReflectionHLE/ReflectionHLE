/* This header must be included *only* from be_cross_fs.c */

#include "be_gamestr.h"

// These MUST have the same order as in the BE_GameVer_T enum
const char *refkeen_gamever_strs[BE_GAMEVER_LAST] = {
#ifdef REFKEEN_HAS_VER_KDREAMS
	BE_STR_GAMEVER_KDREAMSE100,
	BE_STR_GAMEVER_KDREAMSC100,
	BE_STR_GAMEVER_KDREAMSE113,
	BE_STR_GAMEVER_KDREAMSC105,
	BE_STR_GAMEVER_KDREAMSE193,
	BE_STR_GAMEVER_KDREAMSE120,
	BE_STR_GAMEVER_KDREAMS2015,
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
	BE_STR_GAMEVER_CAT3D100,
	BE_STR_GAMEVER_CAT3D122,
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	BE_STR_GAMEVER_CATABYSS113,
	BE_STR_GAMEVER_CATABYSS124,
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	BE_STR_GAMEVER_CATARM102,
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	BE_STR_GAMEVER_CATAPOC101,
#endif
#ifdef REFKEEN_HAS_VER_WL1AP10
	BE_STR_GAMEVER_WL1AP10,
#endif
};

const char *refkeen_gamever_descriptions[BE_GAMEVER_LAST] = {
#ifdef REFKEEN_HAS_VER_KDREAMS
	"Keen Dreams EGA v1.00",
	"Keen Dreams CGA v1.00",
	"Keen Dreams EGA v1.13",
	"Keen Dreams CGA v1.05",
	"Keen Dreams EGA v1.93",
	"Keen Dreams EGA v1.20",
	"Keen Dreams 2015",
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
	"Catacomb 3-D v1.00",
	"Catacomb 3-D v1.22",
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	"Catacomb Abyss v1.13",
	"Catacomb Abyss v1.24",
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	"Catacomb Armageddon v1.02",
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	"Catacomb Apocalypse v1.01",
#endif
#ifdef REFKEEN_HAS_VER_WL1AP10
	"Wolfenstein 3D Shareware v1.0",
#endif
};


#ifdef REFKEEN_HAS_VER_KDREAMS
#include "be_gamedef_kdreams.h"
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
#include "be_gamedef_cat3d.h"
#endif
#ifdef REFKEEN_HAS_VER_CATADVENTURES
#include "be_gamedef_catadventures.h"
#endif
#ifdef REFKEEN_HAS_VER_WOLF3D_ALL
#include "be_gamedef_wolf3d.h"
#endif


static const BE_GameVerDetails_T *g_be_gamever_ptrs[] = {
#ifdef REFKEEN_HAS_VER_KDREAMS
	&g_be_gamever_kdreamse100,
	&g_be_gamever_kdreamsc100,
	&g_be_gamever_kdreamse113,
	&g_be_gamever_kdreamsc105,
	&g_be_gamever_kdreamse193,
	&g_be_gamever_kdreamse120,
	&g_be_gamever_kdreams2015,
#endif
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
#ifdef REFKEEN_HAS_VER_WL1AP10
	&g_be_gamever_wl1ap10,
#endif
};

const BE_GameFileDetails_T *g_be_gamefiledetails_ptrs[]  = {
#ifdef REFKEEN_HAS_VER_KDREAMS
	g_be_reqgameverfiles_kdreamse100,
	g_be_reqgameverfiles_kdreamsc100,
	g_be_reqgameverfiles_kdreamse113,
	g_be_reqgameverfiles_kdreamsc105,
	g_be_reqgameverfiles_kdreamse193,
	g_be_reqgameverfiles_kdreamse120,
	g_be_reqgameverfiles_kdreams2015,
#endif
#ifdef REFKEEN_HAS_VER_CAT3D
	g_be_reqgameverfiles_cat3d100,
	g_be_reqgameverfiles_cat3d122,
#endif
#ifdef REFKEEN_HAS_VER_CATABYSS
	g_be_reqgameverfiles_catabyss113,
	g_be_reqgameverfiles_catabyss124,
#endif
#ifdef REFKEEN_HAS_VER_CATARM
	g_be_reqgameverfiles_catarm102,
#endif
#ifdef REFKEEN_HAS_VER_CATAPOC
	g_be_reqgameverfiles_catapoc101,
#endif
#ifdef REFKEEN_HAS_VER_WL1AP10
	g_be_reqgameverfiles_wl1ap10,
#endif
};
