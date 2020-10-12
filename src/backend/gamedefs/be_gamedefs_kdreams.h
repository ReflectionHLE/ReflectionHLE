/* This header must be included *only* from be_gamedefs.h */

void RefKeen_Patch_id_ca(void);
void RefKeen_Patch_id_us(void);
void RefKeen_Patch_id_rf(void);
void RefKeen_Patch_id_rf_a(void);
void RefKeen_Patch_id_vw(void);
void RefKeen_Patch_id_vw_ac(void);
void RefKeen_Patch_id_vw_ae(void);
void RefKeen_Patch_kd_demo(void);
void RefKeen_Patch_kd_keen(void);
void RefKeen_Patch_kd_play(void);
void RefKeen_FillObjStatesWithDOSPointers(void);
void RefKeen_PrepareAltControllerScheme(void);

// Note: RefKeen_Patch_id_ca MUST be first for Keen Dreams
static void (*g_be_patcherfuncs_kdreams[])(void) = {
	RefKeen_Patch_id_ca, RefKeen_Patch_id_us,
	RefKeen_Patch_id_rf, RefKeen_Patch_id_rf_a,
	RefKeen_Patch_id_vw, RefKeen_Patch_id_vw_ac, RefKeen_Patch_id_vw_ae,
	RefKeen_Patch_kd_demo, RefKeen_Patch_kd_keen, RefKeen_Patch_kd_play,
	RefKeen_FillObjStatesWithDOSPointers, RefKeen_PrepareAltControllerScheme, 0
};

void kdreams_exe_main(void);
int loadscn2_main(int argc, const char **argv);
void RefKeen_Load_Embedded_Resources_From_kdreams_exe(void);

