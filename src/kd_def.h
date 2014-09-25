/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// KD_DEF.H

#include "id_heads.h"
//#include <BIOS.H>

#define FRILLS	0			// Cut out frills for 360K - MIKE MAYNARD


/*
=============================================================================

						GLOBAL CONSTANTS

=============================================================================
*/

#define	MAXACTORS	MAXSPRITES

#define ACCGRAVITY	3
#define SPDMAXY		80

#define BLOCKSIZE	(8*PIXGLOBAL)		// for positioning starting actors

//
// movement scrolling edges
//
#define SCROLLEAST (TILEGLOBAL*11)
#define SCROLLWEST (TILEGLOBAL*9)
#define SCROLLSOUTH (TILEGLOBAL*8)
#define SCROLLNORTH (TILEGLOBAL*4)

#define CLIPMOVE	24					// speed to push out of a solid wall

#define GAMELEVELS	17

/*
=============================================================================

							 TYPES

=============================================================================
*/

typedef	enum	{notdone,resetgame,levelcomplete,warptolevel,died,victorious}
				exittype;

typedef	enum	{nothing,keenobj,powerobj,doorobj,
	bonusobj,broccoobj,tomatobj,carrotobj,celeryobj,asparobj,grapeobj,
	taterobj,cartobj,frenchyobj,melonobj,turnipobj,cauliobj,brusselobj,
	mushroomobj,squashobj,apelobj,peapodobj,peabrainobj,boobusobj,
	shotobj,inertobj}	classtype;

typedef struct
{
  id0_int_t 		leftshapenum,rightshapenum;
  enum		{step,slide,think,stepthink,slidethink} progress;
  id0_boolean_t	skippable;

  id0_boolean_t	pushtofloor;
  id0_int_t tictime;
  id0_int_t xmove;
  id0_int_t ymove;
  void (*think) ();
  void (*contact) ();
  void (*react) ();
  void *nextstate;
} __attribute__((__packed__)) statetype;


typedef	struct
{
	id0_unsigned_t	worldx,worldy;
	id0_boolean_t	leveldone[GAMELEVELS];
	id0_long_t	score,nextextra;
	id0_int_t		flowerpowers;
	id0_int_t		boobusbombs,bombsthislevel;
	id0_int_t		keys;
	id0_int_t		mapon;
	id0_int_t		lives;
	id0_int_t		difficulty;
} __attribute__((__packed__)) gametype;

// TODO? (CHOCO KEEN): Using unions for temporaries now
// (because we may wish to store pointers and these aren't 16-bit...)
//
// But this totally corrupts saved games ; And these depend on EXE layout...

typedef union
{
	id0_int_t val;
	void *ptr;
} objtempfield;


typedef struct	objstruct
{
	classtype	obclass;
	enum		{no,yes,allways,removable} active;
	id0_boolean_t		needtoreact,needtoclip;
	id0_unsigned_t	nothink;
	id0_unsigned_t	x,y;

	id0_int_t			xdir,ydir;
	id0_int_t			xmove,ymove;
	id0_int_t			xspeed,yspeed;

	id0_int_t			ticcount,ticadjust;
	statetype	*state;

	id0_unsigned_t	shapenum;

	id0_unsigned_t	left,top,right,bottom;	// hit rectangle
	id0_unsigned_t	midx;
	id0_unsigned_t	tileleft,tiletop,tileright,tilebottom;	// hit rect in tiles
	id0_unsigned_t	tilemidx;

	id0_int_t			hitnorth,hiteast,hitsouth,hitwest;	// wall numbers contacted

	// TODO (CHOCO KEEN): Use 16-bit integers again with some mechanism
	objtempfield			temp1,temp2,temp3,temp4;

	void		*sprite;

	struct	objstruct	*next,*prev;
} __attribute__((__packed__)) objtype;


/*
=============================================================================

					 KD_MAIN.C DEFINITIONS

=============================================================================
*/

extern	id0_char_t	str[80],str2[20];
extern	id0_boolean_t	singlestep,jumpcheat,godmode,tedlevel;
extern	id0_unsigned_t	tedlevelnum;

void	DebugMemory (void);
void	TestSprites(void);
id0_int_t		DebugKeys (void);
void	StartupId (void);
void	ShutdownId (void);
void	Quit (id0_char_t *error);
void	InitGame (void);
//void	main (void);


/*
=============================================================================

					  KD_DEMO.C DEFINITIONS

=============================================================================
*/

void	Finale (void);
void	GameOver (void);
void	DemoLoop (void);
void	StatusWindow (void);
void	NewGame (void);
void	TEDDeath (void);

id0_boolean_t	LoadGame (id0_int_t file);
id0_boolean_t	SaveGame (id0_int_t file);
void	ResetGame (void);

/*
=============================================================================

					  KD_PLAY.C DEFINITIONS

=============================================================================
*/

