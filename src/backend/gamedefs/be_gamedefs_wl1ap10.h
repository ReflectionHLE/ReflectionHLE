/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(wl1ap10)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_wl1ap10[] = {
	{"AUDIOHED.WL1", 988, 0x384a9496},
	{"AUDIOT.WL1", 130696, 0xf65031a7},
	{"MAPHEAD.WL1", 402, 0xe35f606a},
	{"MAPTEMP.WL1", 56618, 0xf323bce2},
	{"VGADICT.WL1", 1024, 0x3796b7e2},
	{"VGAGRAPH.WL1", 295394, 0xf431f4a4},
	{"VGAHEAD.WL1", 447, 0x1343524f},
	{"VSWAP.WL1", 707072, 0xf97fe230},
	{"WOLF3D.EXE", 94379, 0x6c6bac36},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_wl1ap10[] = {
	{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x1f950},
	{"SIGNON.BIN", 64000, 0x9c00fb86, 0x1fc50},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_wl1ap10[] = {
	{
		g_be_embeddedgamefiles_wl1ap10,
		NULL,
		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl1ap10, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl1ap10, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		296528 - 0x2400,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_wl1ap10 = {
	g_be_reqgameverfiles_wl1ap10,
	g_be_exefiles_wl1ap10,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_WL1AP10),
	"Wolfenstein 3D Shareware v1.0",
	REFKEEN_NS_ENCLOSE(wl1ap10, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL1AP10,
	BE_GAME_WOLF3D_ALL
};
