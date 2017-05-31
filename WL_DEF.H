//#define BETA
#define YEAR	1992
#define MONTH	9
#define DAY		30

#include "ID_HEADS.H"
#include <MATH.H>
#include <VALUES.H>

#include "WL_MENU.H"

#ifdef SPANISH
#include "SPANISH.H"
#else
#include "FOREIGN.H"
#endif

#ifdef SPEAR
#include "F_SPEAR.H"
#endif

/*
=============================================================================

							MACROS

=============================================================================
*/


#define COLORBORDER(color)		asm{mov	dx,STATUS_REGISTER_1;in al,dx;\
	mov dx,ATR_INDEX;mov al,ATR_OVERSCAN;out dx,al;mov al,color;out	dx,al;\
	mov	al,32;out dx,al};

#define MAPSPOT(x,y,plane)		(*(mapsegs[plane]+farmapylookup[y]+x))

#define SIGN(x) 	((x)>0?1:-1)
#define ABS(x) 		((int)(x)>0?(x):-(x))
#define LABS(x) 	((long)(x)>0?(x):-(x))

/*
=============================================================================

						 GLOBAL CONSTANTS

=============================================================================
*/

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define MAXACTORS		127				// max number of nazis, etc / map
#define MAXSTATS		200				// max number of lamps, bonus, etc
#define MAXDOORS		32				// max number of sliding doors
#define MAXWALLTILES	32				// max number of wall tiles
#else
#define MAXACTORS		150				// max number of nazis, etc / map
#define MAXSTATS		400				// max number of lamps, bonus, etc
#define MAXDOORS		64				// max number of sliding doors
#define MAXWALLTILES	64				// max number of wall tiles
#endif

//
// tile constants
//

#define	ICONARROWS		90
#define PUSHABLETILE	98
#define EXITTILE		99				// at end of castle
#define AREATILE		107				// first of NUMAREAS floor tiles
#define NUMAREAS		37
#define ELEVATORTILE	21
#define AMBUSHTILE		106
#define	ALTELEVATORTILE	107

#define NUMBERCHARS	9


//----------------

// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= 19920505L)
#define EXTRAPOINTS		10000
#elif (GAMEVER_WOLFREV <= 19920505L)
#define EXTRAPOINTS		20000
#else
#define EXTRAPOINTS		40000
#endif

#define PLAYERSPEED		3000
#define RUNSPEED   		6000

#define	SCREENSEG		0xa000

#define SCREENBWIDE		80

#define HEIGHTRATIO		0.50		// also defined in id_mm.c

#define BORDERCOLOR	3
#define FLASHCOLOR	5
#define FLASHTICS	4


#define PLAYERSIZE		MINDIST			// player radius
#define MINACTORDIST	0x10000l		// minimum dist from player center
										// to any actor center

#define NUMLATCHPICS	100


#define PI	3.141592657

#define GLOBAL1		(1l<<16)
#define TILEGLOBAL  GLOBAL1
#define PIXGLOBAL	(GLOBAL1/64)
#define TILESHIFT		16l
#define UNSIGNEDSHIFT	8

#define ANGLES		360					// must be divisable by 4
#define ANGLEQUAD	(ANGLES/4)
#define FINEANGLES	3600
#define ANG90		(FINEANGLES/4)
#define ANG180		(ANG90*2)
#define ANG270		(ANG90*3)
#define ANG360		(ANG90*4)
#define VANG90		(ANGLES/4)
#define VANG180		(VANG90*2)
#define VANG270		(VANG90*3)
#define VANG360		(VANG90*4)

#define MINDIST		(0x5800l)


#define	MAXSCALEHEIGHT	256				// largest scale on largest view

#define MAXVIEWWIDTH		320

#define MAPSIZE		64					// maps are 64*64 max
#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3


#define STATUSLINES		40

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
// Earlier alternatives to these are used for screenloc and freelatch in v1.0
#if (GAMEVER_WOLFREV > 19920505L)
#define SCREENSIZE		(SCREENBWIDE*208)
#define PAGE1START		0
#define PAGE2START		(SCREENSIZE)
#define PAGE3START		(SCREENSIZE*2u)
#define	FREESTART		(SCREENSIZE*3u)
#endif


#define PIXRADIUS		512

#define STARTAMMO		8


// object flag values

#define FL_SHOOTABLE	1
#define FL_BONUS		2
#define FL_NEVERMARK	4
#define FL_VISABLE		8
#define FL_ATTACKMODE	16
#define FL_FIRSTATTACK	32
#define FL_AMBUSH		64
#define FL_NONMARK		128


//
// sprite constants
//

enum	{
		SPR_DEMO,
		// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L) && (!defined GAMEVER_NOAH3D)
		SPR_DEATHCAM,
#endif
//
// static sprites
//
		SPR_STAT_0,SPR_STAT_1,SPR_STAT_2,SPR_STAT_3,
		SPR_STAT_4,SPR_STAT_5,SPR_STAT_6,SPR_STAT_7,

		SPR_STAT_8,SPR_STAT_9,SPR_STAT_10,SPR_STAT_11,
		SPR_STAT_12,SPR_STAT_13,SPR_STAT_14,SPR_STAT_15,

		SPR_STAT_16,SPR_STAT_17,SPR_STAT_18,SPR_STAT_19,
		SPR_STAT_20,SPR_STAT_21,SPR_STAT_22,SPR_STAT_23,

		SPR_STAT_24,SPR_STAT_25,SPR_STAT_26,SPR_STAT_27,
		SPR_STAT_28,SPR_STAT_29,SPR_STAT_30,SPR_STAT_31,

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SPR_STAT_32,SPR_STAT_33,SPR_STAT_34,SPR_STAT_35,
		SPR_STAT_36,

		SPR_SNOOZE_1,SPR_SNOOZE_2,SPR_SNOOZE_3,

		SPR_QUIZ,

		SPR_WATERMELON,SPR_WATERMELONEXP,

		SPR_GAMEOVER,

		SPR_ANIMALPROJ,SPR_CANTALOUPE,
		SPR_CANTALOUPEEXP,SPR_ANIMALPROJEXP,

		SPR_YOUWIN,
#else
		SPR_STAT_32,SPR_STAT_33,SPR_STAT_34,SPR_STAT_35,
		SPR_STAT_36,SPR_STAT_37,SPR_STAT_38,SPR_STAT_39,

		SPR_STAT_40,SPR_STAT_41,SPR_STAT_42,SPR_STAT_43,
		SPR_STAT_44,SPR_STAT_45,SPR_STAT_46,SPR_STAT_47,
#endif

#ifdef SPEAR
		SPR_STAT_48,SPR_STAT_49,SPR_STAT_50,SPR_STAT_51,
#endif

//
// guard
//
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		SPR_GRD_S_1,SPR_GRD_S_2,SPR_GRD_S_3,SPR_GRD_S_4,
		SPR_GRD_S_5,SPR_GRD_S_6,SPR_GRD_S_7,SPR_GRD_S_8,
#endif

