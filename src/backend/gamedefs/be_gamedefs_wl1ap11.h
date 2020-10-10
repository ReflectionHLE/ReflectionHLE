/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl1ap11)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl1ap11[] = {
	{"AUDIOHED.WL1", 1156, 0x52133dc4},
	{"AUDIOT.WL1", 132613, 0x73569f4d},
	{"GAMEMAPS.WL1", 27448, 0x7750e1d4},
	{"MAPHEAD.WL1", 402, 0x205253ac},
	{"VGADICT.WL1", 1024, 0xfcad3538},
	{"VGAGRAPH.WL1", 296826, 0x9cb49fea},
	{"VGAHEAD.WL1", 462, 0xd3b22c3c},
	{"VSWAP.WL1", 742912, 0xbb85f24c},
	{"WOLF3D.EXE", 97605, 0xbef1a88b},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl1ap11[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x21c80},
	{"SIGNON.BIN", 64000, 0xb4ea01ff, 0x21f80},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl1ap11[] = {
	{
		g_be_embeddedgamefiles_wl1ap11,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl1ap11, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl1ap11, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		295232 - 0x2600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl1ap11 = {
	g_be_reqgameverfiles_wl1ap11,
	g_be_exefiles_wl1ap11,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL1AP11),
	"Wolfenstein 3D Shareware v1.1 (Custom)",
	REFKEEN_NS_ENCLOSE(wl1ap11, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL1AP11
};
