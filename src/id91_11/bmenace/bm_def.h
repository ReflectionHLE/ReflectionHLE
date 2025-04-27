/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
 * Copyright (C) 2025 NY00123
 *
 * This file is loosely based on:
 * Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __BM_DEF_H__
#define __BM_DEF_H__

//#include <CONIO.H>

#include "id_heads.h"

REFKEEN_NS_B

/*
=============================================================================

						 GLOBAL CONSTANTS & MACROS

=============================================================================
*/

#define MAXACTORS 125
#define SLOPEMASK 7	//the low bits of a tile's north/south wall define the slope angle

#define CONVERT_GLOBAL_TO_TILE(x)  ((x)>>(G_T_SHIFT))
#define CONVERT_TILE_TO_GLOBAL(x)  ((x)<<(G_T_SHIFT))
#define CONVERT_GLOBAL_TO_PIXEL(x) ((x)>>(G_P_SHIFT))
#define CONVERT_PIXEL_TO_GLOBAL(x) ((x)<<(G_P_SHIFT))
#define CONVERT_PIXEL_TO_TILE(x)   ((x)>>(P_T_SHIFT))
#define CONVERT_TILE_TO_PIXEL(x)   ((x)<<(P_T_SHIFT))

#define SPAWN_ADJUST_Y(y, h) (CONVERT_TILE_TO_GLOBAL(y) + (CONVERT_PIXEL_TO_GLOBAL(16-(h))))

#define MAPSPOT(p,x,y) (*(mapsegs[p]+mapbwidthtable[y]/2+x))
#define BACKSPOT(x,y) MAPSPOT(0,x,y)
#define FORESPOT(x,y) MAPSPOT(1,x,y)
#define INFOSPOT(x,y) MAPSPOT(2,x,y)

#define ARRAYLENGTH(x) (sizeof(x)/sizeof(*(x)))

#ifdef BETA // REFKEEN: Shouldn't matter in port anymore. Still documenting.
#define FARSTATE statetype
#else
#define FARSTATE statetype id0_far
#endif

#define PLATFORMEDGE 23	//used as hitnorth or hitsouth value when hitting a blocking object

#define MAXPRIORITY	(PRIORITIES-1)

#define PLACESPRITE RF_PlaceSprite(&ob->sprite, ob->x, ob->y, ob->shapenum, \
	ob->white!=0 ? maskdraw : spritedraw, ob->priority)

#define FRAG_SPAREPLAYER 10
#define FRAG_SIDEWAYS    20

/*
=============================================================================

						   GLOBAL TYPES

=============================================================================
*/

// REFKEEN: Let's use boolean instead of id0_boolean_t (at least for now?)
// given the game code's usage of other integer types further below.
typedef id0_boolean_t boolean;
// REFKEEN: Even if not using SDL directly from game code, these are useful!
//SDL-style integer types - just to make future SDL ports easier
typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint8_t Uint8;
typedef int8_t Sint8;
typedef uint32_t Uint32;
typedef int32_t Sint32;
//Note: only the game code (BM_*.C) uses these!

typedef enum {
	arrow_North,     // 0
	arrow_East,      // 1
	arrow_South,     // 2
	arrow_West,      // 3
	arrow_NorthEast, // 4
	arrow_SouthEast, // 5
	arrow_SouthWest, // 6
	arrow_NorthWest, // 7
	arrow_None       // 8
} arrowdirtype;

typedef enum
{
	nothing,         //  0
	playerobj,       //  1
	decoobj,         //  2
#if (EPISODE == 2)
	jimobj,
#endif
	grenadeobj,      //  3
	classtype_4,     //  4
	explosionobj,    //  5
	bonusobj,        //  6
	wormobj,         //  7
#if (EPISODE != 1)
	kamikazeobj,
#endif
	bugobj,          //  8
#if (EPISODE != 1)
	beeobj,
	bluedevilobj,
#endif
#if (EPISODE == 2)
	trashbossobj,
#endif
	punchyobj,       //  9
	gorpobj,         // 10
	cobraobj,        // 11
	hostageobj,      // 12
	bossobj,         // 13
#if (EPISODE == 2)
	antobj,
#endif
	platformobj,     // 14
	shuttleobj,      // 15
	robopalobj,      // 16
	shotobj,         // 17
	playerwormobj,   // 18
	guardobj,        // 19
	tankobj,         // 20
	pushableobj,     // 21
	blockedobj,      // 22
	bouncerobj,      // 23
	acidmonsterobj,  // 24
	enemyshotobj,    // 25
	caterpillarobj,  // 26
#if (EPISODE == 2)
	queenobj,
	nymphobj,        // ??? (never used, nymphs use beeobj)
#endif
#if (EPISODE != 1)
	spikeyobj,
#endif
	fallingbrickobj, // 27
#if (EPISODE != 1)
	trashtruckobj,
#endif
	turretobj,       // 28
	triangleobj,     // 29
#if (EPISODE == 1)
	beeobj,          // 30
	sewermanobj,     // 31
#endif
#if (EPISODE != 2)
	hairyobj,        // 32
#endif
#if (EPISODE == 1)
	skullheadobj,    // 33
	skullhandobj,    // 34
	classtype_35,    // 35
#endif
	jumpbombobj,     // 36
	barrelobj,       // 37
	classtype_38,    // 38
	slimerobj,       // 39
	devilobj,        // 40
#if (EPISODE == 1)
	helicopterobj,   // 41
	parabotobj       // 42
#elif (EPISODE == 3)
	cyborgobj,
	goliathobj,
	cainobj,
	grenadebotobj
#endif
} classtype;

typedef enum {
	//Note: these values don't change between episodes
	INTILE_NOTHING = 0,    // 0
	INTILE_POLE,           // 1
	INTILE_HURTPLAYER,     // 2
	INTILE_KEYCARDSLOT,    // 3
	INTILE_GREENTRIANGLE,  // 4
	INTILE_BETADOOR,       // 5 (same behavior as INTILE_DOOR; not used in the final game's tile attributes)
	INTILE_DOOR,           // 6
	INTILE_TELEPORT,       // 7
	INTILE_SECRETDOOR,     // 8
	INTILE_BLUESLOT,       // 9
	INTILE_GREENSLOT,      //10
	INTILE_REDSLOT,        //11
	INTILE_CYANSLOT,       //12
	INTILE_RADIATION,      //13
	INTILE_NUKESOCKET,     //14
	INTILE_HOSTAGEKEYSLOT, //15
	INTILE_KILLPLAYER,     //16
	INTILE_OPENDOOR,       //17
	INTILE_BRIDGESWITCH,   //18
	INTILE_PLATFORMSWITCH, //19
	INTILE_TURBOSHUTTLE,   //20
	INTILE_BOSSTRIGGER,    //21
	INTILE_HINTTRIGGER,    //22
	INTILE_NUMBEREDSWITCH, //23
	INTILE_TOWORM,         //24
	INTILE_SPIKETRIGGER,   //25
	INTILE_TOHUMAN,        //26
	INTILE_BEACON,         //27
	INTILE_FOREGROUND = 0x80
} intiletype;

