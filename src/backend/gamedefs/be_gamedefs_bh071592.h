/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bh071592)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BH1 (BM2) July 1992 beta ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bh071592[] = {
	{"ABOUT.BH1", 102, 0x65a39961},
	{"AUDIOHED.BH1", 528, 0x6505e79e},
	{"AUDIOT.BH1", 225140, 0xff6ab4d3},
	{"BHAZARD1.EXE", 83840, 0x5f5a337c}, // NOTE: Patched from 1992 file
//	{"BHAZARD1.EXE", 83840, 0x8f2daa9c},
	{"CONTRART.BH1", 2936, 0x57426cdd},
	{"DEMO0.BH1", 304, 0xf47e6a70},
	{"EGADICT.BH1", 1024, 0xeb176e79},
	{"EGAGRAPH.BH1", 430864, 0x0d13bb9a}, // NOTE: Fixed from 1992 file
//	{"EGAGRAPH.BH1", 430864, 0xbc2b6787},
	{"EGAHEAD.BH1", 13794, 0x0f401bd9},
	{"ENDART.BH1", 361, 0xf54bba4c},
	{"HELPART.BH1", 4062, 0xb52a2dea},
	{"MAPHEAD.BH1", 21120, 0x73a78f95},
	{"MAPTEMP.BH1", 171126, 0x94657a52},
	{"ORDERART.BH1", 4724, 0x61bf89da},
	{"STORYART.BH1", 4062, 0xb52a2dea},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bh071592[] = {
	{"INTROSCN.SCN", 4008, 0x47db3158, 0x23470},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bh071592[] = {
	{
		g_be_embeddedgamefiles_bh071592,
		NULL,
		NULL,
		"BHAZARD1.EXE",
		&REFKEEN_NS_ENCLOSE(bh071592, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bh071592, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		220848 - 0x2C00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bh071592 = {
	g_be_reqgameverfiles_bh071592,
	g_be_exefiles_bh071592,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BH071592),
	"BioHazard (BioMenace) 1992-7 Beta",
	REFKEEN_NS_ENCLOSE(bh071592, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BH071592,
	BE_GAME_BMENACE_ALL
};