		SPR_GRD_W1_1,SPR_GRD_W1_2,SPR_GRD_W1_3,SPR_GRD_W1_4,
		SPR_GRD_W1_5,SPR_GRD_W1_6,SPR_GRD_W1_7,SPR_GRD_W1_8,

		SPR_GRD_W2_1,SPR_GRD_W2_2,SPR_GRD_W2_3,SPR_GRD_W2_4,
		SPR_GRD_W2_5,SPR_GRD_W2_6,SPR_GRD_W2_7,SPR_GRD_W2_8,

		SPR_GRD_W3_1,SPR_GRD_W3_2,SPR_GRD_W3_3,SPR_GRD_W3_4,
		SPR_GRD_W3_5,SPR_GRD_W3_6,SPR_GRD_W3_7,SPR_GRD_W3_8,

		SPR_GRD_W4_1,SPR_GRD_W4_2,SPR_GRD_W4_3,SPR_GRD_W4_4,
		SPR_GRD_W4_5,SPR_GRD_W4_6,SPR_GRD_W4_7,SPR_GRD_W4_8,

		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		SPR_GRD_PAIN_1,SPR_GRD_DIE_1,SPR_GRD_DIE_2,SPR_GRD_DIE_3,
		SPR_GRD_PAIN_2,SPR_GRD_DEAD,
#endif

		SPR_GRD_SHOOT1,SPR_GRD_SHOOT2,SPR_GRD_SHOOT3,
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SPR_GRD_DEAD,SPR_GRD_PAIN,SPR_GRD_DIE_1,SPR_GRD_DIE_2,
		SPR_GRD_DEAD2,
#endif

//
// dogs
//
		SPR_DOG_W1_1,SPR_DOG_W1_2,SPR_DOG_W1_3,SPR_DOG_W1_4,
		SPR_DOG_W1_5,SPR_DOG_W1_6,SPR_DOG_W1_7,SPR_DOG_W1_8,

		SPR_DOG_W2_1,SPR_DOG_W2_2,SPR_DOG_W2_3,SPR_DOG_W2_4,
		SPR_DOG_W2_5,SPR_DOG_W2_6,SPR_DOG_W2_7,SPR_DOG_W2_8,

		SPR_DOG_W3_1,SPR_DOG_W3_2,SPR_DOG_W3_3,SPR_DOG_W3_4,
		SPR_DOG_W3_5,SPR_DOG_W3_6,SPR_DOG_W3_7,SPR_DOG_W3_8,

		SPR_DOG_W4_1,SPR_DOG_W4_2,SPR_DOG_W4_3,SPR_DOG_W4_4,
		SPR_DOG_W4_5,SPR_DOG_W4_6,SPR_DOG_W4_7,SPR_DOG_W4_8,

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SPR_DOG_JUMP1,SPR_DOG_JUMP2,SPR_DOG_JUMP3,SPR_DOG_JUMP4,
		SPR_DOG_DIE_1,SPR_DOG_DIE_2,SPR_DOG_DEAD,SPR_DOG_DEAD2,
#else
		SPR_DOG_DIE_1,SPR_DOG_DIE_2,SPR_DOG_DIE_3,SPR_DOG_DEAD,
		SPR_DOG_JUMP1,SPR_DOG_JUMP2,SPR_DOG_JUMP3,
#endif



		// *** S3DNA RESTORATION ***
		// Relocated and modified Officer sprite definitions
#ifdef GAMEVER_NOAH3D
//
// officer
//
		SPR_OFC_W1_1,SPR_OFC_W1_2,SPR_OFC_W1_3,SPR_OFC_W1_4,
		SPR_OFC_W1_5,SPR_OFC_W1_6,SPR_OFC_W1_7,SPR_OFC_W1_8,

		SPR_OFC_W2_1,SPR_OFC_W2_2,SPR_OFC_W2_3,SPR_OFC_W2_4,
		SPR_OFC_W2_5,SPR_OFC_W2_6,SPR_OFC_W2_7,SPR_OFC_W2_8,

		SPR_OFC_W3_1,SPR_OFC_W3_2,SPR_OFC_W3_3,SPR_OFC_W3_4,
		SPR_OFC_W3_5,SPR_OFC_W3_6,SPR_OFC_W3_7,SPR_OFC_W3_8,

		SPR_OFC_W4_1,SPR_OFC_W4_2,SPR_OFC_W4_3,SPR_OFC_W4_4,
		SPR_OFC_W4_5,SPR_OFC_W4_6,SPR_OFC_W4_7,SPR_OFC_W4_8,

		SPR_OFC_SHOOT1,SPR_OFC_SHOOT2,SPR_OFC_SHOOT3,

		SPR_OFC_DEAD,SPR_OFC_PAIN,SPR_OFC_DIE_1,SPR_OFC_DIE_2,
		SPR_OFC_DEAD2,
#endif



//
// ss
//
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		SPR_SS_S_1,SPR_SS_S_2,SPR_SS_S_3,SPR_SS_S_4,
		SPR_SS_S_5,SPR_SS_S_6,SPR_SS_S_7,SPR_SS_S_8,
#endif

		SPR_SS_W1_1,SPR_SS_W1_2,SPR_SS_W1_3,SPR_SS_W1_4,
		SPR_SS_W1_5,SPR_SS_W1_6,SPR_SS_W1_7,SPR_SS_W1_8,

		SPR_SS_W2_1,SPR_SS_W2_2,SPR_SS_W2_3,SPR_SS_W2_4,
		SPR_SS_W2_5,SPR_SS_W2_6,SPR_SS_W2_7,SPR_SS_W2_8,

		SPR_SS_W3_1,SPR_SS_W3_2,SPR_SS_W3_3,SPR_SS_W3_4,
		SPR_SS_W3_5,SPR_SS_W3_6,SPR_SS_W3_7,SPR_SS_W3_8,

		SPR_SS_W4_1,SPR_SS_W4_2,SPR_SS_W4_3,SPR_SS_W4_4,
		SPR_SS_W4_5,SPR_SS_W4_6,SPR_SS_W4_7,SPR_SS_W4_8,

		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		SPR_SS_PAIN_1,SPR_SS_DIE_1,SPR_SS_DIE_2,SPR_SS_DIE_3,
		SPR_SS_PAIN_2,SPR_SS_DEAD,
#endif

		SPR_SS_SHOOT1,SPR_SS_SHOOT2,SPR_SS_SHOOT3,
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SPR_SS_DEAD,SPR_SS_PAIN,SPR_SS_DIE_1,SPR_SS_DIE_2,
		SPR_SS_DEAD2,
#endif

//
// mutant
//
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		SPR_MUT_S_1,SPR_MUT_S_2,SPR_MUT_S_3,SPR_MUT_S_4,
		SPR_MUT_S_5,SPR_MUT_S_6,SPR_MUT_S_7,SPR_MUT_S_8,
#endif

