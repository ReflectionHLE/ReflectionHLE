/* This header must be included *only* from be_gamedefs_wolf3d.h */

REFKEEN_NS_B_FOR(sdmfg10)

void RefKeen_Patch_wl_play(void);
//void RefKeen_FillObjStatesWithDOSPointers(void);
//void RefKeen_PrepareAltControllerScheme(void);

static void (*g_be_patcherfuncs_sdmfg10[])(void) = {
	RefKeen_Patch_wl_play,
	/*RefKeen_FillObjStatesWithDOSPointers, RefKeen_PrepareAltControllerScheme,*/ 0
};

void wolf3d_exe_main(void);
void RefKeen_Load_Embedded_Resources_From_wolf3d_exe(void);

REFKEEN_NS_E

static const BE_GameFileDetails_T g_be_reqgameverfiles_sdmfg10[] = {
	{"AUDIOHED.SDM", 1072, 0xb6ba466b},
	{"AUDIOT.SDM", 93412, 0x5ca9f085},
	{"GAMEMAPS.SDM", 5534, 0xc9f3afee},
	{"MAPHEAD.SDM", 402, 0xb2b8f1fd},
	{"SOD.EXE", 108215, 0x0d81eedc},
	{"VGADICT.SDM", 1024, 0x5723eb53},
	{"VGAGRAPH.SDM", 305891, 0x52182e63},
	{"VGAHEAD.SDM", 402, 0x3f4ad2e4},
	{"VSWAP.SDM", 730624, 0x5e8cc78b},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_sdmfg10[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"GAMEPAL.BIN", 768, 0xbd030b1d, 0x2fef0},
			{"SIGNON.BIN", 64000, 0x07cf2545, 0x204f0},
			{0}
		},

		NULL,
		"SOD.EXE",
		&REFKEEN_NS_ENCLOSE(sdmfg10, wolf3d_exe_main),
		&REFKEEN_NS_ENCLOSE(sdmfg10, RefKeen_Load_Embedded_Resources_From_wolf3d_exe),
		246430 - 0x2200,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_sdmfg10 = {
	g_be_reqgameverfiles_sdmfg10,
	g_be_exefiles_sdmfg10,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_SDMFG10),
	"Spear of Destiny Demo v1.0 (Custom)",
	REFKEEN_NS_ENCLOSE(sdmfg10, g_be_patcherfuncs_sdmfg10),
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL | BE_AUDIO_DEVICE_DIGI,
	BE_GAMEVER_SDMFG10
};
