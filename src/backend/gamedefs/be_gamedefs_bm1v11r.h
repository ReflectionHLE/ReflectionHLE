/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm1v11r)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM1 registered v1.1 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm1v11r[] = {
	{"AUDIOHED.BM1", 580, 0x84dfc7b8},
	{"AUDIOT.BM1", 205754, 0x920e8faf},
	{"BMENACE1.EXE", 89274, 0xa445576c},
	{"EGADICT.BM1", 1024, 0x629503df},
	{"EGAGRAPH.BM1", 501816, 0x2454d561},
	{"EGAHEAD.BM1", 13866, 0x77c23274},
	{"FILE_ID.DIZ", 442, 0xef56dbbc},
	{"MAPHEAD.BM1", 22200, 0xb8ededc7},
	{"MAPTEMP.BM1", 165402, 0x2aea356c},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm1v11r[] = {
	{"INTROSCN.SCN", 4008, 0xd6dda9d4, 0x279c0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm1v11r[] = {
	{
		g_be_embeddedgamefiles_bm1v11r,
		NULL,
		NULL,
		"BMENACE1.EXE",
		&REFKEEN_NS_ENCLOSE(bm1v11r, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm1v11r, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		235888 - 0x3600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm1v11r = {
	g_be_reqgameverfiles_bm1v11r,
	g_be_exefiles_bm1v11r,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM1V11R),
	"BioMenace Ep1 Registered v1.1",
	REFKEEN_NS_ENCLOSE(bm1v11r, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM1V11R,
	BE_GAME_BMENACE_ALL
};
