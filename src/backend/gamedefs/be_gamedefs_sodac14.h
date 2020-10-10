/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(sodac14)
#include "be_gamedefs_wolf3d_patchers.h"
REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_sodac14[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD|GAMEMAPS.SD1|GAMEMAPS.SD4", 68290, 0xb564a0be},
	{"MAPHEAD.SOD|MAPHEAD.SD1|MAPHEAD.SD4", 402, 0xcdb40ca8},
	{"SPEAR.EXE", 113418, 0x4d38a060},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD|VSWAP.SD1|VSWAP.SD4", 1616544, 0x591cc4dc},
	{0}
};

static const BE_GameFileDetails_T g_be_reqgameverfiles_sd2ac14[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD|GAMEMAPS.SD2|GAMEMAPS.SD4", 135024, 0x5110f693},
	{"MAPHEAD.SOD|MAPHEAD.SD2|MAPHEAD.SD4", 86, 0xdd7bb542},
	{"SPEAR.EXE", 113418, 0x4d38a060},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD|VSWAP.SD2|VSWAP.SD4", 1879712, 0x861dcbf7},
	{0}
};

static const BE_GameFileDetails_T g_be_reqgameverfiles_sd3ac14[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD|GAMEMAPS.SD3|GAMEMAPS.SD4", 148534, 0x0b941824},
	{"MAPHEAD.SOD|MAPHEAD.SD3|MAPHEAD.SD4", 86, 0x4e55a796},
	{"SPEAR.EXE", 113418, 0x4d38a060},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD|VSWAP.SD3|VSWAP.SD4", 1881760, 0x01db521a},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_sodac14[] = {
	{"GAMEPAL.BIN", 768, 0xbd030b1d, 0x3d35c},
	{"SIGNON.BIN", 64000, 0x1644999b, 0x24690},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_sodac14[] = {
	{
		g_be_embeddedgamefiles_sodac14,
		NULL,
		"SPEAR.EXE",
		&REFKEEN_NS_ENCLOSE(sodac14, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(sodac14, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		266110 - 0x2400,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_sodac14 = {
	g_be_reqgameverfiles_sodac14,
	g_be_exefiles_sodac14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SODAC14),
	"Spear of Destiny Activision v1.4 (Custom)",
	REFKEEN_NS_ENCLOSE(sodac14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SODAC14
};

static const BE_GameVerDetails_T g_be_gamever_sd2ac14 = {
	g_be_reqgameverfiles_sd2ac14,
	g_be_exefiles_sodac14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SD2AC14),
	"Spear of Destiny M2 Activision v1.4 (Custom)",
	REFKEEN_NS_ENCLOSE(sodac14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SD2AC14
};

static const BE_GameVerDetails_T g_be_gamever_sd3ac14 = {
	g_be_reqgameverfiles_sd3ac14,
	g_be_exefiles_sodac14,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SD3AC14),
	"Spear of Destiny Activision v1.4 (Custom)",
	REFKEEN_NS_ENCLOSE(sodac14, g_be_patcherfuncs),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SD3AC14
};
