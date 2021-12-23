/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(n3dwt10)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_n3dwt10[] = {
	{"AUDIOHED.N3D", 576, 0x769028e9},
	{"AUDIOT.N3D", 45043, 0x10022c23},
	{"GAMEMAPS.N3D", 60195, 0x5cb6a201},
	{"MAPHEAD.N3D", 402, 0x420cd93c},
	{"NOAH3D.EXE|NOAH3DOS.EXE", 448468, 0xbac8f80a},
	{"VGADICT.N3D", 1024, 0x11390697},
	{"VGAGRAPH.N3D", 235451, 0xb203e02e},
	{"VGAHEAD.N3D", 405, 0x841542a2},
	{"VSWAP.N3D", 1048708, 0xd676f182},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_n3dwt10[] = {
	{"GAMEPAL.BIN", 768, 0xc76ff953, 0x280e0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_n3dwt10[] = {
	{
		g_be_embeddedgamefiles_n3dwt10,
		NULL,
		NULL,
		"NOAH3D.EXE|NOAH3DOS.EXE",
		&REFKEEN_NS_ENCLOSE(n3dwt10, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(n3dwt10, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		260736 - 0x3000,
		BE_EXECOMPRESSION_NONE,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_n3dwt10 = {
	g_be_reqgameverfiles_n3dwt10,
	g_be_exefiles_n3dwt10,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_N3DWT10),
	"Super 3-D Noah's Ark",
	REFKEEN_NS_ENCLOSE(n3dwt10, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_N3DWT10,
	BE_GAME_WOLF3D_ALL
};
