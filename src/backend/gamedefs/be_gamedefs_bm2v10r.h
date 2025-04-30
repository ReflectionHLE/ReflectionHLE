/* This header must be included *only* from be_gamedefs_bmenace.h */

REFKEEN_NS_B_FOR(bm2v10r)
#include "be_gamedefs_bmenace_patchers.h"
REFKEEN_NS_E

/*** BM2 registered v1.0 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_bm2v10r[] = {
	{"AUDIOHED.BM2", 564, 0xd007b1fb},
	{"AUDIOT.BM2", 244224, 0xfdd31c14},
	{"BMENACE2.EXE", 89245, 0xd0c3a77d},
	{"EGADICT.BM2", 1024, 0x1d5b8daa},
	{"EGAGRAPH.BM2", 479241, 0x0633cb47},
	{"EGAHEAD.BM2", 13947, 0xc3d8b425},
	{"FILE_ID.DIZ", 442, 0xef56dbbc},
	{"MAPHEAD.BM2", 21120, 0x89a765d5},
	{"MAPTEMP.BM2", 196526, 0x67bfed4c},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_bm2v10r[] = {
	{"INTROSCN.SCN", 4008, 0x1f71d6b4, 0x27ac0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_bm2v10r[] = {
	{
		g_be_embeddedgamefiles_bm2v10r,
		NULL,
		NULL,
		"BMENACE2.EXE",
		&REFKEEN_NS_ENCLOSE(bm2v10r, bmenace_exe_main),
		&REFKEEN_NS_ENCLOSE(bm2v10r, RefKeen_Load_Embedded_Resources_From_bmenace_exe),
		236736 - 0x3800,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_bm2v10r = {
	g_be_reqgameverfiles_bm2v10r,
	g_be_exefiles_bm2v10r,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_BM2V10R),
	"BioMenace Ep2 Registered v1.0",
	REFKEEN_NS_ENCLOSE(bm2v10r, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_BM2V10R,
	BE_GAME_BMENACE_ALL
};
