/* This header must be included *only* from be_cross_fs.c */

#include "be_gamestr.h"

// Main functions prototypes
void kdreams_exe_main(void);
int loadscn2_main(int argc, const char **argv);
void cat3d_exe_main(void);
void abysgame_exe_main(void);
void armgame_exe_main(void);
void apocgame_exe_main(void);
void intro_exe_main(void);
void slidecat_exe_main(void);
void loadscn_exe_main(void);
// Embedded resources loader functions prototypes
void RefKeen_Load_Embedded_Resources_From_kdreams_exe(void);
void RefKeen_Load_Embedded_Resources_From_catacombs_exe(void);
void RefKeen_Load_Embedded_Resources_From_slidecat_exe(void);

// These MUST have the same order as in the BE_GameVer_T enum
const char *refkeen_gamever_strs[BE_GAMEVER_LAST] = {
#ifdef REFKEEN_VER_KDREAMS
	BE_STR_GAMEVER_KDREAMSE100,
	BE_STR_GAMEVER_KDREAMSC100,
	BE_STR_GAMEVER_KDREAMSE113,
	BE_STR_GAMEVER_KDREAMSC105,
	BE_STR_GAMEVER_KDREAMSE193,
	BE_STR_GAMEVER_KDREAMSE120,
	BE_STR_GAMEVER_KDREAMS2015,
#endif
#ifdef REFKEEN_VER_CAT3D
	BE_STR_GAMEVER_CAT3D100,
	BE_STR_GAMEVER_CAT3D122,
#endif
#ifdef REFKEEN_VER_CATABYSS
	BE_STR_GAMEVER_CATABYSS113,
	BE_STR_GAMEVER_CATABYSS124,
#endif
#ifdef REFKEEN_VER_CATARM
	BE_STR_GAMEVER_CATARM102,
#endif
#ifdef REFKEEN_VER_CATAPOC
	BE_STR_GAMEVER_CATAPOC101,
#endif
};

const char *refkeen_gamever_descriptions[BE_GAMEVER_LAST] = {
#ifdef REFKEEN_VER_KDREAMS
	"Keen Dreams EGA v1.00",
	"Keen Dreams CGA v1.00",
	"Keen Dreams EGA v1.13",
	"Keen Dreams CGA v1.05",
	"Keen Dreams EGA v1.93",
	"Keen Dreams EGA v1.20",
	"Keen Dreams 2015",
#endif
#ifdef REFKEEN_VER_CAT3D
	"Catacomb 3-D v1.00",
	"Catacomb 3-D v1.22",
#endif
#ifdef REFKEEN_VER_CATABYSS
	"Catacomb Abyss v1.13",
	"Catacomb Abyss v1.24",
#endif
#ifdef REFKEEN_VER_CATARM
	"Catacomb Armageddon v1.02",
#endif
#ifdef REFKEEN_VER_CATAPOC
	"Catacomb Apocalypse v1.01",
#endif
};

