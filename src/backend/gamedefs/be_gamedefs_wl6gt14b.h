/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl6gt14b)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6gt14b[] = {
	{"AUDIOHED.WL6", 1156, 0x2f3bcb70},
	{"AUDIOT.WL6", 320209, 0xc79efd4c},
	{"GAMEMAPS.WL6", 150652, 0xada5c827},
	{"MAPHEAD.WL6", 402, 0x015803e9},
	{"VGADICT.WL6", 1024, 0xaa22054e},
	{"VGAGRAPH.WL6", 275774, 0xa4b1de1d},
	{"VGAHEAD.WL6", 450, 0xab2b44d3},
	{"VSWAP.WL6", 1544376, 0xe610c664},
	{"WOLF3D.EXE", 109589, 0xfcdf5431},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl6gt14b[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3afc8},
	{"SIGNON.BIN", 64000, 0xf4e0cbf5, 0x21f10},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl6gt14b[] = {
	{
		g_be_embeddedgamefiles_wl6gt14b,
		NULL,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl6gt14b, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl6gt14b, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		257518 - 0x2600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl6gt14b = {
	g_be_reqgameverfiles_wl6gt14b,
	g_be_exefiles_wl6gt14b,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6GT14B),
	"Wolfenstein 3D: GT #2",
	REFKEEN_NS_ENCLOSE(wl6gt14b, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6GT14B,
	BE_GAME_WOLF3D_ALL
};
