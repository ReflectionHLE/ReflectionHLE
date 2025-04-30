/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm1v10r)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM1 registered v1.0 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm1v10r[] = {
	{"AUDIOHED.BM1", 580, 0x84dfc7b8},
	{"AUDIOT.BM1", 205754, 0x920e8faf},
	{"BMENACE1.EXE", 89270, 0x130d4552},
	{"EGADICT.BM1", 1024, 0x33172d91},
	{"EGAGRAPH.BM1", 485148, 0x5aa722b7},
	{"EGAHEAD.BM1", 13863, 0x3e3bb196},
	{"FILE_ID.DIZ", 442, 0xef56dbbc},
	{"MAPHEAD.BM1", 22200, 0xb8ededc7},
	{"MAPTEMP.BM1", 165402, 0x2aea356c},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm1v10r[] = {
	{"INTROSCN.SCN", 4008, 0x6ae1fb12, 0x279c0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm1v10r[] = {
	{
		g_be_embeddedgamefiles_bm1v10r,
		NULL,
		NULL,
		"BMENACE1.EXE",
		&REFKEEN_NS_ENCLOSE(bm1v10r, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm1v10r, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		235872 - 0x3600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm1v10r = {
	g_be_reqgameverfiles_bm1v10r,
	g_be_exefiles_bm1v10r,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM1V10R),
	"BioMenace Ep1 Registered v1.0",
	REFKEEN_NS_ENCLOSE(bm1v10r, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM1V10R,
	BE_GAME_BMENACE_ALL
};
