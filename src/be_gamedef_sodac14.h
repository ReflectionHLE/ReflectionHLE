/* This header must be included *only* from be_gamedef_wolf3d.h */

REFKEEN_NS_B_FOR(sodac14)

void RefKeen_Patch_wl_play(void);
//void RefKeen_FillObjStatesWithDOSPointers(void);
//void RefKeen_PrepareAltControllerScheme(void);

static void (*g_be_patcherfuncs_sodac14[])(void) = {
	RefKeen_Patch_wl_play,
	/*RefKeen_FillObjStatesWithDOSPointers, RefKeen_PrepareAltControllerScheme,*/ 0
};

void wolf3d_exe_main(void);
void RefKeen_Load_Embedded_Resources_From_wolf3d_exe(void);

REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_sodac14[] = {
	{"AUDIOHED.SOD", 1072, 0x13ce3958},
	{"AUDIOT.SOD", 328620, 0x96d183ec},
	{"GAMEMAPS.SOD", 68290, 0xb564a0be},
	{"MAPHEAD.SOD", 402, 0xcdb40ca8},
	{"SPEAR.EXE", 113418, 0x4d38a060},
	{"VGADICT.SOD", 1024, 0xb9b960ac},
	{"VGAGRAPH.SOD", 947979, 0x5936dddb},
	{"VGAHEAD.SOD", 510, 0xdfb1b473},
	{"VSWAP.SOD", 1616544, 0x591cc4dc},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_sodac14[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"GAMEPAL.BIN", 768, 0xbd030b1d, 0x3d35c},
			{"SIGNON.BIN", 64000, 0x1644999b, 0x24690},
			{0}
		},

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
	REFKEEN_NS_ENCLOSE(sodac14, g_be_patcherfuncs_sodac14),
	0,
	BE_GAMEVER_SODAC14
};
