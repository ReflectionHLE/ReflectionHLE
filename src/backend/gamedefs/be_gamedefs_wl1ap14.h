/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl1ap14)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

// Registered v1.4

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6ap14[] = {
	{"AUDIOHED.WL6", 1156, 0x2f3bcb70},
	{"AUDIOT.WL6", 320209, 0xc79efd4c},
	{"GAMEMAPS.WL6", 150652, 0xada5c827},
	{"MAPHEAD.WL6", 402, 0x15803e9},
	{"VGADICT.WL6", 1024, 0xbe27100},
	{"VGAGRAPH.WL6", 334506, 0x6aa208a5},
	{"VGAHEAD.WL6", 486, 0xeddd83f8},
	{"VSWAP.WL6", 1545400, 0x859e8ffc},
	{"WOLF3D.EXE", 110715, 0xfbe11eb3},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl6ap14[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3ab36},
	{"SIGNON.BIN", 64000, 0x882b2064, 0x21f30},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl6ap14[] = {
	{
		g_be_embeddedgamefiles_wl6ap14,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl1ap14, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl1ap14, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		297968 - 0x2600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl6ap14 = {
	g_be_reqgameverfiles_wl6ap14,
	g_be_exefiles_wl6ap14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6AP14),
	"Wolfenstein 3D 6-ep Apogee v1.4",
	REFKEEN_NS_ENCLOSE(wl1ap14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6AP14
};

// Shareware v1.4

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl1ap14[] = {
	{"AUDIOHED.WL1", 1156, 0x52133dc4},
	{"AUDIOT.WL1", 132613, 0x87bfc399},
	{"GAMEMAPS.WL1", 27425, 0xcc53d341},
	{"MAPHEAD.WL1", 402, 0x088f7551},
	{"VGADICT.WL1", 1024, 0x358ab877},
	{"VGAGRAPH.WL1", 326568, 0x746da9f0},
	{"VGAHEAD.WL1", 471, 0xa60a98f0},
	{"VSWAP.WL1", 742912, 0x85efac55},
	{"WOLF3D.EXE", 109959, 0x213d5d76},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl1ap14[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3a68a},
	{"SIGNON.BIN", 64000, 0x882b2064, 0x21b60},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl1ap14[] = {
	{
		g_be_embeddedgamefiles_wl1ap14,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl1ap14, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl1ap14, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		296256 - 0x2400,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl1ap14 = {
	g_be_reqgameverfiles_wl1ap14,
	g_be_exefiles_wl1ap14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL1AP14),
	"Wolfenstein 3D Shareware v1.4",
	REFKEEN_NS_ENCLOSE(wl1ap14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL1AP14
};