#ifdef REFKEEN_VER_KDREAMS
/*** v1.00 Registered EGA ***/
static const BE_GameFileDetails_T g_be_reqgameverfiles_kdreamse100[] = {
	{"AUDIO.KDR", 3498, 0x80ac85e5},
	{"EGAGRAPH.KDR", 213045, 0x2dc94687},
	{"GAMEMAPS.KDR", 63497, 0x7b517fa0},
	{"KDREAMS.EXE", 77694, 0xc73b8cb2},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse100[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
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
		},

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
	"Keen Dreams EGA v1.00 (Custom)",
	0,
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

static const BE_EXEFileDetails_T g_be_exefiles_kdreamsc100[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
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
		},

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
	"Keen Dreams CGA v1.00 (Custom)",
	0,
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

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse113[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
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
		},

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
	"Keen Dreams EGA v1.13 (Custom)",
	0,
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

static const BE_EXEFileDetails_T g_be_exefiles_kdreamsc105[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
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
		},

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
	"Keen Dreams CGA v1.05 (Custom)",
	0,
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

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse193[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
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
		},

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
	"Keen Dreams EGA v1.93 (Custom)",
	0,
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

static const BE_EXEFileDetails_T g_be_exefiles_kdreamse120[] = {
	{
		(const BE_EmbeddedGameFileDetails_T []) {
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
		},

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
	"Keen Dreams EGA v1.20 (Custom)",
	0,
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
	"Keen Dreams 2015 (Custom)",
	44100, // Digitized sounds have such sample rate (in Hz)
	BE_GAMEVER_KDREAMS2015
};
#endif

#ifdef REFKEEN_VER_CAT3D
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
		&cat3d_exe_main,
		&RefKeen_Load_Embedded_Resources_From_catacombs_exe,
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
		&cat3d_exe_main,
		&RefKeen_Load_Embedded_Resources_From_catacombs_exe,
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
#endif

#ifdef REFKEEN_VER_CATABYSS
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
		(void (*)(void))&intro_exe_main,
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
		&abysgame_exe_main,
		&RefKeen_Load_Embedded_Resources_From_catacombs_exe,
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
		&slidecat_exe_main,
		&RefKeen_Load_Embedded_Resources_From_slidecat_exe,
		62800 - 0x800,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		NULL,

		NULL,
		"LOADSCN.EXE",
		&loadscn_exe_main,
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
		(void (*)(void))&intro_exe_main,
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
		&abysgame_exe_main,
		&RefKeen_Load_Embedded_Resources_From_catacombs_exe,
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
		&slidecat_exe_main,
		&RefKeen_Load_Embedded_Resources_From_slidecat_exe,
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
	0,
	BE_GAMEVER_CATABYSS124
};
#endif

#ifdef REFKEEN_VER_CATARM
static const BE_GameFileDetails_T g_be_reqgameverfiles_catarm102[] = {
	{"ARMGAME.EXE", 82198, 0x7bf5e3d5},
	// Looks like ARM_SLIB.ARM is mentioned in DEF.H but unused
	{"AUDIO.ARM", 7104, 0xb6c28bc7},
	{"EGAGRAPH.ARM", 355832, 0x8eea3210},
	{"GAMEMAPS.ARM", 17653, 0x787794bf},
	{"SHP10.ARM", 9930, 0x6a22743f},
	{"SHP11.ARM", 398, 0x99404417},
	{"SHP12.ARM", 417, 0x201133a8},
	{"SHP13.ARM", 329, 0xaa51f92a},
	{"SHP14.ARM", 1688, 0x98e0af99},
	{"SHP1.ARM", 1376, 0xe04a6d1e},
	{"SHP2.ARM", 3991, 0x1b4bb85b},
	{"SHP3.ARM", 15036, 0xe7c6a5a3},
	{"SHP4.ARM", 15568, 0x9fa3faee},
	{"SHP5.ARM", 13234, 0x3f90b11d},
	{"SHP6.ARM", 10861, 0x42faf6e5},
	{"SHP7.ARM", 9414, 0x1a969278},
	{"SHP8.ARM", 749, 0x086438f0},
	{"SHP9.ARM", 9182, 0x86ce4004},
	// HINTCAT stuff
	{"HINT01.HNT", 9664, 0xb6ecee18},
	{"HINT02.HNT", 9833, 0xa01e9cba},
	{"HINT03.HNT", 12060, 0xa8857173},
	{"HINT04.HNT", 10438, 0x60866c28},
	{"HINT05.HNT", 11555, 0x0973da05},
	{"HINT06.HNT", 10339, 0x1e5089d5},
	{"HINT07.HNT", 12888, 0x0bc7121d},
	{"HINT08.HNT", 10400, 0x0b33d707},
	{"HINT09.HNT", 10754, 0x4f787690},
	{"HINT10.HNT", 10262, 0xbddb06e5},
	{"HINT11.HNT", 12006, 0x47104911},
	{"HINT12.HNT", 12757, 0x340d8383},
	{"HINT13.HNT", 11662, 0xe2e1890f},
	{"HINT14.HNT", 9271, 0x74bbe8c7},
	{"HINT15.HNT", 11624, 0x8e842f18},
	{"HINT16.HNT", 12067, 0xffccf406},
	{"HINT17.HNT", 12342, 0xf24371ec},
	{"HINT18.HNT", 8896, 0x18567f5d},
	{"HINT19.HNT", 9308, 0xdb5b1b65},
	{"HINT20.HNT", 7301, 0xb2a16e89},
	{"HINTCAT.EXE", 15973, 0x79cc6ee3},
	{"SCRIPT.HNT", 90, 0x79a7f746},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_catarm102[] = {
	{
		NULL,

		NULL,
		"CATARM.EXE",
		(void (*)(void))&intro_exe_main,
		NULL,
		36448 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"AUDIODCT.ARM", 1024, 0x8f1d4dd2, 0x240b0},
			{"AUDIOHHD.ARM", 428, 0x5f863ad2, 0x1bb20},
			{"EGADICT.ARM", 1024, 0xab662db8, 0x244b0},
			{"EGAHEAD.ARM", 1977, 0x711cbf10, 0x1bcd0},
			{"MTEMP.TMP", 834, 0x546f00d1, 0x1b7d0},
			{0}
		},

		NULL,
		"ARMGAME.EXE",
		&armgame_exe_main,
		&RefKeen_Load_Embedded_Resources_From_catacombs_exe,
		198304 - 0x2000,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"TEXTSCN.SCN", 4000, 0xf7773f42, 0x6dd0},
			{0}
		},

		"Catacomb Armageddon 3-D Hint Book v1.12",
		"HINTCAT.EXE",
		&slidecat_exe_main,
		&RefKeen_Load_Embedded_Resources_From_slidecat_exe,
		39296 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_catarm102 = {
	g_be_reqgameverfiles_catarm102,
	g_be_exefiles_catarm102,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_CATARM102),
	"Catacomb Armageddon v1.02 (Custom)",
	0,
	BE_GAMEVER_CATARM102
};
#endif