extern	gametype	gamestate;
extern	exittype	playstate;
extern	id0_boolean_t		button0held,button1held;
extern	id0_unsigned_t	originxtilemax,originytilemax;
extern	objtype		*new,*check,*player,*scoreobj;

extern	ControlInfo	c;

extern	objtype dummyobj;

extern	id0_char_t		*levelnames[21];

void	CheckKeys (void);
void	CalcInactivate (void);
void 	InitObjArray (void);
void 	GetNewObj (id0_boolean_t usedummy);
void	RemoveObj (objtype *gone);
void 	ScanInfoPlane (void);
void 	PatchWorldMap (void);
void 	MarkTileGraphics (void);
void 	FadeAndUnhook (void);
void 	SetupGameLevel (id0_boolean_t loadnow);
void 	ScrollScreen (void);
void 	MoveObjVert (objtype *ob, id0_int_t ymove);
void 	MoveObjHoriz (objtype *ob, id0_int_t xmove);
void 	GivePoints (id0_unsigned_t points);
void 	ClipToEnds (objtype *ob);
void 	ClipToEastWalls (objtype *ob);
void 	ClipToWestWalls (objtype *ob);
void 	ClipToWalls (objtype *ob);
void	ClipToSprite (objtype *push, objtype *solid, id0_boolean_t squish);
void	ClipToSpriteSide (objtype *push, objtype *solid);
id0_int_t 	DoActor (objtype *ob,id0_int_t tics);
void 	StateMachine (objtype *ob);
void 	NewState (objtype *ob,statetype *state);
void 	PlayLoop (void);
void 	GameLoop (void);

/*
=============================================================================

					  KD_KEEN.C DEFINITIONS

=============================================================================
*/

void	CalcSingleGravity (void);

void	ProjectileThink		(objtype *ob);
void	VelocityThink		(objtype *ob);
void	DrawReact			(objtype *ob);

void	SpawnScore (void);
void	FixScoreBox (void);
void	SpawnWorldKeen (id0_int_t tilex, id0_int_t tiley);
void	SpawnKeen (id0_int_t tilex, id0_int_t tiley, id0_int_t dir);

void 	KillKeen (void);

extern	id0_int_t	singlegravity;
extern	id0_unsigned_t	bounceangle[8][8];

extern	statetype s_keendie1;

/*
=============================================================================

					  KD_ACT1.C DEFINITIONS

=============================================================================
*/

void WalkReact (objtype *ob);

void 	DoGravity (objtype *ob);
void	AccelerateX (objtype *ob,id0_int_t dir,id0_int_t max);
void 	FrictionX (objtype *ob);

void	ProjectileThink		(objtype *ob);
void	VelocityThink		(objtype *ob);
void	DrawReact			(objtype *ob);
void	DrawReact2			(objtype *ob);
void	DrawReact3			(objtype *ob);
void	ChangeState (objtype *ob, statetype *state);

void	ChangeToFlower (objtype *ob);

void	SpawnBonus (id0_int_t tilex, id0_int_t tiley, id0_int_t type);
void	SpawnDoor (id0_int_t tilex, id0_int_t tiley);
void 	SpawnBrocco (id0_int_t tilex, id0_int_t tiley);
void 	SpawnTomat (id0_int_t tilex, id0_int_t tiley);
void 	SpawnCarrot (id0_int_t tilex, id0_int_t tiley);
void 	SpawnAspar (id0_int_t tilex, id0_int_t tiley);
void 	SpawnGrape (id0_int_t tilex, id0_int_t tiley);

extern	statetype s_doorraise;

extern	statetype s_bonus;
extern	statetype s_bonusrise;

extern	statetype s_broccosmash3;
extern	statetype s_broccosmash4;

extern	statetype s_grapefall;

/*
=============================================================================

					  KD_ACT2.C DEFINITIONS

=============================================================================
*/

void SpawnTater (id0_int_t tilex, id0_int_t tiley);
void SpawnCart (id0_int_t tilex, id0_int_t tiley);
void SpawnFrenchy (id0_int_t tilex, id0_int_t tiley);
void SpawnMelon (id0_int_t tilex, id0_int_t tiley,id0_int_t dir);
void SpawnSquasher (id0_int_t tilex, id0_int_t tiley);
void SpawnApel (id0_int_t tilex, id0_int_t tiley);
void SpawnPeaPod (id0_int_t tilex, id0_int_t tiley);
void SpawnPeaBrain (id0_int_t tilex, id0_int_t tiley);
void SpawnBoobus (id0_int_t tilex, id0_int_t tiley);

extern	statetype s_taterattack2;
extern	statetype s_squasherjump2;
extern	statetype s_boobusdie;

extern	statetype s_deathwait1;
extern	statetype s_deathwait2;
extern	statetype s_deathwait3;
extern	statetype s_deathboom1;
extern	statetype s_deathboom2;
