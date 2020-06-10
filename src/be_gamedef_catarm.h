/* This header must be included *only* from be_gamedef.h */

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