		SPR_MUT_W1_1,SPR_MUT_W1_2,SPR_MUT_W1_3,SPR_MUT_W1_4,
		SPR_MUT_W1_5,SPR_MUT_W1_6,SPR_MUT_W1_7,SPR_MUT_W1_8,

		SPR_MUT_W2_1,SPR_MUT_W2_2,SPR_MUT_W2_3,SPR_MUT_W2_4,
		SPR_MUT_W2_5,SPR_MUT_W2_6,SPR_MUT_W2_7,SPR_MUT_W2_8,

		SPR_MUT_W3_1,SPR_MUT_W3_2,SPR_MUT_W3_3,SPR_MUT_W3_4,
		SPR_MUT_W3_5,SPR_MUT_W3_6,SPR_MUT_W3_7,SPR_MUT_W3_8,

		SPR_MUT_W4_1,SPR_MUT_W4_2,SPR_MUT_W4_3,SPR_MUT_W4_4,
		SPR_MUT_W4_5,SPR_MUT_W4_6,SPR_MUT_W4_7,SPR_MUT_W4_8,

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SPR_MUT_SHOOT1,SPR_MUT_SHOOT2,SPR_MUT_SHOOT3,

		SPR_MUT_DEAD,SPR_MUT_PAIN,SPR_MUT_DIE_1,SPR_MUT_DIE_2,
		SPR_MUT_DEAD2,
#else
		SPR_MUT_PAIN_1,SPR_MUT_DIE_1,SPR_MUT_DIE_2,SPR_MUT_DIE_3,
		SPR_MUT_PAIN_2,SPR_MUT_DIE_4,SPR_MUT_DEAD,

		SPR_MUT_SHOOT1,SPR_MUT_SHOOT2,SPR_MUT_SHOOT3,SPR_MUT_SHOOT4,
#endif

		// *** S3DNA RESTORATION ***
		// Relocated and modified Officer sprite definitions found above
#ifndef GAMEVER_NOAH3D
//
// officer
//
		SPR_OFC_S_1,SPR_OFC_S_2,SPR_OFC_S_3,SPR_OFC_S_4,
		SPR_OFC_S_5,SPR_OFC_S_6,SPR_OFC_S_7,SPR_OFC_S_8,

		SPR_OFC_W1_1,SPR_OFC_W1_2,SPR_OFC_W1_3,SPR_OFC_W1_4,
		SPR_OFC_W1_5,SPR_OFC_W1_6,SPR_OFC_W1_7,SPR_OFC_W1_8,

		SPR_OFC_W2_1,SPR_OFC_W2_2,SPR_OFC_W2_3,SPR_OFC_W2_4,
		SPR_OFC_W2_5,SPR_OFC_W2_6,SPR_OFC_W2_7,SPR_OFC_W2_8,

		SPR_OFC_W3_1,SPR_OFC_W3_2,SPR_OFC_W3_3,SPR_OFC_W3_4,
		SPR_OFC_W3_5,SPR_OFC_W3_6,SPR_OFC_W3_7,SPR_OFC_W3_8,

		SPR_OFC_W4_1,SPR_OFC_W4_2,SPR_OFC_W4_3,SPR_OFC_W4_4,
		SPR_OFC_W4_5,SPR_OFC_W4_6,SPR_OFC_W4_7,SPR_OFC_W4_8,

		SPR_OFC_PAIN_1,SPR_OFC_DIE_1,SPR_OFC_DIE_2,SPR_OFC_DIE_3,
		SPR_OFC_PAIN_2,SPR_OFC_DIE_4,SPR_OFC_DEAD,

		SPR_OFC_SHOOT1,SPR_OFC_SHOOT2,SPR_OFC_SHOOT3,
#endif

		// *** S3DNA RESTORATION ***
		// Some sprite definitions shufflings and edits
#ifdef GAMEVER_NOAH3D
//
// hans
//
		SPR_BOSS_W1,SPR_BOSS_W2,SPR_BOSS_W3,SPR_BOSS_W4,
		SPR_BOSS_DIE1,SPR_BOSS_DIE2,SPR_BOSS_DEAD,SPR_BOSS_DEAD2,
		SPR_BOSS_SHOOT1,SPR_BOSS_SHOOT2,SPR_BOSS_SHOOT3,

//
// gretel
//
		SPR_GRETEL_W1,SPR_GRETEL_W2,SPR_GRETEL_W3,SPR_GRETEL_W4,
		SPR_GRETEL_DIE1,SPR_GRETEL_DIE2,SPR_GRETEL_DEAD,SPR_GRETEL_DEAD2,
		SPR_GRETEL_SHOOT1,SPR_GRETEL_SHOOT2,SPR_GRETEL_SHOOT3,

//
// giftmacher
//
		SPR_GIFT_W1,SPR_GIFT_W2,SPR_GIFT_W3,SPR_GIFT_W4,
		SPR_GIFT_DIE1,SPR_GIFT_DIE2,SPR_GIFT_DEAD,SPR_GIFT_DEAD2,
		SPR_GIFT_SHOOT1,SPR_GIFT_SHOOT2,SPR_GIFT_SHOOT3,SPR_GIFT_SHOOT4,

//
// schabbs
//
		SPR_SCHABB_W1,SPR_SCHABB_W2,SPR_SCHABB_W3,SPR_SCHABB_W4,
		SPR_SCHABB_DIE1,SPR_SCHABB_DIE2,SPR_SCHABB_DEAD,SPR_SCHABB_DEAD2,
		SPR_SCHABB_SHOOT1,SPR_SCHABB_SHOOT2,

//
// fat face
//
		SPR_FAT_W1,SPR_FAT_W2,SPR_FAT_W3,SPR_FAT_W4,
		SPR_FAT_DIE1,SPR_FAT_DIE2,SPR_FAT_DEAD,SPR_FAT_DEAD2,
		SPR_FAT_SHOOT1,SPR_FAT_SHOOT2,SPR_FAT_SHOOT3,SPR_FAT_SHOOT4,

//
// hitler
//
		SPR_MECHA_W1,SPR_MECHA_W2,SPR_MECHA_W3,SPR_MECHA_W4,
		SPR_MECHA_DIE1,SPR_MECHA_DIE2,SPR_MECHA_DIE3,SPR_MECHA_DEAD,
		SPR_MECHA_SHOOT1,SPR_MECHA_SHOOT2,SPR_MECHA_SHOOT3,

