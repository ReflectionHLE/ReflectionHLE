/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm1v11gm)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM1 Gold Medallion v1.1 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm1v11gm[] = {
	{"AUDIOHED.BM1", 580, 0x84dfc7b8},
	{"AUDIOT.BM1", 205754, 0x920e8faf},
	{"BM1GM.EXE", 90488, 0x56a5760e},
	{"EGADICT.BM1", 1024, 0x6d083cf4},
	{"EGAGRAPH.BM1", 514263, 0x54b4404e},
	{"EGAHEAD.BM1", 13869, 0x17476067},
	{"MAPHEAD.BM1", 22200, 0xb8ededc7},
	{"MAPTEMP.BM1", 165402, 0x2aea356c},
	{"FILE_ID.DIZ", 442, 0xef56dbbc},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm1v11gm[] = {
	{"INTROSCN.SCN", 4008, 0xd6dda9d4, 0x29720},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm1v11gm[] = {
	{
		g_be_embeddedgamefiles_bm1v11gm,
		NULL,
		NULL,
		"BM1GM.EXE",
		&REFKEEN_NS_ENCLOSE(bm1v11gm, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm1v11gm, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		239344 - 0x3800,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm1v11gm = {
	g_be_reqgameverfiles_bm1v11gm,
	g_be_exefiles_bm1v11gm,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM1V11GM),
	"BioMenace Ep1 Gold Medallion v1.1",
	REFKEEN_NS_ENCLOSE(bm1v11gm, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM1V11GM,
	BE_GAME_BMENACE_ALL
};
