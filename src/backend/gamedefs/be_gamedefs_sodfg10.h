/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(sodfg10)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_sodfg10[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD|GAMEMAPS.SD1|GAMEMAPS.SD4", 68290, 0xb564a0be},
	{"MAPHEAD.SOD|MAPHEAD.SD1|MAPHEAD.SD4", 402, 0xcdb40ca8},
	{"SPEAR.EXE", 113273, 0xbeebc1f9},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD|VSWAP.SD1|VSWAP.SD4", 1616544, 0x591cc4dc},
	{0}
};

static const BE_GameFileDetails_T g_be_reqgameverfiles_sd2fg10[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD|GAMEMAPS.SD2|GAMEMAPS.SD4", 135024, 0x5110f693},
	{"MAPHEAD.SOD|MAPHEAD.SD2|MAPHEAD.SD4", 86, 0xdd7bb542},
	{"SPEAR.EXE", 113273, 0xbeebc1f9},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD|VSWAP.SD2|VSWAP.SD4", 1879712, 0x861dcbf7},
	{0}
};

static const BE_GameFileDetails_T g_be_reqgameverfiles_sd3fg10[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD|GAMEMAPS.SD3|GAMEMAPS.SD4", 148534, 0x0b941824},
	{"MAPHEAD.SOD|MAPHEAD.SD3|MAPHEAD.SD4", 86, 0x4e55a796},
	{"SPEAR.EXE", 113273, 0xbeebc1f9},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD|VSWAP.SD3|VSWAP.SD4", 1881760, 0x01db521a},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_sodfg10[] = {
	{"GAMEPAL.BIN", 768, 0xbd030b1d, 0x31d60},
	{"SIGNON.BIN", 64000, 0x3bef476e, 0x22360},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_sodfg10[] = {
	{
		g_be_embeddedgamefiles_sodfg10,
		NULL,
		NULL,
		"SPEAR.EXE",
		&REFKEEN_NS_ENCLOSE(sodfg10, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(sodfg10, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		297504 - 0x2600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_sodfg10 = {
	g_be_reqgameverfiles_sodfg10,
	g_be_exefiles_sodfg10,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SODFG10),
	"Spear of Destiny Forgmen v1.0",
	REFKEEN_NS_ENCLOSE(sodfg10, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SODFG10
};

static const BE_GameVerDetails_T g_be_gamever_sd2fg10 = {
	g_be_reqgameverfiles_sd2fg10,
	g_be_exefiles_sodfg10,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SD2FG10),
	"Spear of Destiny M2 Formgen v1.0",
	REFKEEN_NS_ENCLOSE(sodfg10, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SD2FG10
};

static const BE_GameVerDetails_T g_be_gamever_sd3fg10 = {
	g_be_reqgameverfiles_sd3fg10,
	g_be_exefiles_sodfg10,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SD3FG10),
	"Spear of Destiny M3 Formgen v1.0",
	REFKEEN_NS_ENCLOSE(sodfg10, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SD3FG10
};
