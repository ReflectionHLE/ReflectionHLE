/* This header must be included *only* from be_gamedef_catadventures.h */

REFKEEN_NS_B_FOR(catabyss)
void RefKeen_Patch_id_us(void);
void RefKeen_Patch_c4_main(void);
void RefKeen_Patch_c4_play(void);
void RefKeen_Patch_gelib(void);
void RefKeen_Patch_intro(void);
void RefKeen_Patch_slidecat(void);
void RefKeen_FillObjStatesWithDOSPointers(void);
void RefKeen_PrepareAltControllerScheme(void);

static void (*g_be_patcherfuncs_catabyss[])(void) = {
	RefKeen_Patch_id_us, RefKeen_Patch_c4_main, RefKeen_Patch_c4_play,
	RefKeen_Patch_gelib, RefKeen_Patch_intro, RefKeen_Patch_slidecat,
	RefKeen_FillObjStatesWithDOSPointers, RefKeen_PrepareAltControllerScheme, 0
};
REFKEEN_NS_E

/*** v1.13 (Shareware) ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_catabyss113[] = {
	{"ARMAPC.ABS", 10489, 0x9562020e},
	{"AUDIO.ABS", 6904, 0x462d2eb2},
	{"CATABYSS.EXE", 82662, 0xbc3b015e},
	{"EGAGRAPH.ABS", 336143, 0x3b2464a2},
	{"GAMEMAPS.ABS", 20433, 0x97ddb354},
	{"HELP.TXT", 15491, 0x7f8c4a59},
	{"LAST.ABS", 1290, 0xc36ba646},
	{"SHP01.ABS", 1376, 0xe04a6d1e},
	{"SHP02.ABS", 15181, 0x0198b8f7},
	{"SHP03.ABS", 13158, 0x0d6b4986},
	{"SHP04.ABS", 12648, 0x3bd5ccbc},
	{"SHP05.ABS", 724, 0x857573eb},
	{"SHP06.ABS", 9182, 0x86ce4004},
	{"SHP07.ABS", 9930, 0x6a22743f},
	{"SHP08.ABS", 398, 0x99404417},
	{"SHP09.ABS", 417, 0x201133a8},
	{"SHP10.ABS", 329, 0xaa51f92a},
	{"SHP11.ABS", 10595, 0x5a95691e},
	{"SHP12.ABS", 9182, 0x378b5984},
	// Looks like DEMOCAT stuff; Let's require these.
	{"ALTAR.CAT", 15014, 0x833d1ca7},
	{"APC.CAT", 12828, 0x009ec08a},
	{"CEMETRY.CAT", 14983, 0x8cfb3741},
	{"COOKIE.CAT", 16837, 0xee83c4f4},
	{"CRYSTAL.CAT", 15320, 0x16fa18f0},
	{"DEMOCAT.EXE", 32066, 0xce326ea2},
	{"DRAGONS.CAT", 7107, 0x54ea7eb5},
	{"EGYPT.CAT", 8863, 0xc78451d7},
	{"FLAME.CAT", 7062, 0xe422aa64},
	{"FOLLOW.CAT", 10437, 0xbec4bd75},
	{"GRATE.CAT", 8306, 0x8f7cb569},
	{"GROUPIC.CAT", 20563, 0x93531dd8},
	{"HOMEBBS.CAT", 3233, 0xaf3d0e16},
	{"INFERNO.CAT", 13934, 0x3ddd36e9},
	{"INVITED.CAT", 9723, 0xacc2ce73},
	{"LAST.CAT", 1247, 0x73f1ec9e},
	{"LEGEND.CAT", 10977, 0xc99a5540},
	{"LOUNGE.CAT", 10665, 0xb799f029},
	{"OPEN.CAT", 4445, 0xe90e743b},
	{"ORCMINE.CAT", 8903, 0xf8821eda},
	{"QUEST.CAT", 13076, 0x252de058},
	{"REDHALL.CAT", 8214, 0x0d9bd58e},
	{"SCRIPT.CAT", 174, 0x45973aa6},
	{"TRILOGY.CAT", 11397, 0xe09a1995},
	{"VAULT.CAT", 8737, 0x43d6dba7},
	{"VICTORY.CAT", 7194, 0x70846fc6},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_catabyss113[] = {
	// Intro EXE is the first one we begin from
	{
		NULL,

		NULL,
		"INTRO.EXE",
		(void (*)(void))&REFKEEN_NS_ENCLOSE(catabyss, intro_exe_main),
		NULL,
		36560 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"AUDIODCT.ABS", 1024, 0xe9088011, 0x2554c},
			{"AUDIOHHD.ABS", 416, 0xfbfff495, 0x1a210},
			{"EGADICT.ABS", 1024, 0xbb760f1d, 0x2594c},
			{"EGAHEAD.ABS", 1881, 0xe31e1c3b, 0x1a3b0},
			{"MTEMP.TMP", 834, 0x5d9ccfb3, 0x1ab10},
			{0}
		},

		NULL,
		"CATABYSS.EXE",
		&REFKEEN_NS_ENCLOSE(catabyss, abysgame_exe_main),
		&REFKEEN_NS_ENCLOSE(catabyss, RefKeen_Load_Embedded_Resources_From_catacombs_exe),
		201120 - 0x1a00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"TEXTSCN.SCN", 4000, 0xf7773f42, 0xbf70},
			{0}
		},

		"Electronic Catalog v1.00",
		"DEMOCAT.EXE",
		&REFKEEN_NS_ENCLOSE(catabyss, slidecat_exe_main),
		&REFKEEN_NS_ENCLOSE(catabyss, RefKeen_Load_Embedded_Resources_From_slidecat_exe),
		62800 - 0x800,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		NULL,

		NULL,
		"LOADSCN.EXE",
		&REFKEEN_NS_ENCLOSE(catabyss, loadscn_exe_main),
		NULL,
		28992 - 0x400,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_catabyss113 = {
	g_be_reqgameverfiles_catabyss113,
	g_be_exefiles_catabyss113,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_CATABYSS113),
	"Catacomb Abyss v1.13 (Custom)",
	REFKEEN_NS_ENCLOSE(catabyss, g_be_patcherfuncs_catabyss),
	0,
	BE_GAMEVER_CATABYSS113
};

/*** v1.24 (Registered) ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_catabyss124[] = {
	{"ABYSGAME.EXE", 82479, 0x5f0319db},
	{"AUDIO.ABS", 6904, 0x462d2eb2},
	{"EGAGRAPH.ABS", 335994, 0xfd9995ad},
	{"GAMEMAPS.ABS", 20433, 0x97ddb354},
	{"HELP.TXT", 15042, 0xc47ce06e},
	{"SHP01.ABS", 1376, 0xe04a6d1e},
	{"SHP02.ABS", 14912, 0xffc72620},
	{"SHP03.ABS", 13186, 0x7b73bb7b},
	{"SHP04.ABS", 12648, 0x3bd5ccbc},
	{"SHP05.ABS", 724, 0x857573eb},
	{"SHP06.ABS", 9182, 0x86ce4004},
	{"SHP07.ABS", 9930, 0x6a22743f},
	{"SHP08.ABS", 398, 0x99404417},
	{"SHP09.ABS", 417, 0x201133a8},
	{"SHP10.ABS", 329, 0xaa51f92a},
	{"SHP11.ABS", 10328, 0x08ee65f4},
	{"SHP12.ABS", 1652, 0xbeb87fbd},
	// HINTCAT stuff similar to DEMOCAT; Require these as well.
	{"AQUDUCT.HNT", 7886, 0x66a11ac1},
	{"CEMETRY.HNT", 10376, 0xf7b3f888},
	{"COVEN.HNT", 13550, 0xc3fdd8a4},
	{"CRYPT.HNT", 11233, 0x46ca0a4b},
	{"DEN.HNT", 10520, 0xbea64562},
	{"GARDEN.HNT", 11125, 0x8c2385b6},
	{"HAUNT.HNT", 7743, 0x2b9ef944},
	{"HINTCAT.EXE", 16328, 0x2bd569ac},
	{"HINTGO1.HNT", 9751, 0x4fa00485},
	{"HINTGO2.HNT", 9902, 0x6640c7b6},
	{"HINTGO3.HNT", 8372, 0x8e743b2c},
	{"INFERNO.HNT", 11658, 0x9d59623a},
	{"MAUSGND.HNT", 11305, 0x70655f30},
	{"MAUSOL.HNT", 12075, 0xee9b8745},
	{"ORCMINE.HNT", 12664, 0x35502713},
	{"PASSAGE.HNT", 10159, 0x126e01a5},
	{"SANCTUM.HNT", 12030, 0x16c785e6},
	{"SCRIPT.HNT", 155, 0x985da39a},
	{"SUBVALT.HNT", 12110, 0xfb4d589d},
	{"TITANS.HNT", 9821, 0xd491ac71},
	{"TROLLS.HNT", 13409, 0x68a6a6e9},
	{"VICTORY.HNT", 7301, 0xb2a16e89},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_catabyss124[] = {
	// Again, intro EXE is the first one we begin from
	{
		NULL,

		NULL,
		"CATABYSS.EXE",
		(void (*)(void))&REFKEEN_NS_ENCLOSE(catabyss, intro_exe_main),
		NULL,
		36064 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"AUDIODCT.ABS", 1024, 0xe9088011, 0x2543a},
			{"AUDIOHHD.ABS", 416, 0xfbfff495, 0x1a140},
			{"EGADICT.ABS", 1024, 0x63eb06d3, 0x2583a},
			{"EGAHEAD.ABS", 1881, 0x94967205, 0x1a2e0},
			{"MTEMP.TMP", 834, 0x5d9ccfb3, 0x1aa40},
			{0}
		},

		NULL,
		"ABYSGAME.EXE",
		&REFKEEN_NS_ENCLOSE(catabyss, abysgame_exe_main),
		&REFKEEN_NS_ENCLOSE(catabyss, RefKeen_Load_Embedded_Resources_From_catacombs_exe),
		200848 - 0x1a00,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"TEXTSCN.SCN", 4000, 0xf7773f42, 0x6f60},
			{0}
		},

		"Catacomb Abyss 3-D Hint Book v1.01",
		"HINTCAT.EXE",
		&REFKEEN_NS_ENCLOSE(catabyss, slidecat_exe_main),
		&REFKEEN_NS_ENCLOSE(catabyss, RefKeen_Load_Embedded_Resources_From_slidecat_exe),
		39968 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_catabyss124 = {
	g_be_reqgameverfiles_catabyss124,
	g_be_exefiles_catabyss124,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_CATABYSS124),
	"Catacomb Abyss v1.24 (Custom)",
	REFKEEN_NS_ENCLOSE(catabyss, g_be_patcherfuncs_catabyss),
	0,
	BE_GAMEVER_CATABYSS124
};