		SPR_HITLER_W1,SPR_HITLER_W2,SPR_HITLER_W3,SPR_HITLER_W4,
		SPR_HITLER_DIE1,SPR_HITLER_DIE2,SPR_HITLER_DEAD,SPR_HITLER_DEAD2,
		SPR_HITLER_SHOOT1,SPR_HITLER_SHOOT2,SPR_HITLER_SHOOT3,
#else
#ifndef SPEAR
//
// ghosts
//
		SPR_BLINKY_W1,SPR_BLINKY_W2,SPR_PINKY_W1,SPR_PINKY_W2,
		SPR_CLYDE_W1,SPR_CLYDE_W2,SPR_INKY_W1,SPR_INKY_W2,

//
// hans
//
		SPR_BOSS_W1,SPR_BOSS_W2,SPR_BOSS_W3,SPR_BOSS_W4,
		SPR_BOSS_SHOOT1,SPR_BOSS_SHOOT2,SPR_BOSS_SHOOT3,SPR_BOSS_DEAD,

		SPR_BOSS_DIE1,SPR_BOSS_DIE2,SPR_BOSS_DIE3,

//
// schabbs
//
		SPR_SCHABB_W1,SPR_SCHABB_W2,SPR_SCHABB_W3,SPR_SCHABB_W4,
		SPR_SCHABB_SHOOT1,SPR_SCHABB_SHOOT2,

		SPR_SCHABB_DIE1,SPR_SCHABB_DIE2,SPR_SCHABB_DIE3,SPR_SCHABB_DEAD,
		SPR_HYPO1,SPR_HYPO2,SPR_HYPO3,SPR_HYPO4,

//
// fake
//
		SPR_FAKE_W1,SPR_FAKE_W2,SPR_FAKE_W3,SPR_FAKE_W4,
		SPR_FAKE_SHOOT,SPR_FIRE1,SPR_FIRE2,

		SPR_FAKE_DIE1,SPR_FAKE_DIE2,SPR_FAKE_DIE3,SPR_FAKE_DIE4,
		SPR_FAKE_DIE5,SPR_FAKE_DEAD,

//
// hitler
//
		SPR_MECHA_W1,SPR_MECHA_W2,SPR_MECHA_W3,SPR_MECHA_W4,
		SPR_MECHA_SHOOT1,SPR_MECHA_SHOOT2,SPR_MECHA_SHOOT3,SPR_MECHA_DEAD,

		SPR_MECHA_DIE1,SPR_MECHA_DIE2,SPR_MECHA_DIE3,

		SPR_HITLER_W1,SPR_HITLER_W2,SPR_HITLER_W3,SPR_HITLER_W4,
		SPR_HITLER_SHOOT1,SPR_HITLER_SHOOT2,SPR_HITLER_SHOOT3,SPR_HITLER_DEAD,

		SPR_HITLER_DIE1,SPR_HITLER_DIE2,SPR_HITLER_DIE3,SPR_HITLER_DIE4,
		SPR_HITLER_DIE5,SPR_HITLER_DIE6,SPR_HITLER_DIE7,

		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
		// Looks required for correct numbers, guessing this as the name
#if (GAMEVER_WOLFREV <= 19920601L)
		SPR_HITLER_DIE8,
#endif

		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L)
//
// giftmacher
//
		SPR_GIFT_W1,SPR_GIFT_W2,SPR_GIFT_W3,SPR_GIFT_W4,
		SPR_GIFT_SHOOT1,SPR_GIFT_SHOOT2,

		SPR_GIFT_DIE1,SPR_GIFT_DIE2,SPR_GIFT_DIE3,SPR_GIFT_DEAD,
#endif
#endif
//
// Rocket, smoke and small explosion
//
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L)
		SPR_ROCKET_1,SPR_ROCKET_2,SPR_ROCKET_3,SPR_ROCKET_4,
		SPR_ROCKET_5,SPR_ROCKET_6,SPR_ROCKET_7,SPR_ROCKET_8,

		SPR_SMOKE_1,SPR_SMOKE_2,SPR_SMOKE_3,SPR_SMOKE_4,
#endif
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920601L)
		SPR_BOOM_1,SPR_BOOM_2,SPR_BOOM_3,
#endif

//
// Angel of Death's DeathSparks(tm)
//
#ifdef SPEAR
		SPR_HROCKET_1,SPR_HROCKET_2,SPR_HROCKET_3,SPR_HROCKET_4,
		SPR_HROCKET_5,SPR_HROCKET_6,SPR_HROCKET_7,SPR_HROCKET_8,

		SPR_HSMOKE_1,SPR_HSMOKE_2,SPR_HSMOKE_3,SPR_HSMOKE_4,
		SPR_HBOOM_1,SPR_HBOOM_2,SPR_HBOOM_3,

		SPR_SPARK1,SPR_SPARK2,SPR_SPARK3,SPR_SPARK4,
#endif

#ifndef SPEAR
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L)
//
// gretel
//
		SPR_GRETEL_W1,SPR_GRETEL_W2,SPR_GRETEL_W3,SPR_GRETEL_W4,
		SPR_GRETEL_SHOOT1,SPR_GRETEL_SHOOT2,SPR_GRETEL_SHOOT3,SPR_GRETEL_DEAD,

		SPR_GRETEL_DIE1,SPR_GRETEL_DIE2,SPR_GRETEL_DIE3,

//
// fat face
//
		SPR_FAT_W1,SPR_FAT_W2,SPR_FAT_W3,SPR_FAT_W4,
		SPR_FAT_SHOOT1,SPR_FAT_SHOOT2,SPR_FAT_SHOOT3,SPR_FAT_SHOOT4,

		SPR_FAT_DIE1,SPR_FAT_DIE2,SPR_FAT_DIE3,SPR_FAT_DEAD,
#endif // GAMEVER_WOLFREV <= 19920505L

//
// bj
//
		SPR_BJ_W1,SPR_BJ_W2,SPR_BJ_W3,SPR_BJ_W4,
		SPR_BJ_JUMP1,SPR_BJ_JUMP2,SPR_BJ_JUMP3,SPR_BJ_JUMP4,
#else
//
// THESE ARE FOR 'SPEAR OF DESTINY'
//

//
// Trans Grosse
//
		SPR_TRANS_W1,SPR_TRANS_W2,SPR_TRANS_W3,SPR_TRANS_W4,
		SPR_TRANS_SHOOT1,SPR_TRANS_SHOOT2,SPR_TRANS_SHOOT3,SPR_TRANS_DEAD,

		SPR_TRANS_DIE1,SPR_TRANS_DIE2,SPR_TRANS_DIE3,

//
// Wilhelm
//
		SPR_WILL_W1,SPR_WILL_W2,SPR_WILL_W3,SPR_WILL_W4,
		SPR_WILL_SHOOT1,SPR_WILL_SHOOT2,SPR_WILL_SHOOT3,SPR_WILL_SHOOT4,

		SPR_WILL_DIE1,SPR_WILL_DIE2,SPR_WILL_DIE3,SPR_WILL_DEAD,

//
// UberMutant
//
		SPR_UBER_W1,SPR_UBER_W2,SPR_UBER_W3,SPR_UBER_W4,
		SPR_UBER_SHOOT1,SPR_UBER_SHOOT2,SPR_UBER_SHOOT3,SPR_UBER_SHOOT4,

		SPR_UBER_DIE1,SPR_UBER_DIE2,SPR_UBER_DIE3,SPR_UBER_DIE4,
		SPR_UBER_DEAD,

