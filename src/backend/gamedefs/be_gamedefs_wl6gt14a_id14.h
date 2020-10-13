/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl6gt14a)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

// The original EXEs of these versions differ just by the SIGNON screen

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6gt14a[] = {
	{"AUDIOHED.WL6", 1156, 0x2f3bcb70},
	{"AUDIOT.WL6", 320209, 0xc79efd4c},
	{"GAMEMAPS.WL6", 150652, 0xada5c827},
	{"MAPHEAD.WL6", 402, 0x015803e9},
	{"VGADICT.WL6", 1024, 0x8b4ad0f8},
	{"VGAGRAPH.WL6", 276096, 0xe0df1ea0},
	{"VGAHEAD.WL6", 450, 0x27ee37c4},
	{"VSWAP.WL6", 1545400, 0x859e8ffc},
	{"WOLF3D.EXE", 259310, 0xcf660f1b},
	{0}
};

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6id14[] = {
	{"AUDIOHED.WL6", 1156, 0x2f3bcb70},
	{"AUDIOT.WL6", 320209, 0xc79efd4c},
	{"GAMEMAPS.WL6", 150652, 0xada5c827},
	{"MAPHEAD.WL6", 402, 0x015803e9},
	{"VGADICT.WL6", 1024, 0x8b4ad0f8},
	{"VGAGRAPH.WL6", 276096, 0xe0df1ea0},
	{"VGAHEAD.WL6", 450, 0x27ee37c4},
	{"VSWAP.WL6", 1545400, 0x859e8ffc},
	{"WOLF3D.EXE", 259310, 0xded9ff6f},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl6gt14a[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3b0c8},
	{"SIGNON.BIN", 64000, 0xf4e0cbf5, 0x22010},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl6id14[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3b0c8},
	{"SIGNON.BIN", 64000, 0xa64f3090, 0x22010},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl6gt14a[] = {
	{
		g_be_embeddedgamefiles_wl6gt14a,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl6gt14a, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl6gt14a, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		257774 - 0x2600,
		BE_EXECOMPRESSION_NONE,
		false
	},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl6id14[] = {
	{
		g_be_embeddedgamefiles_wl6id14,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl6gt14a, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl6gt14a, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		257774 - 0x2600,
		BE_EXECOMPRESSION_NONE,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl6gt14a = {
	g_be_reqgameverfiles_wl6gt14a,
	g_be_exefiles_wl6gt14a,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6GT14A),
	"Wolfenstein 3D: GT #1",
	REFKEEN_NS_ENCLOSE(wl6gt14a, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6GT14A
};

static const BE_GameVerDetails_T g_be_gamever_wl6id14 = {
	g_be_reqgameverfiles_wl6id14,
	g_be_exefiles_wl6id14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6ID14),
	"Wolfenstein 3D: id",
	REFKEEN_NS_ENCLOSE(wl6gt14a, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6ID14
};
