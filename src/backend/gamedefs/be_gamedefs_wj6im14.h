/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wj6im14)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_wj6im14[] = {
	{"AUDIOHED.WJ6", 1156, 0x2f3bcb70},
	{"AUDIOT.WJ6", 320209, 0xc79efd4c},
	{"GAMEMAPS.WJ6", 150652, 0xada5c827},
	{"MAPHEAD.WJ6", 402, 0x015803e9},
	{"VGADICT.WJ6", 1024, 0xd7d6cd64},
	{"VGAGRAPH.WJ6", 1178641, 0xa64ea29f},
	{"VGAHEAD.WJ6", 516, 0x44cbff76},
	{"VSWAP.WJ6", 1545400, 0x859e8ffc},
	{"WOLF3D_J.EXE", 254046, 0x35a74dd2},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wj6im14[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x39e38},
	{"SIGNON.BIN", 64000, 0xc95d1612, 0x21140},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wj6im14[] = {
	{
		g_be_embeddedgamefiles_wj6im14,
		NULL,
		NULL,
		"WOLF3D_J.EXE",
		&REFKEEN_NS_ENCLOSE(wj6im14, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wj6im14, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		254046 - 0x2A00,
		BE_EXECOMPRESSION_NONE,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wj6im14 = {
	g_be_reqgameverfiles_wj6im14,
	g_be_exefiles_wj6im14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WJ6IM14),
	"Wolfenstein 3D: Imagineer",
	REFKEEN_NS_ENCLOSE(wj6im14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WJ6IM14
};

