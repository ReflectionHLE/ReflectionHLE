/* This header must be included *only* from be_gamedef_wolf3d.h */

REFKEEN_NS_B_FOR(wl1ap14)

void RefKeen_Patch_wl_play(void);
//void RefKeen_FillObjStatesWithDOSPointers(void);
//void RefKeen_PrepareAltControllerScheme(void);

static void (*g_be_patcherfuncs_wl1ap14[])(void) = {
	RefKeen_Patch_wl_play,
	/*RefKeen_FillObjStatesWithDOSPointers, RefKeen_PrepareAltControllerScheme,*/ 0
};

void wolf3d_exe_main(void);
void RefKeen_Load_Embedded_Resources_From_wolf3d_exe(void);

REFKEEN_NS_E

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

static const BE_EXEFileDetails_T g_be_exefiles_wl1ap14[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x3a68a},
			{"SIGNON.BIN", 64000, 0x882b2064, 0x21b60},
			{0}
		},

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
	"Wolfenstein 3D Shareware v1.4 (Custom)",
	REFKEEN_NS_ENCLOSE(wl1ap14, g_be_patcherfuncs_wl1ap14),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL1AP14
};