#define INTILE_TYPEMASK (((Uint8)~0)^INTILE_FOREGROUND)
#define INTILETYPE(mtile) (tinf[(mtile)+INTILE] & INTILE_TYPEMASK)

// GoPlat direction arrow icons:
#define DIRARROWSTART 58
#define DIRARROWEND   (DIRARROWSTART+arrow_None)
// Bump and Stop icons for platforms:
#define BUMPICON 70	// makes platform turn around
#define STOPICON 71	// forces platform to stop until the icon disappears

typedef enum {
	DEMO_HIGHSCORE,
	DEMO_APOGEE,
#if (EPISODE == 1)
	DEMO_FIRST,
	DEMO_SECOND,
#endif
	DEMO_LEVELWARP
} demonames;

typedef enum {
	ex_stillplaying, ex_died, ex_completed, ex_warped,
	ex_resetgame, ex_loadedgame, ex_abort
} exittype;

typedef enum {cl_noclip, cl_midclip, cl_fullclip} cliptype;

// REFKEEN - enum types for fields should be outside struct if we want to
// be able to build the same code as C++. It's also good for other reasons.
typedef enum {step,slide,think,stepthink,slidethink} progresstype;
typedef enum {push_none, push_down, push_up} pushtype;

typedef struct statestruct
{
	Sint16 leftshapenum, rightshapenum;
	progresstype progress;
	//enum {step,slide,think,stepthink,slidethink} progress;
	boolean skippable;
	pushtype pushtofloor;
	//enum {push_none, push_down, push_up} pushtofloor;
	Sint16 tictime;
	Sint16 xmove;
	Sint16 ymove;
	// REFKEEN - Rename function pointers: think ==> thinkptr comes
	// from conflict with the 'think' enum value for progress.
	void (*thinkptr) (struct objstruct*);
	void (*contactptr) (struct objstruct*, struct objstruct*);
	void (*reactptr) (struct objstruct*);
#ifdef BETA
	struct statestruct *nextstate;
#else
	struct statestruct id0_far *nextstate;
#endif
} statetype;

// REFKEEN - enum type for active field should be outside struct if we want
// to be able to build the same code as C++. It's also good for other reasons.
typedef enum {no, yes, allways, removable} activetype;

typedef struct objstruct
{
	classtype obclass;
	activetype active;
	boolean needtoreact;
	cliptype needtoclip;
	Uint16 nothink;
	Uint16 x, y;
	Sint16 xdir, ydir;
	Sint16 xmove, ymove;
	Sint16 xspeed, yspeed;
	Sint16 ticcount;
	FARSTATE *state;
	Uint16 shapenum;
	Uint16 priority;
	Uint16 left, top, right, bottom, midx;
	Uint16 tileleft, tiletop, tileright, tilebottom, tilemidx;
	Sint16 hitnorth, hiteast, hitsouth, hitwest;
	boolean shootable;
#ifdef BETA
	Sint16 health;
	Sint16 white;
	Sint16 _unknown;	// never used in the BETA
#else
	boolean spareplayer;
	Sint16 health;
	Sint16 white;
#endif
	Sint16 temp1, temp2, temp3, temp4, temp5, temp6, temp7;
	void *sprite;
	struct objstruct *next, *prev;
} objtype;

typedef struct
{
	Sint32 score, nextextra;
	Sint16 mapon;
	struct
	{
#ifdef BETA
		Sint16 landmines;	// ? (never used in the BETA)
		Sint16 green;
#else
		Sint8 landmines, green, red;
#endif
	} grenades;
	struct
	{
		Sint16 keycards, keys;
	} keys;
	Uint16 gotshard[4];	// game uses ++ when collecting one, so not boolean
	Uint16 specialkeys;	// game uses ++ when collecting one, so not boolean
	Uint16 nukestate;	// is set to 2 when placing the bomb, so definitely not boolean
	boolean got_pill;
	boolean got_exitkey;
#ifndef BETA
	Sint16 trianglekey;	// game uses ++ when collecting one and -- when using one, so definitely not boolean
#endif
	boolean got_robopal;
#ifdef BETA
	Sint16 potions;
	Sint16 lives;
	Sint16 food;
	Sint16 difficulty;
	Sint16 autofire;
	Sint16 ammo;
	Sint16 clips;
	boolean savedhostage;
#else
	Sint8 potions;
	Sint8 lives;
	Sint8 gems;
	Sint8 health;
	Sint8 difficulty;
	Sint8 autofire;
	Sint8 ammo;
	Sint8 clips;
	Sint8 weapon;
	boolean savedhostage;
	boolean got_warpgem;
	boolean	hint_grenade;
	boolean	hint_machinegun;
	boolean	hint_keycard;
	boolean	hint_key;
	boolean	hint_crystalshard;
	boolean	hint_supergun;
	boolean	hint_landmine;
	boolean	hint_gem;
	boolean	hint_firstaid;
	boolean	hint_invincible;
	boolean	hint_robopal;
	boolean	hint_switchbridge;
	boolean	hint_switchplat;
	boolean	hint_colorsequence;
	boolean	hint_opendoor;
	boolean	hint_plasmabolts;
	boolean	hint_warpgem;
	boolean	hint_beacon;
	Uint16 _zero;
#endif
	objtype *riding;
} gametype;

/*
=============================================================================
						HACK IMPORTS
=============================================================================
*/

extern boolean helpmessages;	//declared in ID_US_2.C
extern boolean practicemode;
extern Uint8 grenadescan;

void CA_SetGrPurge(void);
void CA_FreeGraphics(void);

/*
=============================================================================

						 BM_MAIN DEFINITIONS

=============================================================================
*/

extern boolean debugUnlocked;
extern boolean textdrawscores;
extern Uint16 oldmapon;
extern Sint16 practiceTimeLeft;
extern Uint16 respawnx;
extern Uint16 respawny;
extern Sint16 bosshealth;
extern Sint16 oldhealthbarlength;
extern Sint16 timeleft;
extern boolean pirated;

extern Uint16 tedlevelnum;
extern id0_char_t str[80];
extern id0_char_t str2[20];
extern boolean storedemo;
extern boolean tedlevel;
extern Uint16 currentmusic;
extern Sint16 bossdiv;

void SizeText(const id0_char_t *text, Uint16 *width, Uint16 *height);
void ShutdownId(void);
void Quit(const id0_char_t *error);
void TEDDeath(void);
void bmenace_exe_main(void);

/*
=============================================================================

						 BM_STRS DEFINITIONS

=============================================================================
*/

