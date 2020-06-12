/* This header must be included *only* from be_gamedef.h */

/*** v1.00 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_cat3d100[] = {
	{"AUDIO.C3D", 5062, 0x074f7525},
	{"EGAGRAPH.C3D", 258007, 0x7b756d01},
	{"GAMEMAPS.C3D", 14288, 0x1d8d501e},
	{"CAT3D.EXE", 78483, 0xe20992c7},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_cat3d100[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"AUDIODCT.C3D", 1024, 0xd3dbe849, 0x22c64},
			{"AUDIOHHD.C3D", 368, 0xb83933bc, 0x1aa60},
			{"EGADICT.C3D", 1024, 0xab94fb6c, 0x23064},
			{"EGAHEAD.C3D", 1437, 0x33772bb0, 0x1abd0},
			{"INTROSCN.SCN", 4008, 0xec236c5c, 0x192c0},
			{"MTEMP.TMP", 618, 0x6b7cc556, 0x1b170},
			{0}
		},

		NULL,
		"CAT3D.EXE",
		&REFKEEN_NS_ENCLOSE(cat3d, cat3d_exe_main),
		&REFKEEN_NS_ENCLOSE(cat3d, RefKeen_Load_Embedded_Resources_From_catacombs_exe),
		191536 - 0x1400,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_cat3d100 = {
	g_be_reqgameverfiles_cat3d100,
	g_be_exefiles_cat3d100,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_CAT3D100),
	"Catacomb 3-D v1.00 (Custom)",
	0,
	BE_GAMEVER_CAT3D100
};

/*** v1.22 ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_cat3d122[] = {
	{"AUDIO.C3D", 5062, 0x074f7525},
	{"EGAGRAPH.C3D", 256899, 0xbc122f40},
	{"GAMEMAPS.C3D", 14288, 0x1ecb107b},
	{"CAT3D.EXE", 78285, 0xfaa27410},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_cat3d122[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"AUDIODCT.C3D", 1024, 0xd3dbe849, 0x22bd8},
			{"AUDIOHHD.C3D", 368, 0xb83933bc, 0x1a710},
			{"EGADICT.C3D", 1024, 0xb26a70a6, 0x22fd8},
			{"EGAHEAD.C3D", 1437, 0x3fde00c4, 0x1a880},
			// INTROSCN.SCN isn't displayed in vanilla v1.22, but it's still
			// allocated and in use, so it's safer to require this chunk
			{"INTROSCN.SCN", 4008, 0xcf9696af, 0x18f70},
			{"MTEMP.TMP", 618, 0x6b7cc556, 0x1ae20},
			{0}
		},

		NULL,
		"CAT3D.EXE",
		&REFKEEN_NS_ENCLOSE(cat3d, cat3d_exe_main),
		&REFKEEN_NS_ENCLOSE(cat3d, RefKeen_Load_Embedded_Resources_From_catacombs_exe),
		191904 - 0x1600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_cat3d122 = {
	g_be_reqgameverfiles_cat3d122,
	g_be_exefiles_cat3d122,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_CAT3D122),
	"Catacomb 3-D v1.22 (Custom)",
	0,
	BE_GAMEVER_CAT3D122
};
