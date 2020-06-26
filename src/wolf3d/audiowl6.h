/////////////////////////////////////////////////
//
// MUSE Header for .WL6
// Based on header from Wolf4SDL,
// with changes (inc. alpha version)
//
/////////////////////////////////////////////////

// Do NOT use any enum value to define a macro like NUMSOUNDS in a
// "version agnostic" manner, this can lead to a bit different
// machine code generated from one line in MM_SortMem.
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define NUMSOUNDS		45
#define NUMSNDCHUNKS		143
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define NUMSOUNDS		73
#define NUMSNDCHUNKS		246
#else
#define NUMSOUNDS		87
#define NUMSNDCHUNKS		288
#endif

//
// Sound names & indexes
//
typedef enum {
		HITWALLSND,              // 0
		SELECTWPNSND,            // 1
		SELECTITEMSND,           // 2
		HEARTBEATSND,            // 3
		MOVEGUN2SND,             // 4
		MOVEGUN1SND,             // 5
		NOWAYSND,                // 6
		NAZIHITPLAYERSND,        // 7
		// We can technically use the same name,
		// but good chances are it originally differed
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		FIRESND,                 // 8
#else
		SCHABBSTHROWSND,         // 8
#endif
		PLAYERDEATHSND,          // 9
		DOGDEATHSND,             // 10
		ATKGATLINGSND,           // 11
		GETKEYSND,               // 12
		NOITEMSND,               // 13
		WALK1SND,                // 14
		WALK2SND,                // 15
		TAKEDAMAGESND,           // 16
		GAMEOVERSND,             // 17
		OPENDOORSND,             // 18
		CLOSEDOORSND,            // 19
		DONOTHINGSND,            // 20
		HALTSND,                 // 21
		DEATHSCREAM2SND,         // 22
		ATKKNIFESND,             // 23
		ATKPISTOLSND,            // 24
		DEATHSCREAM3SND,         // 25
		ATKMACHINEGUNSND,        // 26
		HITENEMYSND,             // 27
		SHOOTDOORSND,            // 28
		DEATHSCREAM1SND,         // 29
		GETMACHINESND,           // 30
		GETAMMOSND,              // 31
		SHOOTSND,                // 32
		HEALTH1SND,              // 33
		HEALTH2SND,              // 34
		BONUS1SND,               // 35
		BONUS2SND,               // 36
		BONUS3SND,               // 37
		GETGATLINGSND,           // 38
		ESCPRESSEDSND,           // 39
		LEVELDONESND,            // 40
		DOGBARKSND,              // 41
		ENDBONUS1SND,            // 42
		ENDBONUS2SND,            // 43
		BONUS1UPSND,             // 44
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		BONUS4SND,               // 45
		PUSHWALLSND,             // 46
		NOBONUSSND,              // 47
		PERCENT100SND,           // 48
		BOSSACTIVESND,           // 49
		MUTTISND,                // 50
		SCHUTZADSND,             // 51
		AHHHGSND,                // 52
		DIESND,                  // 53
		EVASND,                  // 54
		GUTENTAGSND,             // 55
		LEBENSND,                // 56
		SCHEISTSND,              // 57
		NAZIFIRESND,             // 58
		BOSSFIRESND,             // 59
		SSFIRESND,               // 60
		SLURPIESND,              // 61
		TOT_HUNDSND,             // 62
		MEINGOTTSND,             // 63
		SCHABBSHASND,            // 64
		HITLERHASND,             // 65
		SPIONSND,                // 66
		NEINSOVASSND,            // 67
		DOGATTACKSND,            // 68
		FLAMETHROWERSND,         // 69
		MECHSTEPSND,             // 70
		GOOBSSND,                // 71
		YEAHSND,                 // 72
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		DEATHSCREAM4SND,         // 73
		DEATHSCREAM5SND,         // 74
		DEATHSCREAM6SND,         // 75
		DEATHSCREAM7SND,         // 76
		DEATHSCREAM8SND,         // 77
		DEATHSCREAM9SND,         // 78
		DONNERSND,               // 79
		EINESND,                 // 80
		ERLAUBENSND,             // 81
		KEINSND,                 // 82
		MEINSND,                 // 83
		ROSESND,                 // 84
		MISSILEFIRESND,          // 85
		MISSILEHITSND,           // 86
#endif
#endif
		LASTSOUND
	     } soundnames;

//
// Base offsets
//
// Again don't use the enum value of LASTSOUND, just use the numbers directly
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define STARTPCSOUNDS		0
#define STARTADLIBSOUNDS	45
#define STARTDIGISOUNDS		90
#define STARTMUSIC		135
#elif (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#define STARTPCSOUNDS		0
#define STARTADLIBSOUNDS	73
#define STARTDIGISOUNDS		146
#define STARTMUSIC		219
#else
#define STARTPCSOUNDS		0
#define STARTADLIBSOUNDS	87
#define STARTDIGISOUNDS		174
#define STARTMUSIC		261
#endif

//
// Music names & indexes
//
typedef enum {
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		WONDERIN_MUS,            // 0
		VICTORS_MUS,             // 1
		SUSPENSE_MUS,            // 2
		NAZI_NOR_MUS,            // 3
		SALUTE_MUS,              // 4
		POW_MUS,                 // 5
		CORNER_MUS,              // 6
		GETTHEM_MUS,             // 7
#else
		CORNER_MUS,              // 0
		DUNGEON_MUS,             // 1
		WARMARCH_MUS,            // 2
		GETTHEM_MUS,             // 3
		HEADACHE_MUS,            // 4
		HITLWLTZ_MUS,            // 5
		INTROCW3_MUS,            // 6
		NAZI_NOR_MUS,            // 7
		NAZI_OMI_MUS,            // 8
		POW_MUS,                 // 9
		SALUTE_MUS,              // 10
		SEARCHN_MUS,             // 11
		SUSPENSE_MUS,            // 12
		VICTORS_MUS,             // 13
		WONDERIN_MUS,            // 14
		FUNKYOU_MUS,             // 15
		ENDLEVEL_MUS,            // 16
		GOINGAFT_MUS,            // 17
		PREGNANT_MUS,            // 18
		ULTIMATE_MUS,            // 19
		NAZI_RAP_MUS,            // 20
		ZEROHOUR_MUS,            // 21
		TWELFTH_MUS,             // 22
		ROSTER_MUS,              // 23
		URAHERO_MUS,             // 24
		VICMARCH_MUS,            // 25
		PACMAN_MUS,              // 26
#endif
		LASTMUSIC
	     } musicnames;

/////////////////////////////////////////////////
//
// Thanks for playing with MUSE!
//
/////////////////////////////////////////////////
