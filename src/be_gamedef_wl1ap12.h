/* This header must be included *only* from be_gamedef_wolf3d.h */

REFKEEN_NS_B_FOR(wl1ap12)

void RefKeen_Patch_wl_play(void);
//void RefKeen_FillObjStatesWithDOSPointers(void);
//void RefKeen_PrepareAltControllerScheme(void);

static void (*g_be_patcherfuncs_wl1ap12[])(void) = {
	RefKeen_Patch_wl_play,
	/*RefKeen_FillObjStatesWithDOSPointers, RefKeen_PrepareAltControllerScheme,*/ 0
};

void wolf3d_exe_main(void);
void RefKeen_Load_Embedded_Resources_From_wolf3d_exe(void);

REFKEEN_NS_E

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

static const BE_EXEFileDetails_T g_be_exefiles_wl1ap12[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"GAMEPAL.BIN", 768, 0x7e2ef7db, 0x21c60},
			{"SIGNON.BIN", 64000, 0xb4ea01ff, 0x21f60},
			{0}
		},

		NULL,
		"WOLF3D.EXE",
		&REFKEEN_NS_ENCLOSE(wl1ap12, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(wl1ap12, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
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
	REFKEEN_NS_ENCLOSE(wl1ap12, g_be_patcherfuncs_wl1ap12),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_WL1AP12
};
