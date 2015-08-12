/////////////////////////////////////////////////
//
// MUSE Header for .N3D
// Recreated using ECWolf definitions
//
/////////////////////////////////////////////////

#define NUMSOUNDS		44
#define NUMSNDCHUNKS		143

//
// Sound names & indexes
//
typedef enum {
		SCHUTZADSND,             // 0
		DIESND,                  // 1
		GETITEMSND,              // 2
		GUTENTAGSND,             // 3
		CLOSEDOORSND,            // 4
		MISSILEHITSND,           // 5
		MISSILEFIRESND,          // 6
		DOGATTACKSND,            // 7
		GULPSND,                 // 8
		SLEEPSND,                // 9
		ERLAUBENSND,             // 10
		BONUS1UPSND,             // 11
		ATKGATLINGSND,           // 12
		KEINSND,                 // 13
		DOGBARKSND,              // 14
		ATKPISTOLSND,            // 15
		ENDBONUS1SND,            // 16
		EINESND,                 // 17
		GETKEYSND,               // 18
		ATKKNIFESND,             // 19
		SCHABBSHASND,            // 20
		MECHSTEPSND,             // 21
		OPENDOORSND,             // 22
		SPIONSND,                // 23
		GROWLSND,                // 24 (TODO: New one for S3DNA)
		TAKEDAMAGESND,           // 25
		PLAYERDEATHSND,          // 26
		NOBONUSSND,              // 27 (TODO no digitized form exists)
		PUSHWALLSND,             // 28
		HALTSND,                 // 29
		ATKMACHINEGUNSND,        // 30
		NAZIFIRESND,             // 31
		WATERMELONHITSND,        // 32
		WATERMELONFIRESND,       // 33
		NOWAYSND,                // 34
		ESCPRESSEDSND,           // 35
		MOVEGUN1SND,             // 36
		MOVEGUN2SND,             // 37
		HITWALLSND,              // 38
		SHOOTSND,                // 39
		SHOOTDOORSND,            // 40
		ENDBONUS2SND,            // 41
		GETMACHINESND,           // 42 (TODO: Unused)
		GETGATLINGSND,           // 43 (TODO: Unused)
		LASTSOUND
	     } soundnames;

//
// Base offsets
//
// Again don't use the enum value of LASTSOUND, just use the numbers directly
#define STARTPCSOUNDS		0
#define STARTADLIBSOUNDS	44
#define STARTDIGISOUNDS		88
#define STARTMUSIC		132

//
// Music names & indexes
//
typedef enum {
		SONG1_MUS,               // 0
		ALL_GOOD_MUS,            // 1
		SONG3_MUS,               // 2
		SONG4_MUS,               // 3
		FEEDTIME_MUS,            // 4
		SONG6_MUS,               // 5
		SONG7_MUS,               // 6
		SONG8_MUS,               // 7
		SONG9_MUS,               // 8
		THEHAPPY_MUS,            // 9
		SONG11_MUS,              // 10
		LASTMUSIC
	     } musicnames;

// Hacks for N3D restoration
#define HEALTH1SND GETITEMSND
#define HEALTH2SND GETITEMSND
#define BONUS1SND GETITEMSND
#define BONUS2SND GETITEMSND
#define BONUS3SND GETITEMSND
#define BONUS4SND GETITEMSND
#define GETAMMOSND GETITEMSND

#define SSFIRESND NAZIFIRESND
#define BOSSFIRESND NAZIFIRESND

#define SCHABBSTHROWSND MISSILEFIRESND

/////////////////////////////////////////////////
//
// Thanks for playing with MUSE!
//
/////////////////////////////////////////////////