extern const id0_char_t id0_far str_easy[];
extern const id0_char_t id0_far str_normal[];
extern const id0_char_t id0_far str_hard[];
extern const id0_char_t id0_far str_gameover[];
extern const id0_char_t id0_far str_nomemgame[];
extern const id0_char_t id0_far str_didntmakeit[];
extern const id0_char_t id0_far str_tryagain[];
extern const id0_char_t id0_far str_nomemlevel[];
extern const id0_char_t id0_far str_onemoment[];
extern const id0_char_t id0_far str_godmodeon[];
extern const id0_char_t id0_far str_godmodeoff[];
extern const id0_char_t id0_far str_freeitems[];
extern const id0_char_t id0_far str_jumpcheaton[];
extern const id0_char_t id0_far str_jumpcheatoff[];
extern const id0_char_t id0_far str_warpprompt[];
extern const id0_char_t id0_far str_practiceprompt[];
extern const id0_char_t id0_far str_paused[];
extern const id0_char_t id0_far str_location[];
extern const id0_char_t id0_far str_score[];
extern const id0_char_t id0_far str_extra[];
extern const id0_char_t id0_far str_keycards[];
extern const id0_char_t id0_far str_shards[];
extern const id0_char_t id0_far str_level[];
extern const id0_char_t id0_far str_leasy[];
extern const id0_char_t id0_far str_lnormal[];
extern const id0_char_t id0_far str_lhard[];
extern const id0_char_t id0_far str_keys[];
extern const id0_char_t id0_far str_ammo[];
extern const id0_char_t id0_far str_lives[];
#ifdef BETA
extern const id0_char_t id0_far str_potions[];
extern const id0_char_t id0_far str_food[];
#else
extern const id0_char_t id0_far str_clips[];
extern const id0_char_t id0_far str_gems[];
#endif
extern const id0_char_t id0_far str_question[];
extern const id0_char_t id0_far str_nomemmusic[];
extern const id0_char_t id0_far str_forgothostage[];

/*
=============================================================================

						 BM_TEXT DEFINITIONS

=============================================================================
*/

void HelpMessage(const id0_char_t id0_far *message);
void HelpScreens(void);
void FinaleLayout(void);

/*
=============================================================================

						 BM_DEMO DEFINITIONS

=============================================================================
*/

extern boolean scorescreenkludge;

void CheckLastScan(void);
#ifdef BETA
void TitleScreen(void);
void MiscScreen(void);
#else
void TitleScreen(Uint16 seconds);
void MiscScreen(Sint16 num);
#endif
void RunDemo(Sint16 num);
void DrawHighScores(void);
void CheckHighScore(Sint32 score, Sint16 completed);
void ShowHighScores(void);

/*
=============================================================================

						 BM_GAME DEFINITIONS

=============================================================================
*/

void NewGame(void);
void ResetGame(void);
boolean SaveTheGame(BE_FILE_T handle);
boolean LoadTheGame(BE_FILE_T handle);
void SetupGameLevel(boolean loadnow);
void GameLoop(void);

/*
=============================================================================

						 BM_PLAY DEFINITIONS

=============================================================================
*/
extern boolean upheld;
extern Uint16 playstate;
extern gametype gamestate;
extern ControlInfo c;
extern boolean button0held;
extern boolean button1held;
extern boolean throwkeyheld;
extern objtype objarray[MAXACTORS];
extern objtype *lastobj;
extern objtype *objfreelist;
extern objtype *newobj;
extern objtype *player;
extern objtype *scoreobj;
extern Uint16 originxtilemax;
extern Uint16 originytilemax;
extern Uint16 centerlevel;
extern Uint16 objectcount;
extern Sint16 inactivateleft;
extern Sint16 inactivateright;
extern Sint16 inactivatetop;
extern Sint16 inactivatebottom;
extern objtype dummyobj;

void StatusWindow(void);
void CenterActor(objtype *ob);
void InitObjArray(void);
Sint16 GetNewObj(boolean usedummy);
void RemoveObj(objtype *ob);
void GivePoints(Uint16 points);
void StopMusic(void);
void StartMusic(Uint16 num);
void PlayLoop(void);

/*
=============================================================================

						 BM_DEBUG DEFINITIONS

=============================================================================
*/

extern boolean showscorebox;

extern boolean singlestep;
extern boolean jumpcheat;
extern boolean godmode;
extern Uint16 extraVBLs;
extern boolean DebugOk;

void CheckKeys(void);
void StartDemoRecord(void);
void EndDemoRecord(void);

/*
=============================================================================

						 BM_STATE DEFINITIONS

=============================================================================
*/

extern Sint16 wallclip[8][16];

extern Sint16 xtry;
extern Sint16 ytry;

boolean CheckPosition(objtype *ob);
boolean StatePositionOk(objtype *ob, FARSTATE *state);
void PushObj(objtype *ob);
void ClipToSpriteSide(objtype *push, objtype *solid);
void ClipToSpriteTop(objtype *push, objtype *solid);
void ClipToSprite(objtype *push, objtype *solid, boolean squish);
void ClipPushSprite(objtype *push, objtype *solid, boolean squish);
void StateMachine(objtype *ob);
void NewState(objtype *ob, FARSTATE *state);
void ChangeState(objtype *ob, FARSTATE *state);
boolean OnScreen(objtype *ob);
void DoGravity(objtype *ob);
void AccelerateX(objtype *ob, Sint16 dir, Sint16 maxspeed);
void AccelerateY(objtype *ob, Sint16 dir, Sint16 maxspeed);
void FrictionX(objtype *ob);
void ProjectileThink(objtype *ob);
void VelocityThink(objtype *ob);
void DrawReact(objtype *ob);
void WalkReact(objtype *ob);

/*
=============================================================================

						 BM_SNAKE DEFINITIONS

=============================================================================
*/

extern Sint16 hintstate;
extern boolean teleported;
extern boolean bossActive;
extern boolean snakedied;
extern Sint16 cooldown;

extern Uint16 jumptime;
extern Uint16 colorseqnum;
extern Uint8 shuttlecooldown;
extern Sint16 doorx;
extern Sint16 doory;
extern Uint16 invincibility;
extern Uint32 leavepoletime;
extern boolean upKeyDown;
extern boolean downKeyDown;
extern boolean leftKeyDown;
extern boolean rightKeyDown;
extern boolean button0down;
extern boolean button1down;
extern boolean grenadeKeyDown;