#ifdef REFKEEN_VER_CATAPOC
static const BE_GameFileDetails_T g_be_reqgameverfiles_catapoc101[] = {
	{"APOCGAME.EXE", 82386, 0x48ca6808},
	// No APC_SLIB.APC file was found, although it is mentioned in DEF.H
	{"AUDIO.APC", 7366, 0x23934cec},
	{"EGAGRAPH.APC", 382078, 0x291893c3},
	{"GAMEMAPS.APC", 18855, 0xd317b2c9},
	{"SHP10.APC", 9930, 0x6a22743f},
	{"SHP11.APC", 398, 0x99404417},
	{"SHP12.APC", 417, 0x201133a8},
	{"SHP13.APC", 329, 0xaa51f92a},
	{"SHP14.APC", 1668, 0xa4a8e44e},
	{"SHP1.APC", 1376, 0xe04a6d1e},
	{"SHP2.APC", 6329, 0xff48bc57},
	{"SHP3.APC", 13187, 0x5800b7f5},
	{"SHP4.APC", 13592, 0x1c498a90},
	{"SHP5.APC", 12698, 0xf2fa1802},
	{"SHP6.APC", 11131, 0x110da23b},
	{"SHP7.APC", 13266, 0x9ec74514},
	{"SHP8.APC", 640, 0x864e6d0c},
	{"SHP9.APC", 9182, 0x86ce4004},
	// HINTCAT stuff
	{"HINT01.HNT", 9664, 0x1641c8f0},
	{"HINT02.HNT", 9823, 0x0d319717},
	{"HINT03.HNT", 10822, 0x994d94cd},
	{"HINT04.HNT", 11654, 0xe754554f},
	{"HINT05.HNT", 8021, 0xfd819cab},
	{"HINT06.HNT", 9839, 0x13f601ce},
	{"HINT07.HNT", 10314, 0xa049c277},
	{"HINT08.HNT", 9616, 0xa5990371},
	{"HINT09.HNT", 9074, 0x61e1681e},
	{"HINT10.HNT", 9744, 0x38ae11c9},
	{"HINT11.HNT", 14563, 0x7c233efa},
	{"HINT12.HNT", 9261, 0x714bd15a},
	{"HINT13.HNT", 10056, 0xebf72992},
	{"HINT14.HNT", 6552, 0x5039141c},
	{"HINT15.HNT", 10738, 0x60fd9f81},
	{"HINT16.HNT", 8302, 0xcace81b8},
	{"HINT17.HNT", 8348, 0x00783edb},
	{"HINT18.HNT", 9990, 0x04e56bbe},
	{"HINT19.HNT", 7301, 0xb2a16e89},
	{"HINTCAT.EXE", 16215, 0xfefa177c},
	{"SCRIPT.HNT", 88, 0x8dd529b2},
	{0}
};