//
// Death Knight
//
		SPR_DEATH_W1,SPR_DEATH_W2,SPR_DEATH_W3,SPR_DEATH_W4,
		SPR_DEATH_SHOOT1,SPR_DEATH_SHOOT2,SPR_DEATH_SHOOT3,SPR_DEATH_SHOOT4,

		SPR_DEATH_DIE1,SPR_DEATH_DIE2,SPR_DEATH_DIE3,SPR_DEATH_DIE4,
		SPR_DEATH_DIE5,SPR_DEATH_DIE6,SPR_DEATH_DEAD,

//
// Ghost
//
		SPR_SPECTRE_W1,SPR_SPECTRE_W2,SPR_SPECTRE_W3,SPR_SPECTRE_W4,
		SPR_SPECTRE_F1,SPR_SPECTRE_F2,SPR_SPECTRE_F3,SPR_SPECTRE_F4,

//
// Angel of Death
//
		SPR_ANGEL_W1,SPR_ANGEL_W2,SPR_ANGEL_W3,SPR_ANGEL_W4,
		SPR_ANGEL_SHOOT1,SPR_ANGEL_SHOOT2,SPR_ANGEL_TIRED1,SPR_ANGEL_TIRED2,

		SPR_ANGEL_DIE1,SPR_ANGEL_DIE2,SPR_ANGEL_DIE3,SPR_ANGEL_DIE4,
		SPR_ANGEL_DIE5,SPR_ANGEL_DIE6,SPR_ANGEL_DIE7,SPR_ANGEL_DEAD,

#endif
#endif // GAMEVER_NOAH3D

//
// player attack frames
//
		SPR_KNIFEREADY,SPR_KNIFEATK1,SPR_KNIFEATK2,SPR_KNIFEATK3,
		SPR_KNIFEATK4,

		SPR_PISTOLREADY,SPR_PISTOLATK1,SPR_PISTOLATK2,SPR_PISTOLATK3,
		SPR_PISTOLATK4,

		SPR_MACHINEGUNREADY,SPR_MACHINEGUNATK1,SPR_MACHINEGUNATK2,MACHINEGUNATK3,
		SPR_MACHINEGUNATK4,

		SPR_CHAINREADY,SPR_CHAINATK1,SPR_CHAINATK2,SPR_CHAINATK3,
		SPR_CHAINATK4,

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SPR_CANTAREADY,SPR_CANTAATK1,SPR_CANTAATK2,SPR_CANTAATK3,
		SPR_CANTAATK4,

		SPR_WATERREADY,SPR_WATERATK1,SPR_WATERATK2,SPR_WATERATK3,
		SPR_WATERATK4,

		SPR_LAST
#endif
		};


/*
=============================================================================

						   GLOBAL TYPES

=============================================================================
*/

typedef long fixed;

typedef enum {
	di_north,
	di_east,
	di_south,
	di_west
} controldir_t;

typedef enum {
	dr_normal,
	dr_lock1,
	dr_lock2,
	dr_lock3,
	dr_lock4,
	dr_elevator
} door_t;

typedef enum {
	ac_badobject = -1,
	ac_no,
	ac_yes,
	ac_allways
} activetype;

typedef enum {
	nothing,
	playerobj,
	inertobj,
	guardobj,
	officerobj,
	ssobj,
	dogobj,
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	mutantobj,
#endif
	bossobj,
	schabbobj,
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	gretelobj,
	giftobj,
	fatobj,
	mechahitlerobj,
	realhitlerobj,
	rocketobj,
	needleobj,
	flameobj,
	missileobj
#else
	fakeobj,
	mechahitlerobj,
	mutantobj,
	needleobj,
	fireobj,
	bjobj,
	ghostobj,
	realhitlerobj,
	gretelobj,
	giftobj,
	fatobj,
	rocketobj
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
	,
	spectreobj,
	angelobj,
	transobj,
	uberobj,
	willobj,
	deathobj,
	hrocketobj,
	sparkobj
#endif
#endif // *** S3DNA RESTORATION ***
} classtype;

typedef enum {
	dressing,
	block,
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	bo_gibs,
#endif
	bo_alpo,
	bo_firstaid,
	bo_key1,
	bo_key2,
	bo_key3,
	bo_key4,
	bo_cross,
	bo_chalice,
	bo_bible,
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
	bo_crown,
#endif
	bo_clip,
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	bo_clip2 = bo_clip,
#else
	bo_clip2,
#endif
	bo_machinegun,
	bo_chaingun,
	bo_food,
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
	bo_fullheal
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
	,
	bo_25clip,
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	bo_chest,
	bo_bandolier,
	bo_flamethrower,
	bo_gascan,
	bo_launcher,
	bo_missiles,
	bo_map,
	bo_exit,
	bo_secret
#else
	bo_spear
#endif
#endif // GAMEVER_WOLFREV > 19920312L
#endif
} stat_t;

typedef enum {
	east,
	northeast,
	north,
	northwest,
	west,
	southwest,
	south,
	southeast,
	nodir
} dirtype;


// *** APOGEE + ALPHA + S3DNA VERSIONS RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if (GAMEVER_WOLFREV <= 19920312L)
#define NUMENEMIES		5
#elif (GAMEVER_WOLFREV <= 19920505L)
#define NUMENEMIES		13
#elif (GAMEVER_WOLFREV <= 19920610L)
#define NUMENEMIES		16
#else
#define NUMENEMIES		22
#endif
#endif
typedef enum {
	en_guard,
	en_officer,
	en_ss,
	en_dog,
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	en_mutant,
#endif
	en_boss,
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
	en_schabbs,
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	en_gretel,
	en_gift,
	en_fat,
	en_hitler
#else
	en_fake,
	en_hitler,
	en_mutant,
	en_blinky,
	en_clyde,
	en_pinky,
	en_inky,
	en_gretel,
	en_gift,
	en_fat
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
	,
	en_spectre,
	en_angel,
	en_trans,
	en_uber,
	en_will,
	en_death
#endif
#endif // GAMEVER_NOAH3D
#endif // GAMEVER_WOLFREV > 19920312L

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	,
	NUMENEMIES
#endif
} enemy_t;


typedef struct	statestruct
{
	boolean	rotate;
	int		shapenum;			// a shapenum of -1 means get from ob->temp1
	int		tictime;
	void	(*think) (),(*action) ();
	struct	statestruct	*next;
} statetype;


//---------------------
//
// trivial actor structure
//
//---------------------

typedef struct statstruct
{
	byte	tilex,tiley;
	byte	*visspot;
	int		shapenum;			// if shapenum == -1 the obj has been removed
	byte	flags;
	byte	itemnumber;
} statobj_t;


//---------------------
//
// door actor structure
//
//---------------------