extern FARSTATE s_snakedie1;
extern FARSTATE s_snakedie2;
extern FARSTATE s_snakeskeleton;
extern FARSTATE s_snakeshield1;
extern FARSTATE s_snakeshield2;
extern FARSTATE s_snakeUNUSED1;
extern FARSTATE s_snakestand;
extern FARSTATE s_snakestanduse;
extern FARSTATE s_snakeUNUSED2;
extern FARSTATE s_snakeslide;
extern FARSTATE s_snaketeleport1;
extern FARSTATE s_snaketeleport2;
extern FARSTATE s_snakeuse1;
extern FARSTATE s_snakeuse2;
extern FARSTATE s_snakeopendoor1;
extern FARSTATE s_snakeopendoor2;
extern FARSTATE s_snakeduck;
extern FARSTATE s_snakeplacemine;
extern FARSTATE s_snakeladder;
extern FARSTATE s_snakeclimb1;
extern FARSTATE s_snakeclimb2;
extern FARSTATE s_snakewalk1;
extern FARSTATE s_snakewalk2;
extern FARSTATE s_snakewalk3;
extern FARSTATE s_snakewalk4;
extern FARSTATE s_snakethrow1;
extern FARSTATE s_snakethrow2;
extern FARSTATE s_snakethrow3;
extern FARSTATE s_snakeairthrow1;
extern FARSTATE s_snakeairthrow2;
extern FARSTATE s_snakeairthrow3;
extern FARSTATE s_snakejump1;
extern FARSTATE s_snakejump2;
extern FARSTATE s_snakejump3;
extern FARSTATE s_snakestandfire1;
extern FARSTATE s_snakestandfire2;
extern FARSTATE s_snakestandfire3;
extern FARSTATE s_snakeairfire1;
extern FARSTATE s_snakeairfire2;
extern FARSTATE s_snakeairfire3;
extern FARSTATE s_snakeduckfire1;
extern FARSTATE s_snakeduckfire2;
extern FARSTATE s_snakeduckfire3;
extern FARSTATE s_snakestandsutofire1;
extern FARSTATE s_snakestandautofire2;
extern FARSTATE s_snakeduckautofire1;
extern FARSTATE s_snakeduckautofire2;
extern FARSTATE s_snakeairautofire1;
extern FARSTATE s_snakeairautofire2;

void SpawnPlayer(Uint16 x, Uint16 y, Sint16 xdir);
boolean CheckInteractiveTiles(objtype *ob);
boolean CheckGrabPole(objtype *ob);
boolean PoleActions(objtype *ob);
void SnakeSlideThink(objtype *ob);
void SnakeThrow(objtype *ob);
void SnakeTeleportThink(objtype *ob);
void SnakeUseThink(objtype *ob);
void SnakeOpenDoorThink(objtype *ob);
void SnakeFireThink(objtype *ob);
void SnakeDuckFireThink(objtype *ob);
void SnakeAutofireThink(objtype *ob);
void SnakeAirAutofireThink(objtype *ob);
void SnakeDuckAutofireThink(objtype *ob);
void SnakeAirNofireThink(objtype *ob);
void PollControls(void);
void SnakeStartJump(objtype *ob);
void HurtPlayer(objtype *ob, Sint16 damage);
void SnakeDieThink(objtype *ob);
void SnakeDeadThink(objtype *ob);
void SnakeShieldContact(objtype *ob, objtype *hit);
void SnakePauseThink(objtype *ob);
void SnakeWalkThink(objtype *ob);
void SnakeAirThink(objtype *ob);
void SnakeLadderThink(objtype *ob);
void SnakeClimbThink(objtype *ob);
void CheckPole(objtype *ob);
void SnakeAttackThink(objtype *ob);
void KillPlayer(void);
void SnakeStandContact(objtype *ob, objtype *hit);
void SnakeStandReact(objtype *ob);
void SnakeWalkReact(objtype *ob);
void SnakeAirReact(objtype *ob);
void SnakeDieReact(objtype *ob);
void CheckInTiles(objtype *ob);
void HandleRiding(objtype *ob);

#if (EPISODE == 2)

extern FARSTATE s_snakewormstand;
extern FARSTATE s_snakewormwalk1;
extern FARSTATE s_snakewormwalk2;
extern FARSTATE s_snakewormair1;
extern FARSTATE s_snakewormair2;
extern FARSTATE s_snakewormair3;
extern FARSTATE s_snakewormattack;
extern FARSTATE s_snakewormairattack;

void SnakeWormStandActions(objtype *ob);
void SnakeWormStandThink(objtype *ob);
void SnakeWormWalkThink(objtype *ob);
void SnakeWormAirThink(objtype *ob);
void SnakeWormStandReact(objtype *ob);
void SnakeWormWalkReact(objtype *ob);
void SnakeWormAirReact(objtype *ob);

#endif

/*
=============================================================================

						 BM_SNAK2 DEFINITIONS

=============================================================================
*/

extern Uint16 bounceangle[8][8];
extern Uint16 bonusscores[10];

extern FARSTATE s_score;
extern FARSTATE s_smallexplosion1;
extern FARSTATE s_smallexplosion2;
extern FARSTATE s_smallexplosion3;
extern FARSTATE s_chunk1fly1;
extern FARSTATE s_chunk1fly2;
extern FARSTATE s_chunkeye1;
extern FARSTATE s_chunkeye2;
extern FARSTATE s_chunk3fly1;
extern FARSTATE s_chunk3fly2;
extern FARSTATE s_chunkground;
extern FARSTATE s_metalchunk1;
extern FARSTATE s_metalchunk2;
extern FARSTATE s_metalchunk3;
extern FARSTATE s_metalchunk4;
extern FARSTATE s_metalchunkground;
extern FARSTATE s_frag1;
extern FARSTATE s_frag2;
extern FARSTATE s_frag3;
extern FARSTATE s_frag4;
extern FARSTATE s_grenadeg1;
extern FARSTATE s_grenadeg2;
extern FARSTATE s_grenadeg3;
extern FARSTATE s_grenadeg4;
extern FARSTATE s_grenader1;
extern FARSTATE s_grenader2;
extern FARSTATE s_grenader3;
extern FARSTATE s_grenader4;
extern FARSTATE s_grenadeexplosion1;
extern FARSTATE s_grenadeexplosion2;
extern FARSTATE s_grenadeexplosion3;
extern FARSTATE s_grenadeexplosion4;
extern FARSTATE s_grenadeexplosion5;
extern FARSTATE s_grenadeexplosion6;
extern FARSTATE s_robopalshot1;
extern FARSTATE s_robopalshot2;
extern FARSTATE s_plasmabolt;
extern FARSTATE s_superplasmabolt;
extern FARSTATE s_explosion_a1;
extern FARSTATE s_explosion_a2;
extern FARSTATE s_explosion_b1;
extern FARSTATE s_explosion_b2;

