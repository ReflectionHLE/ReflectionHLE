/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm2v11f)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM2 freeware v1.1 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm2v11f[] = {
	{"AUDIOHED.BM2", 564, 0xd007b1fb},
	{"AUDIOT.BM2", 244224, 0xfdd31c14},
	{"BMENACE2.EXE", 89921, 0xb82db8db},
	{"EGADICT.BM2", 1024, 0x1d5b8daa},
	{"EGAGRAPH.BM2", 479241, 0x0633cb47},
	{"EGAHEAD.BM2", 13947, 0xc3d8b425},
	{"MAPHEAD.BM2", 21120, 0x89a765d5},
	{"MAPTEMP.BM2", 196526, 0x67bfed4c},
	{"FILE_ID.DIZ", 442, 0xef56dbbc},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm2v11f[] = {
	{"INTROSCN.SCN", 4008, 0x842b796b, 0x28000},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm2v11f[] = {
	{
		g_be_embeddedgamefiles_bm2v11f,
		NULL,
		NULL,
		"BMENACE2.EXE",
		&REFKEEN_NS_ENCLOSE(bm2v11f, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm2v11f, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		238208 - 0x3800,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm2v11f = {
	g_be_reqgameverfiles_bm2v11f,
	g_be_exefiles_bm2v11f,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM2V11F),
	"BioMenace Ep2 Freeware v1.1",
	REFKEEN_NS_ENCLOSE(bm2v11f, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM2V11F,
	BE_GAME_BMENACE_ALL
};