typedef struct doorstruct
{
	byte	tilex,tiley;
	boolean	vertical;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	boolean	seen;
#endif
	byte	lock;
	enum	{dr_open,dr_closed,dr_opening,dr_closing}	action;
	int		ticcount;
} doorobj_t;


//--------------------
//
// thinking actor structure
//
//--------------------

typedef struct objstruct
{
	activetype	active;
	int			ticcount;
	classtype	obclass;
	statetype	*state;

	byte		flags;				//	FL_SHOOTABLE, etc

	long		distance;			// if negative, wait for that door to open
	dirtype		dir;

	fixed 		x,y;
	unsigned	tilex,tiley;
	byte		areanumber;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	byte		snore;		// 2.6 number. Upper is frame, Lower is tics*2
#endif

	int	 		viewx;
	unsigned	viewheight;
	fixed		transx,transy;		// in global coord

	int 		angle;
	int			hitpoints;
	long		speed;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	int			temp2;
#else
	int			temp1,temp2,temp3;
#endif
	struct		objstruct	*next,*prev;
} objtype;


// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define NUMBUTTONS	10
#else
#define NUMBUTTONS	8
#endif
enum	{
	bt_nobutton=-1,
	bt_attack=0,
	bt_strafe,
	bt_run,
	bt_use,
	bt_readyknife,
	bt_readypistol,
	bt_readymachinegun,
	bt_readychaingun
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	,
	bt_readycantaloupe,
	bt_readywatermelon
#endif
};


	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define NUMWEAPONS	7
#else
#define NUMWEAPONS	5
#endif
typedef enum	{
	wp_knife,
	wp_pistol,
	wp_machinegun,
	wp_chaingun
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	,
	wp_flamethrower,
	wp_missile
#endif
} weapontype;


typedef enum	{
	gd_baby,
	gd_easy,
	gd_medium,
	gd_hard
};

//---------------
//
// gamestate structure
//
//---------------

typedef	struct
{
	int			difficulty;
	int			mapon;
	long		oldscore,score,nextextra;
	int			lives;
	int			health;
	int			ammo;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	int			treasure;
	int			maxammo;
	int			gas;
	int			missiles;
	boolean			machinegun;
	boolean			chaingun;
	boolean			missile;
	boolean			flamethrower;
	boolean			automap;
#endif
	int			keys;
	weapontype		bestweapon,weapon,chosenweapon;

	int			faceframe;
	int			attackframe,attackcount,weaponframe;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	int			secretcount,treasurecount,killcount,
#else
	int			episode,secretcount,treasurecount,killcount,
#endif
				secrettotal,treasuretotal,killtotal;
	long		TimeCount;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L)
	long		killx,killy;
#endif
	boolean		victoryflag;		// set during victory animations
} gametype;


typedef	enum	{
	ex_stillplaying,
	ex_completed,
	ex_died,
	ex_warped,
	ex_resetgame,
	ex_loadedgame,
	ex_victorious,
	ex_abort,
	ex_demodone
	// *** SHAREWARE V1.0+V1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920601L)
	,
	ex_secretlevel
#endif
} exit_t;


/*
=============================================================================

						 WL_MAIN DEFINITIONS

=============================================================================
*/

extern	boolean		MS_CheckParm (char far *string);

extern	char		str[80],str2[20];
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= 19920610L)
extern	unsigned			tedlevelnum;
#else
extern	int			tedlevelnum;
#endif
extern	boolean		tedlevel;
extern	boolean		nospr;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	boolean		noquestions, nofloors;
extern	int		questionnum;
#endif
extern	boolean		IsA386;

extern	byte far	*scalermemory;

extern	fixed		focallength;
extern	unsigned	viewangles;
extern	unsigned	screenofs;
extern	int		    viewwidth;
extern	int			viewheight;
extern	int			centerx;
extern	int			shootdelta;

extern	int			dirangle[9];

// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
extern	boolean         startgame,loadedgame
#if (GAMEVER_WOLFREV > 19920610L) && (!defined GAMEVER_NOAH3D)
	,virtualreality
#endif
	;
extern	int		mouseadjustment;
//
// math tables
//
extern	int			pixelangle[MAXVIEWWIDTH];
extern	long		far finetangent[FINEANGLES/4];
extern	fixed 		far sintable[],far *costable;

//
// derived constants
//
extern	fixed 	scale,maxslope;
extern	long	heightnumerator;
extern	int		minheightdiv;

extern	char	configname[13];



void		HelpScreens (void);
void		OrderingInfo (void);
void		TEDDeath(void);
void		Quit (char *error);
void 		CalcProjection (long focal);
boolean		SetViewSize (unsigned width, unsigned height);
// *** ALPHA VERSION RESTORATION ***
#ifdef GAMEVER_ALPHA
void		NewGame (int difficulty);
#else
void		NewGame (int difficulty,int episode);
#endif
void 		NewViewSize (int width);
boolean 	LoadTheGame(int file,int x,int y);
boolean		SaveTheGame(int file,int x,int y);
void 		ShowViewSize (int width);
void		ShutdownId (void);


/*
=============================================================================

						 WL_GAME DEFINITIONS

=============================================================================
*/


// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L) && (GAMEVER_WOLFREV <= 19920505L)
extern	boolean		ingame,fizzlein,screensplit;
// *** S3DNA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
extern	boolean		ingame;
extern	int		endtics,fizzlein;
#else
extern	boolean		ingame,fizzlein;
#endif
extern	unsigned	latchpics[NUMLATCHPICS];
extern	gametype	gamestate;
extern	int			doornum;

extern	char		demoname[13];

// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
extern	long		spearx,speary;
extern	unsigned	spearangle;
extern	boolean		spearflag;
#endif


void 	DrawPlayBorder (void);
void 	ScanInfoPlane (void);
void	SetupGameLevel (void);
void 	NormalScreen (void);
void 	DrawPlayScreen (void);
void 	FizzleOut (void);
void 	GameLoop (void);
// *** ALPHA RESTORATION ***/
// FIXME
#if (GAMEVER_WOLFREV <= 19920312L)
#define ClearMemory()	// FIXME
#else
// JAB
void ClearMemory (void);
#endif
void PlayDemo (int demonumber);
void RecordDemo (void);
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L)
void DrawAllPlayBorder (void);
#endif
void	DrawHighScores(void);
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920505L)
void DrawAllPlayBorderSides (void);
#endif

// *** ALPHA RESTORATION ***/
// Quick hacks
#if (GAMEVER_WOLFREV <= 19920312L)
#define	PlaySoundLocTile(s,tx,ty)	SD_PlaySound(s)
#define	PlaySoundLocActor(s,ob)		SD_PlaySound(s)
#define UpdateSoundLoc()		// FIXME
#else
// JAB
#define	PlaySoundLocTile(s,tx,ty)	PlaySoundLocGlobal(s,(((long)(tx) << TILESHIFT) + (1L << (TILESHIFT - 1))),(((long)ty << TILESHIFT) + (1L << (TILESHIFT - 1))))
#define	PlaySoundLocActor(s,ob)		PlaySoundLocGlobal(s,(ob)->x,(ob)->y)
void	PlaySoundLocGlobal(word s,fixed gx,fixed gy);
void UpdateSoundLoc(void);
#endif