void SnakeContact(objtype *ob, objtype *hit);
void SpawnScore(void);
void ResetScoreObj(void);
void UpdateScorebox(objtype *ob);
void HurtObject(objtype *ob, Sint16 damage);
void SpawnExplosion(Uint16 x, Uint16 y);
void DoShooting(Uint16 x, Uint16 y, Sint16 xdir, Uint16 damage);
void ChunkBloom(objtype *ob, Uint16 x, Uint16 y, Uint16 dir);
void ChunkReact(objtype *ob);
void FragBloom(Uint16 x, Uint16 y, Sint16 dir);
void FragContact(objtype *ob, objtype *hit);
void FragReact(objtype *ob);
void GrenadeExplosionThink(objtype *ob);
void ThrowPower(Uint16 x, Uint16 y, Uint16 dir);
void GrenadeContact(objtype *ob, objtype *hit);
void GrenadeExplosionContact(objtype *ob, objtype *hit);
void GrenadeReact(objtype *ob);
void SpawnShot(Uint16 x, Uint16 y, Sint16 xdir);
void SpawnLaserShot(Uint16 x, Uint16 y, Sint16 xdir);
void SpawnSuperLaserShot(Uint16 x, Uint16 y, Sint16 xdir);
void RobopalShotContact(objtype *ob, objtype *hit);
void PlasmaboltContact(objtype *ob, objtype *hit);
void PlasmaboltExplode(objtype *ob);
void ShotThink(objtype *ob);
void ShotReact(objtype *ob);
void SuperPlasmaboltReact(objtype *ob);

/*
=============================================================================

						 BM_SPEC DEFINITIONS

=============================================================================
*/

extern const id0_char_t *levelnames[];

void ScanInfoPlane(void);
void HostageDialog(void);
void HintDialog(void);
void BossDialog(void);

/*
=============================================================================

						 BM_ACT1 DEFINITIONS

=============================================================================
*/

void ProjectileReact(objtype *ob);

extern FARSTATE s_platform;
void SpawnPlatform(Uint16 x, Uint16 y, arrowdirtype dir);

extern FARSTATE s_apogee;
void SpawnApogee(Uint16 x, Uint16 y, arrowdirtype dir);

void PlatformThink(objtype *ob);

#if (EPISODE != 1)
extern FARSTATE s_shuttle;
void SpawnShuttle(Uint16 x, Uint16 y, arrowdirtype dir);
#endif

#if (EPISODE == 2)
extern FARSTATE s_queen1;
extern FARSTATE s_queen2;
void SpawnQueen(Uint16 x, Uint16 y, arrowdirtype dir);
void QueenReact(objtype *ob);
#endif

#if (EPISODE == 1)
extern FARSTATE s_helicopter1;
extern FARSTATE s_helicopter2;
extern FARSTATE s_helicopter3;
extern FARSTATE s_parabotwalk1;
extern FARSTATE s_parabotwalk2;
extern FARSTATE s_parabotjump;
extern FARSTATE s_parabotfall;
void SpawnHelicopter(Uint16 x, Uint16 y, arrowdirtype dir);
void HelicopterReact(objtype *ob);
void HelicopterSpawnParabot(objtype *ob);
void ParabotWalkThink(objtype *ob);
void ParabotContact(objtype *ob, objtype *hit);
void ParabotFallReact(objtype *ob);
void ParabotJumpReact(objtype *ob);
#else
extern FARSTATE s_caterpillarhead1;
extern FARSTATE s_caterpillarhead2;
extern FARSTATE s_caterpillarbody;
void SpawnCaterpillar(Uint16 x, Uint16 y, arrowdirtype dir, Uint16 kind);
#endif

void GoplatThink(objtype *ob);

extern FARSTATE s_cobrawalk1;
extern FARSTATE s_cobrawalk2;
extern FARSTATE s_cobrarun1;
extern FARSTATE s_cobrarun2;
extern FARSTATE s_cobraattack1;
extern FARSTATE s_cobraattack2;
extern FARSTATE s_cobraspit1;
extern FARSTATE s_cobraspit2;
void SpawnCobra(Uint16 x, Uint16 y);
void CobraRunThink(objtype *ob);
void CobraAttackThink(objtype *ob);
void CobraWalkThink(objtype *ob);

#if (EPISODE == 1)
extern FARSTATE s_sewermanwalk1;
extern FARSTATE s_sewermanwalk2;
extern FARSTATE s_sewermanattack;
extern FARSTATE s_sewermanspit1;
extern FARSTATE s_sewermanspit2;
void SpawnSewerman(Uint16 x, Uint16 y);
void SewermanAttackThink(objtype *ob);
void SewermanWalkThink(objtype *ob);

extern FARSTATE s_hostage1stand1;
extern FARSTATE s_hostage1stand2;
extern FARSTATE s_hostage2stand1;
extern FARSTATE s_hostage2stand2;
extern FARSTATE s_hostage3stand1;
extern FARSTATE s_hostage3stand2;
void SpawnHostage(Uint16 x, Uint16 y, Uint16 type);
#endif

#if (EPISODE == 2)
extern FARSTATE s_jimstand;
extern FARSTATE s_jimattack1;
extern FARSTATE s_jimattack2;
extern FARSTATE s_georgestand1;
extern FARSTATE s_georgestand2;
extern FARSTATE s_keenstand1;
extern FARSTATE s_keenstand2;
void SpawnJim(Uint16 x, Uint16 y);
void SpawnGeorge(Uint16 x, Uint16 y);
void SpawnKeen(Uint16 x, Uint16 y);
void JimThink(objtype *ob);
void JimAttackThink(objtype *ob);
#endif

#if (EPISODE != 1)
extern FARSTATE s_hostage1stand;
extern FARSTATE s_hostage1walk;
extern FARSTATE s_hostage2stand1;
extern FARSTATE s_hostage2stand2;
extern FARSTATE s_hostage3stand1;
extern FARSTATE s_hostage3stand2;
void SpawnHostage1(Uint16 x, Uint16 y);
void Hostage1Think(objtype *ob);
void SpawnHostage2(Uint16 x, Uint16 y);
void Hostage2Think(objtype *ob);
void SpawnHostage3(Uint16 x, Uint16 y);
void Hostage3Think(objtype *ob);
#endif

extern FARSTATE s_bosshead1;
extern FARSTATE s_bosshead2;
extern FARSTATE s_bosshead3;
extern FARSTATE s_bossheadfade1;
extern FARSTATE s_bossheadfade2;
extern FARSTATE s_bossheadfade3;
extern FARSTATE s_bossheadmetal;
void SpawnBosshead(Uint16 x, Uint16 y);
void BossheadThink(objtype *ob);
void BossheadMetalThink(objtype *ob);
void BossheadReact(objtype *ob);

#if (EPISODE == 1)
extern FARSTATE s_manglestand1;
extern FARSTATE s_manglestand2;
extern FARSTATE s_manglewalk1;
extern FARSTATE s_manglewalk2;
extern FARSTATE s_manglejump;
extern FARSTATE s_mangledead1;
extern FARSTATE s_mangledead2;
extern FARSTATE s_mangledead3;
void SpawnMangle(Uint16 x, Uint16 y);
void MangleThink(objtype *ob);
void MangleDeadThink(objtype *ob);
void MangleJumpReact(objtype *ob);
#endif

/*
=============================================================================

						 BM_ACT2 DEFINITIONS

=============================================================================
*/

extern boolean robopal_mustfire;

void CeilWalkReact(objtype *ob);

