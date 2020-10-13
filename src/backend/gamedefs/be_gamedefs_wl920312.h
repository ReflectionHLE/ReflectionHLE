/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl920312)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl920312[] = {
	{"AUDIOHED.WL1", 576, 0x7b9e338a},
	{"AUDIOT.WL1", 77265, 0x6df8532f},
	{"HELPART.WL1", 1103, 0x52fe336b},
	{"MAPHEAD.WL1", 402, 0xeb984d14},
	{"MAPTEMP.WL1", 62660, 0xcdd4c671},
	{"ORDERART.WL1", 150, 0xfa51dce3},
	{"VGADICT.WL1", 1024, 0x78092916},
	{"VGAGRAPH.WL1", 195624, 0x9d2c7f3b},
	{"VGAHEAD.WL1", 1281, 0x0e0aa60b},
	{"VSWAP.WL1", 561192, 0x80cb7a9e},
	{"WOLF1V.EXE", 83375, 0x92b9a37f},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl920312[] = {
	{"GAMEPAL.BIN", 768, 0x7f49ec2a, 0x2b0b0},
	{"SIGNON.BIN", 64000, 0xcc0bfd35, 0x1b6b0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl920312[] = {
	{
		g_be_embeddedgamefiles_wl920312,
		NULL,
		"WOLF1V.EXE",
		&REFKEEN_NS_ENCLOSE(wl920312, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl920312, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		269904 - 0x1A00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl920312 = {
	g_be_reqgameverfiles_wl920312,
	g_be_exefiles_wl920312,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL920312),
	"Wolfenstein 3D March '92 Proto.",
	REFKEEN_NS_ENCLOSE(wl920312, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL920312
};