/*
=============================================================================

						 WL_PLAY DEFINITIONS

=============================================================================
*/

#ifdef SPEAR
extern	long		funnyticount;		// FOR FUNNY BJ FACE
#endif

extern	exit_t		playstate;

// *** PRE-V1.4 APOGEE RESTORATION *** - There was apparently some unused variable here
#if (GAMEVER_WOLFREV <= 19920610L)
extern	boolean unusedplayvar;
#endif
extern	boolean		madenoise;

extern	objtype 	objlist[MAXACTORS],*new,*obj,*player,*lastobj,
					*objfreelist,*killerobj;
extern	statobj_t	statobjlist[MAXSTATS],*laststatobj;
extern	doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;

extern	unsigned	farmapylookup[MAPSIZE];
extern	byte		*nearmapylookup[MAPSIZE];

extern	byte		tilemap[MAPSIZE][MAPSIZE];	// wall values only
extern	byte		spotvis[MAPSIZE][MAPSIZE];
extern	objtype		*actorat[MAPSIZE][MAPSIZE];

// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Not sure why but looks correct
#if (GAMEVER_WOLFREV <= 19920505L)
#define UPDATESIZE			((UPDATEWIDE+7)*UPDATEHIGH)
#else
#define UPDATESIZE			(UPDATEWIDE*UPDATEHIGH)
#endif
extern	byte		update[UPDATESIZE];

extern	boolean		singlestep,godmode,noclip;
extern	int			extravbls;

//
// control info
//
extern	boolean		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
extern	int			joystickport;
extern	int			dirscan[4];
extern	int			buttonscan[NUMBUTTONS];
extern	int			buttonmouse[4];
extern	int			buttonjoy[4];

extern	boolean		buttonheld[NUMBUTTONS];

extern	int			viewsize;

//
// curent user input
//
extern	int			controlx,controly;		// range from -100 to 100
extern	boolean		buttonstate[NUMBUTTONS];

extern	boolean		demorecord,demoplayback;
extern	char		far *demoptr, far *lastdemoptr;
extern	memptr		demobuffer;



void	InitRedShifts (void);
void 	FinishPaletteShifts (void);

void	CenterWindow(word w,word h);
void 	InitActorList (void);
void 	GetNewActor (void);
void 	RemoveObj (objtype *gone);
void 	PollControls (void);
void 	StopMusic(void);
void 	StartMusic(void);
void	PlayLoop (void);
void StartDamageFlash (int damage);
void StartBonusFlash (void);
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void UpdatePaletteShifts (void);
#endif

/*
=============================================================================

							WL_INTER

=============================================================================
*/

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	int	FloorTile[];
#endif

void IntroScreen (void);
void PreloadGraphics(void);
void LevelCompleted (void);
void	CheckHighScore (long score,word other);
void Victory (void);
// *** ALPHA RESTORATION ***
// FIXME HACK
#if (GAMEVER_WOLFREV <= 19920312L)
#define ClearSplitVWB()
#else
void ClearSplitVWB (void);
#endif
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void Briefing (int mission, boolean credits);
#endif


/*
=============================================================================

							WL_DEBUG

=============================================================================
*/

int DebugKeys (void);
void PicturePause (void);


/*
=============================================================================

						 WL_DRAW DEFINITIONS

=============================================================================
*/

extern	unsigned screenloc[3];
extern	unsigned freelatch;

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= 19920505L)
extern	int	screenpage;
#endif

extern	long 	lasttimecount;
extern	long 	frameon;
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
extern	boolean	fizzlein;
#endif

extern	unsigned	wallheight[MAXVIEWWIDTH];

extern	fixed	tileglobal;
extern	fixed	focallength;
extern	fixed	mindist;

//
// math tables
//
extern	int			pixelangle[MAXVIEWWIDTH];
extern	long		far finetangent[FINEANGLES/4];
extern	fixed 		far sintable[],far *costable;

//
// derived constants
//
extern	fixed 	scale;
extern	long	heightnumerator,mindist;

//
// refresh variables
//
extern	fixed	viewx,viewy;			// the focal point
extern	int		viewangle;
extern	fixed	viewsin,viewcos;

extern	long		postsource;
extern	unsigned	postx;
extern	unsigned	postwidth;


extern	int		horizwall[],vertwall[];

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
extern	unsigned	pwallpos;
#endif


fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
int		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);
void  FarScalePost (void);
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void WallRefresh (void);
#endif

/*
=============================================================================

						 WL_STATE DEFINITIONS

=============================================================================
*/
#define TURNTICS	10
#define SPDPATROL	512
#define SPDDOG		1500


extern	dirtype opposite[9];
extern	dirtype diagonal[9][9];


void	InitHitRect (objtype *ob, unsigned radius);
void	SpawnNewObj (unsigned tilex, unsigned tiley, statetype *state);
void	NewState (objtype *ob, statetype *state);

boolean TryWalk (objtype *ob);
void 	SelectChaseDir (objtype *ob);
void 	SelectDodgeDir (objtype *ob);
void	SelectRunDir (objtype *ob);
void	MoveObj (objtype *ob, long move);
boolean SightPlayer (objtype *ob);

void	KillActor (objtype *ob);
void	DamageActor (objtype *ob, unsigned damage);

boolean CheckLine (objtype *ob);
boolean	CheckSight (objtype *ob);


/*
=============================================================================

						 WL_SCALE DEFINITIONS

=============================================================================
*/


#define COMPSCALECODESTART	(65*4)		// offset to start of code in comp scaler

typedef struct
{
	unsigned	codeofs[65];
	unsigned	width[65];
	byte		code[];
}	t_compscale;

typedef struct
{
	unsigned	leftpix,rightpix;
	unsigned	dataofs[64];
// table data after dataofs[rightpix-leftpix+1]
}	t_compshape;


extern	t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
extern	long			fullscalefarcall[MAXSCALEHEIGHT+1];

extern	byte		bitmasks1[8][8];
extern	byte		bitmasks2[8][8];
extern	unsigned	wordmasks[8][8];

extern	byte		mapmasks1[4][8];
extern	byte		mapmasks2[4][8];
extern	byte		mapmasks3[4][8];

extern	int			maxscale,maxscaleshl2;

// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
extern	boolean	insetupscaling;
#endif

void SetupScaling (int maxscaleheight);
void ScaleShape (int xcenter, int shapenum, unsigned height);
void SimpleScaleShape (int xcenter, int shapenum, unsigned height);

/*
=============================================================================

						 WL_AGENT DEFINITIONS

=============================================================================
*/