extern FARSTATE s_bonus1;
extern FARSTATE s_bonus2;
extern FARSTATE s_bonusrise;
void SpawnBonus(Uint16 x, Uint16 y, Uint16 num);
void BonusThink(objtype *ob);
void FlyBonusThink(objtype *ob);

#if (EPISODE != 1)
extern FARSTATE s_bug1;
extern FARSTATE s_bug2;
void SpawnBug(Uint16 x, Uint16 y);
void BugThink(objtype *ob);
void BugReact(objtype *ob);

extern FARSTATE s_bee1;
extern FARSTATE s_bee2;
void SpawnBee(Uint16 x, Uint16 y);
#if (EPISODE == 2)
extern FARSTATE s_nymph1;
extern FARSTATE s_nymph2;
void SpawnNymph(objtype *ob);
#endif
void BeeThink(objtype *ob);
void BeeReact(objtype *ob);
#endif

extern FARSTATE s_robopalwait1;
extern FARSTATE s_robopalwait2;
extern FARSTATE s_robopalfollow1;
extern FARSTATE s_robopalfollow2;
extern FARSTATE s_robopalteleport1;
extern FARSTATE s_robopalteleport2;
extern FARSTATE s_robopalteleport3;
extern FARSTATE s_robopalteleport4;
extern FARSTATE s_robopalteleport5;
void SpawnRobopal(Uint16 x, Uint16 y);
void RobopalThink(objtype *ob);
void RobopalReact(objtype *ob);
void RobopalTeleportThink(objtype *ob);

extern FARSTATE s_wormwalk1;
extern FARSTATE s_wormwalk2;
extern FARSTATE s_wormjump;
extern FARSTATE s_grenadefire1;
extern FARSTATE s_grenadefire2;
void SpawnWorm(Uint16 x, Uint16 y);
void WormWalkThink(objtype *ob);
void GrenadeFireThink(objtype *ob);
void WormJumpReact(objtype *ob);
void GrenadeFireReact(objtype *ob);

#if (EPISODE == 1)
extern FARSTATE s_plasmawormwalk1;
extern FARSTATE s_plasmawormwalk2;
extern FARSTATE s_plasmawormjump;
void SpawnPlasmaWorm(Uint16 x, Uint16 y);
void PlasmaWormWalkThink(objtype *ob);
void PlasmaWormJumpReact(objtype *ob);
#endif

#if (EPISODE != 1)
extern FARSTATE s_bluedevilwalk1;
extern FARSTATE s_bluedevilwalk2;
extern FARSTATE s_bluedevilwalk3;
extern FARSTATE s_bluedevilwalk4;
extern FARSTATE s_bluedeviljump;
void SpawnBlueDevil(Uint16 x, Uint16 y);
void BlueDevilWalkReact(objtype *ob);
void BlueDevilJumpReact(objtype *ob);

extern FARSTATE s_kamikazestand1;
extern FARSTATE s_kamikazestand2;
extern FARSTATE s_kamikazeturn1;
extern FARSTATE s_kamikazeturn2;
extern FARSTATE s_kamikazewalk1;
extern FARSTATE s_kamikazewalk2;
extern FARSTATE s_kamikazedie1;
extern FARSTATE s_kamikazedie2;
void SpawnKamikaze(Uint16 x, Uint16 y);
void KamikazeTurnThink(objtype *ob);
void KamikazeDieThink(objtype *ob);
void KamikazeWalkThink(objtype *ob);
#endif

#if (EPISODE == 2)

extern FARSTATE s_trashbossdie;
extern FARSTATE s_trashbossstand;
extern FARSTATE s_trashbosswalk1;
extern FARSTATE s_trashbosswalk2;
extern FARSTATE s_trashbossthrow;
extern FARSTATE s_trashcan1;
extern FARSTATE s_trashcan2;
extern FARSTATE s_trashcan3;
extern FARSTATE s_trashcan4;
extern FARSTATE s_trashapple1;
extern FARSTATE s_trashapple2;
extern FARSTATE s_trashapple3;
extern FARSTATE s_trashapple4;
extern FARSTATE s_trashbird1;
extern FARSTATE s_trashbird2;
void SpawnTrashBoss(Uint16 x, Uint16 y);
void TrashBossStandThink(objtype *ob);
void TrashBossWalkThink(objtype *ob);
void TrashBossReact(objtype *ob);
void TrashBossAttackThink(objtype *ob);

#else

extern FARSTATE s_jumpbomb;
extern FARSTATE s_jumpbombfly1;
extern FARSTATE s_jumpbombfly2;
extern FARSTATE s_jumpbombfly3;
void SpawnJumpbomb(Uint16 x, Uint16 y);
void JumpbombThink(objtype *ob);
void JumpbombReact(objtype *ob);

extern FARSTATE s_devilwalk1;
extern FARSTATE s_devilwalk2;
extern FARSTATE s_devilwalk3;
extern FARSTATE s_devilwalk4;
extern FARSTATE s_devilburn1;
extern FARSTATE s_devilburn2;
extern FARSTATE s_devilstand;
extern FARSTATE s_deviljump;
void SpawnDevil(Uint16 x, Uint16 y);
void DevilWalkThink(objtype *ob);
void DevilBurnThink(objtype *ob);
void DevilWalkReact(objtype *ob);
void DevilJumpReact(objtype *ob);

extern FARSTATE s_hairywalk1;
extern FARSTATE s_hairywalk2;
extern FARSTATE s_hairywalk3;
extern FARSTATE s_hairywalk4;
extern FARSTATE s_hairystand;
extern FARSTATE s_hairylook;
extern FARSTATE s_hairyjump;
void SpawnHairy(Uint16 x, Uint16 y);
void HairyWalkThink(objtype *ob);
void HairyStandThink(objtype *ob);
void HairyLookThink(objtype *ob);
void HairyReact(objtype *ob);
void HairyJumpReact(objtype *ob);

#endif

extern FARSTATE s_punchystand;
extern FARSTATE s_punchywalk1;
extern FARSTATE s_punchywalk2;
extern FARSTATE s_punchywalk3;
extern FARSTATE s_punchywalk4;
extern FARSTATE s_punchyattack1;
extern FARSTATE s_punchyattack2;
void SpawnPunchy(Uint16 x, Uint16 y);
void PunchyThink(objtype *ob);

extern FARSTATE s_gorpceil1;
extern FARSTATE s_gorpceil2;
extern FARSTATE s_gorpceil3;
extern FARSTATE s_gorpceil4;
extern FARSTATE s_gorpwalk1;
extern FARSTATE s_gorpwalk2;
extern FARSTATE s_gorpwalk3;
extern FARSTATE s_gorpwalk4;
extern FARSTATE s_gorpfall;
void SpawnGorp(Uint16 x, Uint16 y);
void GorpCeilThink(objtype *ob);
void GorpWalkThink(objtype *ob);
void GorpFallReact(objtype *ob);

