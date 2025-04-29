/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm3v11f)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM3 freeware v1.1 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm3v11f[] = {
	{"AUDIOHED.BM3", 552, 0x7db93b51},
	{"AUDIOT.BM3", 201662, 0x2e831f51},
	{"BMENACE3.EXE", 89754, 0xa85c7923},
	{"EGADICT.BM3", 1024, 0x9256a0be},
	{"EGAGRAPH.BM3", 412364, 0xa3303aa3},
	{"EGAHEAD.BM3", 12693, 0xa90ed136},
	{"MAPHEAD.BM3", 21336, 0x6aa1b2fd},
	{"MAPTEMP.BM3", 197166, 0x17118070},
	{"FILE_ID.DIZ", 442, 0xef56dbbc},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm3v11f[] = {
	{"INTROSCN.SCN", 4008, 0x12594cbe, 0x28290},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm3v11f[] = {
	{
		g_be_embeddedgamefiles_bm3v11f,
		NULL,
		NULL,
		"BMENACE3.EXE",
		&REFKEEN_NS_ENCLOSE(bm3v11f, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm3v11f, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		238160 - 0x3A00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm3v11f = {
	g_be_reqgameverfiles_bm3v11f,
	g_be_exefiles_bm3v11f,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM3V11F),
	"BioMenace Ep3 Freeware v1.1",
	REFKEEN_NS_ENCLOSE(bm3v11f, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM3V11F,
	BE_GAME_BMENACE_ALL
};