/*** v1.00 Registered EGA ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamse100[] = {
	{"AUDIO.KDR", 3498, 0x80ac85e5},
	{"EGAGRAPH.KDR", 213045, 0x2dc94687},
	{"GAMEMAPS.KDR", 63497, 0x7b517fa0},
	{"KDREAMS.EXE", 77694, 0xc73b8cb2},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_kdreamse100[] = {
	{"AUDIODCT.KDR", 1024, 0x8b6116d7, 0x28984},
	{"AUDIOHHD.KDR", 340, 0x499e0cbf, 0x1f370},
	{"CONTEXT.KDR", 4759, 0x5bae2337, 0x1f4d0},
	{"EGADICT.KDR", 1024, 0xa69af202, 0x28188},
	{"EGAHEAD.KDR", 12068, 0xb9d789ee, 0x19610},
	{"GAMETEXT.KDR", 4686, 0x046c5328, 0x20770},
	{"MAPDICT.KDR", 1020, 0x8aa7334b, 0x28588},
	{"MAPHEAD.KDR", 11824, 0x4b9c9ebe, 0x1c540},
	{"PIRACY.BIN", 4001, 0x94458def, 0x14970}, // A bit different from PIRACY.SCN
	{"STORY.KDR", 2487, 0xed0ea5fe, 0x219c0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse100[] = {
	{
		g_be_embeddedgamefiles_kdreamse100,
		NULL,
		"KDREAMS.EXE",
		&kdreams_exe_main,
		&RefKeen_Load_Embedded_Resources_From_kdreams_exe,
		175424 - 0x1a00,
		BE_EXECOMPRESSION_PKLITE105,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreamse100 = {
	g_be_reqgameverfiles_kdreamse100,
	g_be_exefiles_kdreamse100,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMSE100),
	"Keen Dreams EGA v1.00",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_KDREAMSE100
};

/*** v1.00 CGA ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamsc100[] = {
	{"AUDIO.KDR", 3498, 0x80ac85e5},
	{"CGAGRAPH.KDR", 134691, 0x05e32626},
	{"GAMEMAPS.KDR", 63497, 0x7b517fa0},
	{"KDREAMS.EXE", 75015, 0xb6ff595a},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_kdreamsc100[] = {
	{"AUDIODCT.KDR", 1024, 0x8b6116d7, 0x281c2},
	{"AUDIOHHD.KDR", 340, 0x499e0cbf, 0x1e6b0},
	{"CGADICT.KDR", 1024, 0xaba89759, 0x279c6},
	{"CGAHEAD.KDR", 12068, 0x36d48226, 0x18950},
	{"CONTEXT.KDR", 4759, 0x5bae2337, 0x1e810},
	{"GAMETEXT.KDR", 4686, 0x046c5328, 0x1fab0},
	{"MAPDICT.KDR", 1020, 0x8aa7334b, 0x27dc6},
	{"MAPHEAD.KDR", 11824, 0x4b9c9ebe, 0x1b880},
	{"PIRACY.BIN", 4001, 0x94458def, 0x13cb0}, // A bit different from PIRACY.SCN
	{"STORY.KDR", 2487, 0xed0ea5fe, 0x20d00},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamsc100[] = {
	{
		g_be_embeddedgamefiles_kdreamsc100,
		NULL,
		"KDREAMS.EXE",
		&kdreams_exe_main,
		&RefKeen_Load_Embedded_Resources_From_kdreams_exe,
		172896 - 0x1800,
		BE_EXECOMPRESSION_PKLITE105,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreamsc100 = {
	g_be_reqgameverfiles_kdreamsc100,
	g_be_exefiles_kdreamsc100,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMSC100),
	"Keen Dreams CGA v1.00",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_KDREAMSC100
};

/*** v1.13 (Shareware) ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamse113[] = {
	{"KDREAMS.AUD", 3498, 0x80ac85e5},
	{"KDREAMS.CMP", 14189, 0x97628ca0},
	{"KDREAMS.EGA", 213045, 0x2dc94687},
	{"KDREAMS.EXE", 81619, 0x9dce0a39},
	{"KDREAMS.MAP", 65673, 0x8dce09af},
	{"LAST.SHL", 1634, 0xc0a3560f},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_kdreamse113[] = {
	{"AUDIODCT.KDR", 1024, 0x8b6116d7, 0x2a042},
	{"AUDIOHHD.KDR", 340, 0x499e0cbf, 0x22880},
	{"CONTEXT.KDR", 1283, 0x5a33439d, 0x229e0},
	{"EGADICT.KDR", 1024, 0xa69af202, 0x29846},
	{"EGAHEAD.KDR", 12068, 0xb9d789ee, 0x1cb20},
	{"GAMETEXT.KDR", 413, 0xb0df2792, 0x22ef0},
	{"MAPDICT.KDR", 1020, 0x9faa7213, 0x29c46},
	{"MAPHEAD.KDR", 11824, 0xb2f36c60, 0x1fa50},
	{"STORY.KDR", 2526, 0xcafc1d15, 0x23090},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse113[] = {
	{
		g_be_embeddedgamefiles_kdreamse113,
		NULL,
		"KDREAMS.EXE",
		&kdreams_exe_main,
		&RefKeen_Load_Embedded_Resources_From_kdreams_exe,
		213536 - 0x1c00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		NULL,
		NULL,
		"LOADSCN.EXE",
		(void (*)(void))&loadscn2_main,
		NULL,
		17712 - 0x200,
		BE_EXECOMPRESSION_LZEXE9X,
		true
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreamse113 = {
	g_be_reqgameverfiles_kdreamse113,
	g_be_exefiles_kdreamse113,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMSE113),
	"Keen Dreams EGA v1.13",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_KDREAMSE113
};

/*** v1.05 (CGA) ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamsc105[] = {
	{"AUDIO.KDR", 3498, 0x80ac85e5},
	{"CGAGRAPH.KDR", 134691, 0x05e32626},
	{"GAMEMAPS.KDR", 65736, 0x4b92df5f},
	{"KDREAMS.EXE", 78253, 0x5af7ce2b},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_kdreamsc105[] = {
	{"AUDIODCT.KDR", 1024, 0x8b6116d7, 0x28490},
	{"AUDIOHHD.KDR", 340, 0x499e0cbf, 0x1eb50},
	{"CGADICT.KDR", 1024, 0xaba89759, 0x27c94},
	{"CGAHEAD.KDR", 12068, 0x36d48226, 0x18df0},
	{"CONTEXT.KDR", 4759, 0x5bae2337, 0x1ecb0},
	{"GAMETEXT.KDR", 4686, 0x046c5328, 0x1ff50},
	{"MAPDICT.KDR", 1020, 0xfa8362f3, 0x28094},
	{"MAPHEAD.KDR", 11824, 0x66c122b4, 0x1bd20},
	{"STORY.KDR", 2487, 0xed0ea5fe, 0x211a0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamsc105[] = {
	{
		g_be_embeddedgamefiles_kdreamsc105,
		NULL,
		"KDREAMS.EXE",
		&kdreams_exe_main,
		&RefKeen_Load_Embedded_Resources_From_kdreams_exe,
		202320 - 0x1800,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreamsc105 = {
	g_be_reqgameverfiles_kdreamsc105,
	g_be_exefiles_kdreamsc105,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMSC105),
	"Keen Dreams CGA v1.05",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_KDREAMSC105
};

// Keen Dreams v1.93 and v1.20 actually share the exact same game data,
// minus the EXE (and v1.92 is also the same, except for KDREAMS.CMP).

/*** v1.93 (Registered EGA) ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamse193[] = {
	{"KDREAMS.AUD", 3498, 0x80ac85e5},
	{"KDREAMS.CMP", 14620, 0x1681bc21},
	{"KDREAMS.EGA", 213045, 0x2dc94687},
	{"KDREAMS.EXE", 80715, 0x3d0cd9ed},
	{"KDREAMS.MAP", 65674, 0x78bd24fc},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_kdreamse193[] = {
	{"AUDIODCT.KDR", 1024, 0x8b6116d7, 0x29fba},
	{"AUDIOHHD.KDR", 340, 0x499e0cbf, 0x21990},
	{"CONTEXT.KDR", 1283, 0x5a33439d, 0x21af0},
	{"EGADICT.KDR", 1024, 0xa69af202, 0x297be},
	{"EGAHEAD.KDR", 12068, 0xb9d789ee, 0x1bc30},
	{"GAMETEXT.KDR", 4256, 0xbfe72f94, 0x22000},
	{"MAPDICT.KDR", 1020, 0x6bb0de32, 0x29bbe},
	{"MAPHEAD.KDR", 11824, 0x2b821e29, 0x1eb60},
	{"STORY.KDR", 2526, 0xcafc1d15, 0x230a0},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse193[] = {
	{
		g_be_embeddedgamefiles_kdreamse193,
		NULL,
		"KDREAMS.EXE",
		&kdreams_exe_main,
		&RefKeen_Load_Embedded_Resources_From_kdreams_exe,
		213200 - 0x1c00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreamse193 = {
	g_be_reqgameverfiles_kdreamse193,
	g_be_exefiles_kdreamse193,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMSE193),
	"Keen Dreams EGA v1.93",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_KDREAMSE193
};

/*** v1.20 (Shareware) ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamse120[] = {
	{"KDREAMS.AUD", 3498, 0x80ac85e5},
	{"KDREAMS.CMP", 14620, 0x1681bc21},
	{"KDREAMS.EGA", 213045, 0x2dc94687},
	{"KDREAMS.EXE", 81519, 0xf36f0edf},
	{"KDREAMS.MAP", 65674, 0x78bd24fc},
	{0}
};

static const BE_EmbeddedGameFileDetails_T g_be_embeddedgamefiles_kdreamse120[] = {
	{"AUDIODCT.KDR", 1024, 0x8b6116d7, 0x2a66c},
	{"AUDIOHHD.KDR", 340, 0x499e0cbf, 0x21d70},
	{"CONTEXT.KDR", 1283, 0x5a33439d, 0x21ed0},
	{"EGADICT.KDR", 1024, 0xa69af202, 0x29e70},
	{"EGAHEAD.KDR", 12068, 0xb9d789ee, 0x1c010},
	{"GAMETEXT.KDR", 4256, 0xbfe72f94, 0x223e0},
	{"MAPDICT.KDR", 1020, 0x6bb0de32, 0x2a270},
	{"MAPHEAD.KDR", 11824, 0x2b821e29, 0x1ef40},
	{"STORY.KDR", 2526, 0xcafc1d15, 0x23480},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse120[] = {
	{
		g_be_embeddedgamefiles_kdreamse120,
		NULL,
		"KDREAMS.EXE",
		&kdreams_exe_main,
		&RefKeen_Load_Embedded_Resources_From_kdreams_exe,
		214912 - 0x1c00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreamse120 = {
	g_be_reqgameverfiles_kdreamse120,
	g_be_exefiles_kdreamse120,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMSE120),
	"Keen Dreams EGA v1.20",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_PCSPKR_REQUIRED | BE_AUDIO_DEVICE_OPL,
	BE_GAMEVER_KDREAMSE120
};

/*** 2015 edition - All files are external here ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreams2015[] = {
	{"CONTEXT.KDR", 1331, 0x70920ec3},
	{"EGADICT.CGA", 1024, 0x75a458f2},
	{"EGADICT.KDR", 1024, 0xfe21f5c4},
	{"EGAHEAD.CGA", 12068, 0x38c15a01},
	{"EGAHEAD.KDR", 12068, 0xdeb318b3},
	{"GAMETEXT.KDR", 4314, 0x0996e7f2},
	{"KDREAMS.CGA", 179725, 0x83b0c6c4},
	{"KDREAMS.CMP", 40878, 0xf8a33e2f},
	{"KDREAMS.EGA", 213221, 0xee1024b9},
	{"KDREAMS.MAP", 65674, 0x934cd898},
	{"KDREAMS.SND", 1449826, 0xfb74b9a2},
	{"MAPDICT.KDR", 1020, 0x6bb0de32},
	{"MAPHEAD.KDR", 11824, 0xff335e8c},
	{"SOUNDDCT.KDR", 1024, 0x2217ceb9},
	{"SOUNDHHD.KDR", 228, 0xb8b39c5c},
	{"STORY.KDR", 2526, 0xcafc1d15},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreams2015[] = {
	{
		NULL,
		NULL,
		NULL, // No EXE file
		&kdreams_exe_main,
		NULL, // No EXE file
		0, // No EXE file
		BE_EXECOMPRESSION_NONE, // No EXE file
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_kdreams2015 = {
	g_be_reqgameverfiles_kdreams2015,
	g_be_exefiles_kdreams2015,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_KDREAMS2015),
	"Keen Dreams 2015",
	g_be_patcherfuncs_kdreams,
	BE_AUDIO_DEVICE_DIGI_REQUIRED,
	BE_GAMEVER_KDREAMS2015
};