#if (EPISODE != 2)
extern FARSTATE s_slimerstand;
extern FARSTATE s_slimerwalk1;
extern FARSTATE s_slimerwalk2;
extern FARSTATE s_slimerattack;
void SpawnSlimer(Uint16 x, Uint16 y);
void SlimerThink(objtype *ob);
#endif

#if (EPISODE == 3)
extern FARSTATE s_cyborgwalk1;
extern FARSTATE s_cyborgwalk2;
extern FARSTATE s_cyborgwalk3;
extern FARSTATE s_cyborgwalk4;
void SpawnCyborg(Uint16 x, Uint16 y);
void CyborgWalkThink(objtype *ob);
#endif

/*
=============================================================================

						 BM_ACT3 DEFINITIONS

=============================================================================
*/

#if (EPISODE == 1)
extern Sint16 skullmanstate;
#endif
extern Sint16 crusherstate;

extern FARSTATE s_bigexplosion1;
extern FARSTATE s_bigexplosion2;
extern FARSTATE s_bigexplosion3;
void ExplodeThink(objtype *ob);
void SpawnBigExplosion(Uint16 x, Uint16 y);

Sint16 SpawnEnemyShot(Uint16 x, Uint16 y, FARSTATE *state);

#if (EPISODE == 2)
extern FARSTATE s_antwalk1;
extern FARSTATE s_antwalk2;
extern FARSTATE s_antwalk3;
extern FARSTATE s_antwalk4;
extern FARSTATE s_antattack1;
extern FARSTATE s_antattack2;
extern FARSTATE s_antjump;
void SpawnAnt(Uint16 x, Uint16 y);
void AntWalkReact(objtype *ob);
void AntWalkThink(objtype *ob);
void AntJumpReact(objtype *ob);
#endif

extern FARSTATE s_tankdie;
extern FARSTATE s_tankstand;
extern FARSTATE s_tankwalk1;
extern FARSTATE s_tankwalk2;
extern FARSTATE s_tankattack1;
extern FARSTATE s_tankattack2;
extern FARSTATE s_smallrocketfly;
extern FARSTATE s_smallrocketfall;
extern FARSTATE s_tankshot;
extern FARSTATE s_tankshotexplode1;
extern FARSTATE s_tankshotexplode2;
extern FARSTATE s_smallfire1;
extern FARSTATE s_smallfire2;
extern FARSTATE s_smallfire3;
void SpawnTank(Uint16 x, Uint16 y);
void TankStandThink(objtype *ob);
void TankWalkThink(objtype *ob);
void TankReact(objtype *ob);
void TankAttack1Think(objtype *ob);
void TankAttack2Think(objtype *ob);
void TankshotReact(objtype *ob);
void SmallfireThink(objtype *ob);
void SmallrocketFlyReact(objtype *ob);
void SmallrocketFallReact(objtype *ob);

extern FARSTATE s_guardstand;
extern FARSTATE s_guardattack1;
extern FARSTATE s_guardattack2;
extern FARSTATE s_guardattack3;
extern FARSTATE s_guarddie;
void SpawnGuard(Uint16 x, Uint16 y);
void GuardStandThink(objtype *ob);
void GuardAttackThink(objtype *ob);

extern FARSTATE s_enemyshot;
extern FARSTATE s_explosion1;
extern FARSTATE s_explosion2;
#if (EPISODE == 1)
extern FARSTATE s_bossshot1;
extern FARSTATE s_bossshot2;
#elif (EPISODE == 3)
extern FARSTATE s_cyborgshot;
extern FARSTATE s_goliathshot1;
extern FARSTATE s_goliathshot2;
extern FARSTATE s_cainshot1;
extern FARSTATE s_cainshot2;
extern FARSTATE s_cainshot3;
#endif
void EnemyshotContact(objtype *ob, objtype *hit);
void EnemyshotReact(objtype *ob);

extern FARSTATE s_block;
extern FARSTATE s_blockfall;
void SpawnPushableBlock(Uint16 x, Uint16 y);
void BlockReact(objtype *ob);
void BlockFallingReact(objtype *ob);
void BlockContact(objtype *ob, objtype *hit);

extern FARSTATE s_brick;
extern FARSTATE s_brickfall;
void SpawnFallingBrick(Uint16 x, Uint16 y);
void BrickThink(objtype *ob);
void BrickFallingReact(objtype *ob);
void BrickContact(objtype *ob, objtype *hit);

extern FARSTATE s_crusher;
extern FARSTATE s_crushersink;
void SpawnCrusher(Uint16 x, Uint16 y);
void CrusherThink(objtype *ob);
void CrusherSinkThink(objtype *ob);
void CrusherSinkReact(objtype *ob);
void CrusherSinkContact(objtype *ob, objtype *hit);

extern FARSTATE s_bouncer1;
extern FARSTATE s_bouncer2;
extern FARSTATE s_bouncer3;
extern FARSTATE s_bouncer4;
extern FARSTATE s_bouncer_ground;
void SpawnBouncer(Uint16 x, Uint16 y);
void BouncerThink(objtype *ob);
void BouncerReact(objtype *ob);

extern FARSTATE s_acidmonster1;
extern FARSTATE s_acidmonster2;
extern FARSTATE s_acidmonster3;
extern FARSTATE s_acid1;
extern FARSTATE s_acid2;
extern FARSTATE s_acidfall;
extern FARSTATE s_acidland1;
extern FARSTATE s_acidland2;
extern FARSTATE s_acidland3;
extern FARSTATE s_acidland4;
void SpawnDrool(Uint16 x, Uint16 y);
void AcidmonsterThink(objtype *ob);
void SpawnAcid(objtype *ob);
void AcidThink(objtype *ob);
void AcidFallReact(objtype *ob);

#if (EPISODE != 1)

extern FARSTATE s_spikey1;
extern FARSTATE s_spikey2;
void SpawnSpikey(Uint16 x, Uint16 y);
void SpikeyThink(objtype *ob);

extern FARSTATE s_trashtruck1;
extern FARSTATE s_trashtruck2;
void SpawnTrashTruck(Uint16 x, Uint16 y);

#endif

extern FARSTATE s_turret1;
extern FARSTATE s_turretattack1;
extern FARSTATE s_turretattack2;
extern FARSTATE s_turretattack3;
void SpawnTurret(Uint16 x, Uint16 y);
void TurretThink(objtype *ob);
void TurretAttackThink(objtype *ob);

extern FARSTATE s_asteroid1;
extern FARSTATE s_asteroid2;
extern FARSTATE s_asteroid3;
extern FARSTATE s_asteroid4;
void SpawnAsteroid(Uint16 x, Uint16 y);

extern FARSTATE s_landmine1;
extern FARSTATE s_landmine2;
void SpawnLandmine(void); // REFKEEN: Args were ignored.
//void SpawnLandmine(Uint16 x, Uint16 y);
void LandmineContact(objtype *ob, objtype *hit);
void LandmineReact(objtype *ob);

