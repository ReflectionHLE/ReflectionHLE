/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl6ap11)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

// Registered v1.1

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6ap11[] = {
	{"AUDIOHED.WL6", 1156, 0x56ac3db},
	{"AUDIOT.WL6", 317049, 0x1585d305},
	{"GAMEMAPS.WL6", 150746, 0xdb7b0962},
	{"MAPHEAD.WL6", 402, 0x74b8087},
	{"VGADICT.WL6", 1024, 0x28687d1a},
	{"VGAGRAPH.WL6", 302374, 0xe1d6765f},
	{"VGAHEAD.WL6", 477, 0x53b4b582},
	{"VSWAP.WL6", 1545400, 0x97fe030d},
	{"WOLF3D.EXE", 98402, 0xeeb3decc},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl6ap11[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x22030},
	{"SIGNON.BIN", 64000, 0xb4ea01ff, 0x22330},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl6ap11[] = {
	{
		g_be_embeddedgamefiles_wl6ap11,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl6ap11, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl6ap11, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		297920 - 0x2600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl6ap11 = {
	g_be_reqgameverfiles_wl6ap11,
	g_be_exefiles_wl6ap11,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6AP11),
	"Wolfenstein 3D 6-ep Apogee v1.1 (Custom)",
	REFKEEN_NS_ENCLOSE(wl6ap11, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6AP11
};

// Registered v1.2 reused v1.1's exe

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl6ap12[] = {
	{"AUDIOHED.WL6", 1156, 0x56ac3db},
	{"AUDIOT.WL6", 317049, 0x1585d305},
	{"GAMEMAPS.WL6", 150758, 0x1029c727},
	{"MAPHEAD.WL6", 402, 0x5e43caee},
	{"VGADICT.WL6", 1024, 0x28687d1a},
	{"VGAGRAPH.WL6", 302374, 0xe1d6765f},
	{"VGAHEAD.WL6", 477, 0x53b4b582},
	{"VSWAP.WL6", 1545400, 0x97fe030d},
	{"WOLF3D.EXE", 98402, 0xeeb3decc},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl6ap12 = {
	g_be_reqgameverfiles_wl6ap12,
	g_be_exefiles_wl6ap11,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL6AP12),
	"Wolfenstein 3D 6-ep Apogee v1.2 (Custom)",
	REFKEEN_NS_ENCLOSE(wl6ap11, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL6AP12
};

// Shareware v1.2

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl1ap12[] = {
	{"AUDIOHED.WL1", 1156, 0x52133dc4},
	{"AUDIOT.WL1", 132613, 0x87bfc399},
	{"GAMEMAPS.WL1", 27450, 0x1751b3a1},
	{"MAPHEAD.WL1", 402, 0x0f499e20},
	{"VGADICT.WL1", 1024, 0x779a9b48},
	{"VGAGRAPH.WL1", 296813, 0xf7aa4e63},
	{"VGAHEAD.WL1", 462, 0xdaa52442},
	{"VSWAP.WL1", 742912, 0x708157b5},
	{"WOLF3D.EXE", 97676, 0x3dc4b0ce},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl1ap12[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x21c60},
	{"SIGNON.BIN", 64000, 0xb4ea01ff, 0x21f60},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl1ap12[] = {
	{
		g_be_embeddedgamefiles_wl1ap12,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl6ap11, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl6ap11, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		296720 - 0x2600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl1ap12 = {
	g_be_reqgameverfiles_wl1ap12,
	g_be_exefiles_wl1ap12,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL1AP12),
	"Wolfenstein 3D Shareware v1.2 (Custom)",
	REFKEEN_NS_ENCLOSE(wl6ap11, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL1AP12
};
