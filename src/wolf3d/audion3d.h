/////////////////////////////////////////////////
//
// MUSE Header for .N3D
// Initially recreated using ECWolf definitions,
// and then filled based on debugging symbols
// from the original EXE.
//
/////////////////////////////////////////////////

REFKEEN_NS_B

#define NUMSOUNDS		44
#define NUMSNDCHUNKS		143

//
// Sound names & indexes
//
typedef enum {
		D_ANTLPESND,             // 0
		D_BEARSND,               // 1
		D_BONUSSND,              // 2
		D_CAMELSND,              // 3
		D_CLOSDRSND,             // 4
		D_COCHITSND,             // 5
		D_COCTHRSND,             // 6
		D_DBITESND,              // 7
		D_EATSND,                // 8
		D_EDIESND,               // 9
		D_ELPHNTSND,             // 10
		D_EXTRASND,              // 11
		D_GATLINSND,             // 12
		D_GIRAFESND,             // 13
		D_GOATSND,               // 14
		D_GUNSHTSND,             // 15
		D_INCSND,                // 16
		D_KANGROSND,             // 17
		D_KEYSND,                // 18
		D_KNIFESND,              // 19
		D_MONKEYSND,             // 20
		D_MSTEPSND,              // 21
		D_OPENDRSND,             // 22
		D_OSTRCHSND,             // 23
		D_OXSND,                 // 24
		D_PAINSND,               // 25
		D_PDIESND,               // 26
		NOBONUSSND,              // 27
		D_PWALLSND,              // 28
		D_SHEEPSND,              // 29
		D_SLING2SND,             // 30
		D_SPITSND,               // 31
		D_WATHITSND,             // 32
		D_WATTHRSND,             // 33
		NOWAYSND,                // 34
		ESCPRESSEDSND,           // 35
		MOVEGUN1SND,             // 36
		MOVEGUN2SND,             // 37
		HITWALLSND,              // 38
		SHOOTSND,                // 39
		SHOOTDOORSND,            // 40
		PERCENT100SND,           // 41
		ENDBONUS1SND,            // 42
		ENDBONUS2SND,            // 43
		LASTSOUND
	     } soundnames;

//
// Hacks for N3D restoration
//
#define SCHUTZADSND D_ANTLPESND
#define DIESND D_BEARSND

#define GUTENTAGSND D_CAMELSND
#define CLOSEDOORSND D_CLOSDRSND
#define MISSILEHITSND D_COCHITSND
#define MISSILEFIRESND D_COCTHRSND
#define DOGATTACKSND D_DBITESND

#define ERLAUBENSND D_ELPHNTSND
#define BONUS1UPSND D_EXTRASND
#define ATKGATLINGSND D_GATLINSND
#define KEINSND D_GIRAFESND
#define DOGBARKSND D_GOATSND
#define ATKPISTOLSND D_GUNSHTSND

#define EINESND D_KANGROSND
#define GETKEYSND D_KEYSND
#define ATKKNIFESND D_KNIFESND
#define SCHABBSHASND D_MONKEYSND
#define MECHSTEPSND D_MSTEPSND
#define OPENDOORSND D_OPENDRSND
#define SPIONSND D_OSTRCHSND

#define TAKEDAMAGESND D_PAINSND
#define PLAYERDEATHSND D_PDIESND

#define PUSHWALLSND D_PWALLSND
#define HALTSND D_SHEEPSND
#define ATKMACHINEGUNSND D_SLING2SND


#define HEALTH1SND D_BONUSSND
#define HEALTH2SND D_BONUSSND
#define BONUS1SND D_BONUSSND
#define BONUS2SND D_BONUSSND
#define BONUS3SND D_BONUSSND
#define BONUS4SND D_BONUSSND
#define GETAMMOSND D_BONUSSND
#define GETAMMOBOXSND D_BONUSSND
#define GETMACHINESND D_BONUSSND
#define GETGATLINGSND D_BONUSSND

#define NAZIFIRESND D_SPITSND
#define SSFIRESND D_SPITSND
#define BOSSFIRESND D_SPITSND

#define SCHABBSTHROWSND D_COCTHRSND

//
// Base offsets
//
#define STARTPCSOUNDS		0
#define STARTADLIBSOUNDS	44
#define STARTDIGISOUNDS		88
#define STARTMUSIC		132

//
// Music names & indexes
//
typedef enum {
		NOAH01_MUS,              // 0
		NOAH02_MUS,              // 1
		NOAH03_MUS,              // 2
		NOAH04_MUS,              // 3
		NOAH05_MUS,              // 4
		NOAH06_MUS,              // 5
		NOAH07_MUS,              // 6
		NOAH08_MUS,              // 7
		NOAH09_MUS,              // 8
		NOAH10_MUS,              // 9
		NOAH11_MUS,              // 10
		LASTMUSIC
	     } musicnames;

/////////////////////////////////////////////////
//
// Thanks for playing with MUSE!
//
/////////////////////////////////////////////////

REFKEEN_NS_E
