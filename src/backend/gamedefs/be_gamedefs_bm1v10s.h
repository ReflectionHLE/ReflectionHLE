/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm1v10s)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM1 shareware v1.0 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm1v10s[] = {
	{"AUDIOHED.BM1", 580, 0x84dfc7b8},
	{"AUDIOT.BM1", 205754, 0x920e8faf},
	{"BMENACE1.EXE", 89160, 0x6990e6a4},
	{"EGADICT.BM1", 1024, 0x33172d91},
	{"EGAGRAPH.BM1", 485148, 0x5aa722b7},
	{"EGAHEAD.BM1", 13863, 0x3e3bb196},
	{"MAPHEAD.BM1", 22200, 0xb8ededc7},
	{"MAPTEMP.BM1", 165402, 0x2aea356c},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm1v10s[] = {
	{"INTROSCN.SCN", 4008, 0x6ae1fb12, 0x27920},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm1v10s[] = {
	{
		g_be_embeddedgamefiles_bm1v10s,
		NULL,
		NULL,
		"BMENACE1.EXE",
		&REFKEEN_NS_ENCLOSE(bm1v10s, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm1v10s, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		235632 - 0x3600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm1v10s = {
	g_be_reqgameverfiles_bm1v10s,
	g_be_exefiles_bm1v10s,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM1V10S),
	"BioMenace Ep1 Shareware v1.0",
	REFKEEN_NS_ENCLOSE(bm1v10s, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM1V10S,
	BE_GAME_BMENACE_ALL
};