static const BE_EXEFileDetails_T g_be_exefiles_catapoc101[] = {
	{
		NULL,

		NULL,
		"CATAPOC.EXE",
		(void (*)(void))&intro_exe_main,
		NULL,
		40208 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"AUDIODCT.APC", 1024, 0x26658498, 0x2439c},
			{"AUDIOHHD.APC", 452, 0x76adb051, 0x1bd80},
			{"EGADICT.APC", 1024, 0xb2ed57fd, 0x2479c},
			{"EGAHEAD.APC", 2049, 0xd7548ed8, 0x1bf50},
			{"MTEMP.TMP", 834, 0x90742162, 0x1ba30},
			{0}
		},

		NULL,
		"APOCGAME.EXE",
		&apocgame_exe_main,
		&RefKeen_Load_Embedded_Resources_From_catacombs_exe,
		200064 - 0x2200,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{
		(const BE_EmbeddedGameFileDetails_T []) {
			{"TEXTSCN.SCN", 4000, 0xf7773f42, 0x6ef0},
			{0}
		},


		"Catacomb Apocalypse 3-D Hint Book v1.13",
		"HINTCAT.EXE",
		&slidecat_exe_main,
		&RefKeen_Load_Embedded_Resources_From_slidecat_exe,
		39568 - 0x600,
		BE_EXECOMPRESSION_LZEXE9X,
		false
	},
	{0}
};

static const BE_GameVerDetails_T g_be_gamever_catapoc101 = {
	g_be_reqgameverfiles_catapoc101,
	g_be_exefiles_catapoc101,
	CSTR_TO_TCSTR(BE_STR_GAMEVER_CATAPOC101),
	"Catacomb Apocalypse v1.01 (Custom)",
	0,
	BE_GAMEVER_CATAPOC101
};
#endif

static const BE_GameVerDetails_T *g_be_gamever_ptrs[] = {
#ifdef REFKEEN_VER_KDREAMS
	&g_be_gamever_kdreamse100,
	&g_be_gamever_kdreamsc100,
	&g_be_gamever_kdreamse113,
	&g_be_gamever_kdreamsc105,
	&g_be_gamever_kdreamse193,
	&g_be_gamever_kdreamse120,
	&g_be_gamever_kdreams2015,
#endif
#ifdef REFKEEN_VER_CAT3D
	&g_be_gamever_cat3d100,
	&g_be_gamever_cat3d122,
#endif
#ifdef REFKEEN_VER_CATABYSS
	&g_be_gamever_catabyss113,
	&g_be_gamever_catabyss124,
#endif
#ifdef REFKEEN_VER_CATARM
	&g_be_gamever_catarm102,
#endif
#ifdef REFKEEN_VER_CATAPOC
	&g_be_gamever_catapoc101,
#endif
};

const BE_GameFileDetails_T *g_be_gamefiledetails_ptrs[]  = {
#ifdef REFKEEN_VER_KDREAMS
	g_be_reqgameverfiles_kdreamse100,
	g_be_reqgameverfiles_kdreamsc100,
	g_be_reqgameverfiles_kdreamse113,
	g_be_reqgameverfiles_kdreamsc105,
	g_be_reqgameverfiles_kdreamse193,
	g_be_reqgameverfiles_kdreamse120,
	g_be_reqgameverfiles_kdreams2015,
#endif
#ifdef REFKEEN_VER_CAT3D
	g_be_reqgameverfiles_cat3d100,
	g_be_reqgameverfiles_cat3d122,
#endif
#ifdef REFKEEN_VER_CATABYSS
	g_be_reqgameverfiles_catabyss113,
	g_be_reqgameverfiles_catabyss124,
#endif
#ifdef REFKEEN_VER_CATARM
	g_be_reqgameverfiles_catarm102,
#endif
#ifdef REFKEEN_VER_CATAPOC
	g_be_reqgameverfiles_catapoc101,
#endif
};