//
// player state info
//
extern	boolean		running;
extern	long		thrustspeed;
extern	unsigned	plux,pluy;		// player coordinates scaled to unsigned

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
extern	int			anglefrac;
#endif
extern	int			facecount;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern int	MapEpisode[];
extern int	MapLevel[];
#endif

void	SpawnPlayer (int tilex, int tiley, int dir);
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void StatusDrawPic (unsigned x, unsigned y, unsigned picnum);
void	DrawTreasure (void);
#endif
void 	DrawFace (void);
void	DrawHealth (void);
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
void	TakeDamage (int points,objtype *attacker);
#endif
void	HealSelf (int points);
void	DrawLevel (void);
void	DrawLives (void);
void	GiveExtraMan (void);
void	DrawScore (void);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= 19920312L)
void	GivePoints (int points);
#else
void	GivePoints (long points);
#endif
// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
void	DrawWeapon (void);
#endif
void	DrawKeys (void);
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void	GiveTreasure (int treasure);
#endif
void	GiveWeapon (int weapon);
void	DrawAmmo (void);
void	GiveAmmo (int ammo);
void	GiveKey (int key);
void	GetBonus (statobj_t *check);

void	Thrust (int angle, long speed);

/*
=============================================================================

						 WL_ACT1 DEFINITIONS

=============================================================================
*/

extern	doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;
extern	int			doornum;

extern	unsigned	doorposition[MAXDOORS],pwallstate;

extern	byte		far areaconnect[NUMAREAS][NUMAREAS];

extern	boolean		areabyplayer[NUMAREAS];

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
extern unsigned	pwallstate;
extern unsigned	pwallpos;			// amount a pushable wall has been moved (0-63)
extern unsigned	pwallx,pwally;
extern int			pwalldir;
#endif


// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
void InitDoorList (void);
void InitStaticList (void);
void SpawnStatic (int tilex, int tiley, int type);
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void SpawnExit (int tilex, int tiley, boolean secret);
#endif
void SpawnDoor (int tilex, int tiley, boolean vertical, int lock);
void MoveDoors (void);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
void MovePWalls (void);
#endif
void OpenDoor (int door);
void PlaceItemType (int itemtype, int tilex, int tiley);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
void PushWall (int checkx, int checky, int dir);
#endif
void OperateDoor (int door);
void InitAreas (void);
#endif

/*
=============================================================================

						 WL_ACT2 DEFINITIONS

=============================================================================
*/

#define s_nakedbody s_static10

extern	statetype s_grddie1;
extern	statetype s_dogdie1;
extern	statetype s_ofcdie1;
extern	statetype s_mutdie1;
extern	statetype s_ssdie1;
extern	statetype s_bossdie1;
extern	statetype s_schabbdie1;
extern	statetype s_fakedie1;
extern	statetype s_mechadie1;
extern	statetype s_hitlerdie1;
extern	statetype s_greteldie1;
extern	statetype s_giftdie1;
extern	statetype s_fatdie1;

extern	statetype s_spectredie1;
extern	statetype s_angeldie1;
extern	statetype s_transdie0;
extern	statetype s_uberdie0;
extern	statetype s_willdie1;
extern	statetype s_deathdie1;


extern	statetype s_grdchase1;
extern	statetype s_dogchase1;
extern	statetype s_ofcchase1;
extern	statetype s_sschase1;
extern	statetype s_mutchase1;
extern	statetype s_bosschase1;
extern	statetype s_schabbchase1;
extern	statetype s_fakechase1;
extern	statetype s_mechachase1;
extern	statetype s_gretelchase1;
extern	statetype s_giftchase1;
extern	statetype s_fatchase1;

extern	statetype s_spectrechase1;
extern	statetype s_angelchase1;
extern	statetype s_transchase1;
extern	statetype s_uberchase1;
extern	statetype s_willchase1;
extern	statetype s_deathchase1;

extern	statetype s_blinkychase1;
extern	statetype s_hitlerchase1;

// *** PRE-V1.4 APOGEE RESTORATION *** - Disabling pain1 states
extern	statetype s_grdpain;
#if (GAMEVER_WOLFREV > 19920610L)
extern	statetype s_grdpain1;
#endif
extern	statetype s_ofcpain;
#if (GAMEVER_WOLFREV > 19920610L)
extern	statetype s_ofcpain1;
#endif
extern	statetype s_sspain;
#if (GAMEVER_WOLFREV > 19920610L)
extern	statetype s_sspain1;
#endif
extern	statetype s_mutpain;
#if (GAMEVER_WOLFREV > 19920610L)
extern	statetype s_mutpain1;
#endif

extern	statetype s_deathcam;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern	statetype s_gameover;

extern	statetype s_flame;
extern	statetype s_fexplode1;
extern	statetype s_fexplode2;
extern	statetype s_fexplode3;

extern	statetype s_missile;
extern	statetype s_mexplode1;
extern	statetype s_mexplode2;
extern	statetype s_mexplode3;
#endif

extern	statetype s_schabbdeathcam2;
extern	statetype s_hitlerdeathcam2;
extern	statetype s_giftdeathcam2;
extern	statetype s_fatdeathcam2;



// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void SpawnStand (enemy_t which, int tilex, int tiley, int dir, boolean ambush);
void A_DeathScream (objtype *ob);
#else
void SpawnStand (enemy_t which, int tilex, int tiley, int dir);
void SpawnPatrol (enemy_t which, int tilex, int tiley, int dir);
#endif
void KillActor (objtype *ob);

void	US_ControlPanel(byte);

// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > 19920610L)
void SpawnDeadGuard (int tilex, int tiley);
void SpawnBoss (int tilex, int tiley);
void SpawnGretel (int tilex, int tiley);
void SpawnTrans (int tilex, int tiley);
void SpawnUber (int tilex, int tiley);
void SpawnWill (int tilex, int tiley);
void SpawnDeath (int tilex, int tiley);
void SpawnAngel (int tilex, int tiley);
void SpawnSpectre (int tilex, int tiley);
void SpawnGhosts (int which, int tilex, int tiley);
void SpawnSchabbs (int tilex, int tiley);
void SpawnGift (int tilex, int tiley);
void SpawnFat (int tilex, int tiley);
void SpawnFakeHitler (int tilex, int tiley);
void SpawnHitler (int tilex, int tiley);
#endif

/*
=============================================================================

						 WL_TEXT DEFINITIONS

=============================================================================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > 19920312L)
extern	char	helpfilename[],endfilename[];
#endif

extern	void	HelpScreens(void);
extern	void	EndText(void);

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
/*
=============================================================================

						 WL_MAP DEFINITIONS

=============================================================================
*/

void AutoMap (void);

/*
=============================================================================

						 WL_QUIZ DEFINITIONS

=============================================================================
*/

int AskQuestion (int question);
/*
=============================================================================

						 WOLFHACK DEFINITIONS

=============================================================================
*/

void LoadFloorTiles (int tile);
void SetPlaneViewSize (void);
void DrawPlanes (void);
#endif
