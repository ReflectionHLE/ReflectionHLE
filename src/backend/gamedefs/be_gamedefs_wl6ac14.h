/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl6ac14)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6ac14[] = {
	{"AUDIOHED.WL6", 1156, 0x2f3bcb70},
	{"AUDIOT.WL6", 320209, 0xc79efd4c},
	{"GAMEMAPS.WL6", 150652, 0xada5c827},
	{"MAPHEAD.WL6", 402, 0x015803e9},
	{"VGADICT.WL6", 1024, 0xaa22054e},
	{"VGAGRAPH.WL6", 275774, 0xa4b1de1d},
	{"VGAHEAD.WL6", 450, 0xab2b44d3},
	{"VSWAP.WL6", 1544376, 0xe610c664},
	{"WOLF3D.EXE", 108779, 0x6d818dbe},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl6ac14[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3c81a},
	{"SIGNON.BIN", 64000, 0x7433e826, 0x23760},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl6ac14[] = {
	{
		g_be_embeddedgamefiles_wl6ac14,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl6ac14, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl6ac14, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		263230 - 0x2400,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl6ac14 = {
	g_be_reqgameverfiles_wl6ac14,
	g_be_exefiles_wl6ac14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6AC14),
	"Wolfenstein 3D Activision v1.4 (Custom)",
	REFKEEN_NS_ENCLOSE(wl6ac14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6AC14
};