#if (EPISODE == 1)
extern FARSTATE s_skullman1;
extern FARSTATE s_skullman2;
extern FARSTATE s_lefthand;
extern FARSTATE s_righthand;
extern FARSTATE s_lefthandfly;
extern FARSTATE s_righthandfly;
extern FARSTATE s_lefthandattack1;
extern FARSTATE s_lefthandattack2;
extern FARSTATE s_righthandattack1;
extern FARSTATE s_righthandattack2;
void SpawnSkullHand(Uint16 x, Uint16 y, Uint16 type);
void HandFlyReact(objtype *ob);
void LeftHandAttackReact(objtype *ob);
void RightHandAttackReact(objtype *ob);
void LeftHandThink(objtype *ob);
void RightHandThink(objtype *ob);
void LeftHandFlyThink(objtype *ob);
void RightHandFlyThink(objtype *ob);
void SpawnSkullman(Uint16 x, Uint16 y);
void SkullmanThink(objtype *ob);
#endif

#if (EPISODE != 2)
extern FARSTATE s_plasmashooter1;
extern FARSTATE s_plasmashooter2;
extern FARSTATE s_plasma1;
extern FARSTATE s_plasma2;
extern FARSTATE s_plasmaexplode1;
extern FARSTATE s_plasmaexplode2;
void SpawnPlasmashooter(Uint16 x, Uint16 y);
void PlasmashooterThink(objtype *ob);
void PlasmaReact(objtype *ob);
#endif

#if (EPISODE == 3)

extern FARSTATE s_goliathdie;
extern FARSTATE s_goliathstand;
extern FARSTATE s_goliathwalk1;
extern FARSTATE s_goliathwalk2;
extern FARSTATE s_goliath_unused;
extern FARSTATE s_goliathattack1;
extern FARSTATE s_goliathattack2;
void SpawnGoliath(Uint16 x, Uint16 y);
void GoliathStandThink(objtype *ob);
void GoliathWalkThink(objtype *ob);
void GoliathReact(objtype *ob);
void GoliathUNUSEDThink(objtype *ob);
void GoliathAttackThink(objtype *ob);

extern FARSTATE s_enemylandmine1;
extern FARSTATE s_enemylandmine2;
extern FARSTATE s_enemylandmine3;
extern FARSTATE s_barrel;
extern FARSTATE s_barrelfall;
void SpawnEnemyLandmine(Uint16 x, Uint16 y);
void SpawnBarrel(Uint16 x, Uint16 y);
void BarrelThink(objtype *ob);
void EnemyLandmineContact(objtype *ob, objtype *hit);
void BarrelContact(objtype *ob, objtype *hit);
void BarrelFallReact(objtype *ob);

extern FARSTATE s_grenadebotstand;
extern FARSTATE s_grenadebotattack1;
extern FARSTATE s_grenadebotattack2;
extern FARSTATE s_grenadebotattack3;
extern FARSTATE s_grenadebotdie;
extern FARSTATE s_plasmagrenade;
void SpawnGrenadeBot(Uint16 x, Uint16 y);
void GrenadebotStandThink(objtype *ob);
void GrenadebotAttackThink(objtype *ob);
void PlasmagrenadeContact(objtype *ob, objtype *hit);

extern FARSTATE s_cainwait;
extern FARSTATE s_cainstand;
extern FARSTATE s_caindead1;
extern FARSTATE s_caindead2;
extern FARSTATE s_cainattack1;
extern FARSTATE s_cainattack2;
extern FARSTATE s_cainattack3;
extern FARSTATE s_cainduckattack1;
extern FARSTATE s_cainduckattack2;
extern FARSTATE s_cainduckattack3;
extern FARSTATE s_cainduck;
extern FARSTATE s_cainexplode;
extern FARSTATE s_caintransform1;
extern FARSTATE s_caintransform2;
extern FARSTATE s_cainghost1;
extern FARSTATE s_cainghost2;
void SpawnCain(Uint16 x, Uint16 y);
void CainStandThink(objtype *ob);
void CainAttackThink(objtype *ob);
void CainTransformThink(objtype *ob);
void CainGhostReact(objtype *ob);
void CainDeadThink(objtype *ob);

#endif

extern FARSTATE s_beacon;
extern FARSTATE s_beacon1;
extern FARSTATE s_beacon2;
extern FARSTATE s_beacon3;
extern FARSTATE s_beacon4;
void SpawnBeacon(Uint16 x, Uint16 y);
void BeaconContact(objtype *ob, objtype *hit);


/*
=============================================================================

						 BSS SEGMENT CHEATING

=============================================================================
*/

// The Gold Medallion Software version was compiled with Borland C++ 3.1 and
// doesn't need this "cheat" to recreate the original BSS segment layout.
#ifndef GMS_VERSION

/*
I couldn't find names that would allow these variables to appear in the right
order when compiled with Borland C++ 2.0, so I grouped them together in a
struct to force them into the same order as in the original executables.

If the struct name didn't give it away, these variables all come from
BM_SNAKE.C
*/

#define __BSS_CHEAT__ snake_bss_vars
typedef struct {
	Uint32 leavepoletime;
	Uint16 invincibility;
	boolean upKeyDown;
	boolean leftKeyDown;
	boolean button0down;
	boolean rightKeyDown;
	boolean button1down;
	boolean downKeyDown;
#ifndef BETA
	boolean grenadeKeyDown;
#endif
	Uint16 colorseqnum;
	Sint16 doorx;
	Sint16 doory;
	Uint16 jumptime;
	Uint8 shuttlecooldown;
} __snakebssvarstype;

extern __snakebssvarstype __BSS_CHEAT__;

#define leavepoletime (__BSS_CHEAT__.leavepoletime)
#define invincibility (__BSS_CHEAT__.invincibility)
#define upKeyDown (__BSS_CHEAT__.upKeyDown)
#define leftKeyDown (__BSS_CHEAT__.leftKeyDown)
#define button0down (__BSS_CHEAT__.button0down)
#define rightKeyDown (__BSS_CHEAT__.rightKeyDown)
#define button1down (__BSS_CHEAT__.button1down)
#define downKeyDown (__BSS_CHEAT__.downKeyDown)
#ifndef BETA
#define grenadeKeyDown (__BSS_CHEAT__.grenadeKeyDown)
#endif
#define colorseqnum (__BSS_CHEAT__.colorseqnum)
#define doorx (__BSS_CHEAT__.doorx)
#define doory (__BSS_CHEAT__.doory)
#define jumptime (__BSS_CHEAT__.jumptime)
#define shuttlecooldown (__BSS_CHEAT__.shuttlecooldown)

/*
These #defines may cause problems if you try to use one of these names for a
local variable, but when you add new variables, you obviously won't be able to
recreate the original executables and therefore you don't need this BSS segment
layout "cheat" anyway.
*/
#endif	//!GMS_VERSION

REFKEEN_NS_E

#endif	//__BM_DEF_H__
