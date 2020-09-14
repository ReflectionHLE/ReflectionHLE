/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020 NY00123
 *
 * This file is part of Wolfenstein 3D.
 *
 * Wolfenstein 3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Wolfenstein 3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an addendum, you can further use the Wolfenstein 3D Source Code under
 * the terms of the limited use software license agreement for Wolfenstein 3D.
 * See id-wolf3d.txt for these terms.
 */

// WL_AGENT.C

#include "wl_def.h"
//#pragma hdrstop


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define MAXMOUSETURN	10


#define MOVESCALE		150l
#define BACKMOVESCALE	100l
#define ANGLESCALE		20

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/



//
// player state info
//
// *** ALPHA RESTORATION ***
// Need to define this (and declare in header) for recreation of memory layout
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
id0_boolean_t		someUnusedAgentVar;
#endif
id0_boolean_t		running;
id0_long_t		thrustspeed;

id0_unsigned_t	plux,pluy;			// player coordinates scaled to id0_unsigned_t

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_int_t			anglefrac;
id0_int_t			gotgatgun;	// JR
#endif

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
objtype		*LastAttacker;
#endif

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


void	T_Player (objtype *ob);
void	T_Attack (objtype *ob);

statetype s_player = {false,0,0,T_Player,NULL,NULL};
statetype s_attack = {false,0,0,T_Attack,NULL,NULL};


id0_long_t	playerxmove,playerymove;

// *** S3DNA RESTORATION ***
struct atkinf
{
	id0_char_t	tics,attack,frame;		// attack is 1 for gun, 2 for knife
}
#ifdef GAMEVER_NOAH3D
attackinfo[6][14] =
#else
attackinfo[4][14] =
#endif

{
{ {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
{ {6,0,1},{6,1,2},{6,0,3},{8,-1,4} },
#else
{ {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },
#endif
{ {6,0,1},{6,1,2},{6,3,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,4,3},{6,-1,4} },
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
{ {7,0,1},{7,5,2},{7,5,3},{7,-1,4} },
{ {6,0,1},{6,0,2},{6,6,3},{6,-1,4} },
#endif
};


id0_int_t	strafeangle[9] = {0,90,180,270,45,135,225,315,0};

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_int_t	MapEpisode[]   = {1,1,1,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5,5,6,6,6,6,6,6,6,7};
id0_int_t	MapLevel[] = {1,2,3,1,2,3,4,1,2,3,4,5,1,2,3,4,5,1,2,3,4,5,6,1,2,3,4,5,6,7,1};
#endif

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
void DrawWeapon (void);
#endif
void GiveWeapon (id0_int_t weapon);
void	GiveAmmo (id0_int_t ammo);

//===========================================================================

//----------

void Attack (void);
void Use (void);
void Search (objtype *ob);
void SelectWeapon (void);
void SelectItem (void);

//----------

id0_boolean_t TryMove (objtype *ob);
void T_Player (objtype *ob);

void ClipMove (objtype *ob, id0_long_t xmove, id0_long_t ymove);

/*
=============================================================================

						CONTROL STUFF

=============================================================================
*/

/*
======================
=
= CheckWeaponChange
=
= Keys 1-4 change weapons
=
======================
*/

void CheckWeaponChange (void)
{
	id0_int_t	i,buttons;

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	if (!gamestate.ammo)		// must use knife with no ammo
		return;
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	for (i=wp_knife ; i<=wp_missile ; i++)
#else
	for (i=wp_knife ; i<=gamestate.bestweapon ; i++)
#endif
		if (buttonstate[bt_readyknife+i-wp_knife])
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			switch (i)
			{
			case wp_knife:
				gamestate.weapon = gamestate.chosenweapon = i;
				DrawAmmo ();
				break;
			case wp_chaingun:
				if (!gamestate.chaingun)
					break;
			case wp_machinegun:
				if (!gamestate.machinegun)
					break;
			case wp_pistol:
				if (gamestate.ammo)
				{
					gamestate.weapon = gamestate.chosenweapon = i;
					DrawAmmo ();
					return;
				}
				break;
			case wp_flamethrower:
				if (gamestate.flamethrower && gamestate.gas)
				{
					gamestate.weapon = gamestate.chosenweapon = i;
					DrawAmmo ();
					return;
				}
				break;
			case wp_missile:
				if (gamestate.missile && gamestate.missiles)
				{
					gamestate.weapon = gamestate.chosenweapon = i;
					DrawAmmo ();
					return;
				}
				break;
			}
#else
		{
			gamestate.weapon = gamestate.chosenweapon = i;
			DrawWeapon ();
			return;
		}
#endif
}


/*
=======================
=
= ControlMovement
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
=======================
*/

void ControlMovement (objtype *ob)
{
	id0_long_t	oldx,oldy;
	id0_int_t		angle,maxxmove;
	id0_int_t		angleunits;
	id0_long_t	speed;

	thrustspeed = 0;

	oldx = player->x;
	oldy = player->y;

//
// side to side move
//
	if (buttonstate[bt_strafe])
	{
	//
	// strafing
	//
	//
		if (controlx > 0)
		{
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,controlx*MOVESCALE);	// move to left
		}
		else if (controlx < 0)
		{
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,-controlx*MOVESCALE);	// move to right
		}
	}
	else
	{
	//
	// not strafing
	//
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		ob->angle -= controlx/ANGLESCALE;
#else
		anglefrac += controlx;
		angleunits = anglefrac/ANGLESCALE;
		anglefrac -= angleunits*ANGLESCALE;
		ob->angle -= angleunits;
#endif

		if (ob->angle >= ANGLES)
			ob->angle -= ANGLES;
		if (ob->angle < 0)
			ob->angle += ANGLES;

	}

//
// forward/backwards move
//
	if (controly < 0)
	{
		Thrust (ob->angle,-controly*MOVESCALE);	// move forwards
	}
	else if (controly > 0)
	{
		angle = ob->angle + ANGLES/2;
		if (angle >= ANGLES)
			angle -= ANGLES;
		Thrust (angle,controly*BACKMOVESCALE);		// move backwards
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (gamestate.victoryflag)		// watching the BJ actor
		return;
#endif

//
// calculate total move
//
	playerxmove = player->x - oldx;
	playerymove = player->y - oldy;
}

/*
=============================================================================

					STATUS WINDOW STUFF

=============================================================================
*/

// *** SHAREWARE V1.0 APOGEE RESTORATION *** - StatusDrawPic wasn't used,
// instead there was just a call to LatchDrawPic everywhere, more-or-less,
// along with a few bits of fiddling with bufferofs.

#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
// HACK for restoration of v1.0
#define StatusDrawPic LatchDrawPic

#else
/*
==================
=
= StatusDrawPic
=
==================
*/

void StatusDrawPic (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t picnum)
{
	id0_unsigned_t	temp;

	temp = bufferofs;
	bufferofs = 0;

	bufferofs = PAGE1START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);
	bufferofs = PAGE2START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);
	bufferofs = PAGE3START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);

	bufferofs = temp;
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10


// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void	LatchNumber (id0_int_t x, id0_int_t y, id0_int_t width, id0_long_t number);

void DrawTreasure ()
{
	LatchNumber (35,16,2,gamestate.treasure);
}
#endif

/*
==================
=
= DrawFace
=
==================
*/

void DrawFace (void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	temp;

	temp = bufferofs;
	bufferofs = 0;
#endif
	if (gamestate.health)
	{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (godmode)
			StatusDrawPic (19,4,GODMODEFACE1PIC+gamestate.faceframe);
		else
			StatusDrawPic (19,4,FACE1APIC+3*((100-gamestate.health)/16)+gamestate.faceframe);
#else
		#ifdef SPEAR
		if (godmode)
			StatusDrawPic (17,4,GODMODEFACE1PIC+gamestate.faceframe);
		else
		#endif
		StatusDrawPic (17,4,FACE1APIC+3*((100-gamestate.health)/16)+gamestate.faceframe);
#endif
	}
	else
	{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	   StatusDrawPic (19,4,FACE8APIC);
#else
	// ALPHA RESTORATION
#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
	 if (LastAttacker->obclass == needleobj)
	   StatusDrawPic (17,4,MUTANTBJPIC);
	 else
#endif
	   StatusDrawPic (17,4,FACE8APIC);
#endif
	}
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	bufferofs = temp;
#endif
}


/*
===============
=
= UpdateFace
=
= Calls draw face if time to change
=
===============
*/

#define FACETICS	70

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
static	id0_int_t	facecount;
#else
id0_int_t	facecount;
#endif

void	UpdateFace (void)
{

	// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (SD_SoundPlaying() == GETGATLINGSND)
	  return;
#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (SD_SoundPlaying() == D_BONUSSND)
#endif
		facecount += tics;
	if (facecount > US_RndT())
	{
		gamestate.faceframe = (US_RndT()>>6);
		if (gamestate.faceframe==3)
			gamestate.faceframe = 1;

		facecount = 0;
		DrawFace ();
	}
}



/*
===============
=
= LatchNumber
=
= right justifies and pads with blanks
=
===============
*/

void	LatchNumber (id0_int_t x, id0_int_t y, id0_int_t width, id0_long_t number)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	temp;

#endif
	id0_unsigned_t	length,c;
	id0_char_t	str[20];

	BE_Cross_ltoa_dec (number,str);

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	temp = bufferofs;
	bufferofs = 0;

#endif
	length = strlen (str);

	while (length<width)
	{
		StatusDrawPic (x,y,N_BLANKPIC);
		x++;
		width--;
	}

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	c = 0;
#else
	c= length <= width ? 0 : length-width;
#endif

	while (c<length)
	{
		StatusDrawPic (x,y,str[c]-'0'+ N_0PIC);
		x++;
		c++;
	}
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	bufferofs = temp;
#endif
}


/*
===============
=
= DrawHealth
=
===============
*/

void	DrawHealth (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	LatchNumber (23,16,3,gamestate.health);
#else
	LatchNumber (21,16,3,gamestate.health);
#endif
}


/*
===============
=
= TakeDamage
=
===============
*/

void	TakeDamage (id0_int_t points,objtype *attacker)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	LastAttacker = attacker;
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (gamestate.victoryflag)
		return;
#endif
	// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	switch (gamestate.difficulty)
	{
	case gd_baby: points>>=2;
		break;
	case gd_easy: points>>=1;
		break;
	case gd_medium: points-=(points>>2);
		break;
	}
	if (!points && (gamestate.difficulty != gd_baby))
		points++;
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (gamestate.difficulty==gd_baby)
	  points>>=2;
#endif

	if (!godmode)
		gamestate.health -= points;

	if (gamestate.health<=0)
	{
		gamestate.health = 0;
		playstate = ex_died;
		killerobj = attacker;
	}

	StartDamageFlash (points);
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	SD_PlaySound (TAKEDAMAGESND);
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	gotgatgun=0;
#endif

	DrawHealth ();
	DrawFace ();

	//
	// MAKE BJ'S EYES BUG IF MAJOR DAMAGE!
	//
	#ifdef SPEAR
	if (points > 30 && gamestate.health!=0 && !godmode)
	{
		StatusDrawPic (17,4,BJOUCHPIC);
		facecount = 0;
	}
	#endif

}


/*
===============
=
= HealSelf
=
===============
*/

void	HealSelf (id0_int_t points)
{
	gamestate.health += points;
	if (gamestate.health>100)
		gamestate.health = 100;

	DrawHealth ();
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	gotgatgun = 0;	// JR
#endif
	DrawFace ();
}


//===========================================================================


/*
===============
=
= DrawLevel
=
===============
*/

void	DrawLevel (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_int_t 	level,episode;

	if (gamestate.mapon >= 31)
		Quit ("DrawLevel(): Bad level number!\n");

	episode = MapEpisode[gamestate.mapon];
	level = MapLevel[gamestate.mapon];
	LatchNumber (2,16,1,episode);
	LatchNumber (4,16,1,level);
#else
#ifdef SPEAR
	if (gamestate.mapon == 20)
		LatchNumber (2,16,2,18);
	else
#endif
	LatchNumber (2,16,2,gamestate.mapon+1);
#endif
}

//===========================================================================


/*
===============
=
= DrawLives
=
===============
*/

void	DrawLives (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	LatchNumber (16,16,1,gamestate.lives);
#else
	LatchNumber (14,16,1,gamestate.lives);
#endif
}


/*
===============
=
= GiveExtraMan
=
===============
*/

void	GiveExtraMan (void)
{
	if (gamestate.lives<9)
		gamestate.lives++;
	DrawLives ();
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	SD_PlaySound (HEALTH1SND);
#else
	SD_PlaySound (BONUS1UPSND);
#endif
}

//===========================================================================

/*
===============
=
= DrawScore
=
===============
*/

void	DrawScore (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	LatchNumber (7,16,7,gamestate.score);
#else
	LatchNumber (6,16,6,gamestate.score);
#endif
}

/*
===============
=
= GivePoints
=
===============
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void	GivePoints (id0_int_t points)
#else
void	GivePoints (id0_long_t points)
#endif
{
	gamestate.score += points;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (gamestate.score >= gamestate.nextextra)
#else
	while (gamestate.score >= gamestate.nextextra)
#endif
	{
		gamestate.nextextra += EXTRAPOINTS;
		GiveExtraMan ();
	}
	DrawScore ();
}

//===========================================================================

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
==================
=
= DrawWeapon
=
==================
*/

void DrawWeapon (void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	temp;

	temp = bufferofs;
	bufferofs = 0;
#endif
	StatusDrawPic (32,8,KNIFEPIC+gamestate.weapon);
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	bufferofs = temp;
#endif
}
#endif


/*
==================
=
= DrawKeys
=
==================
*/

void DrawKeys (void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	temp;

	temp = bufferofs;
	bufferofs = 0;
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (gamestate.keys & 1)
		StatusDrawPic (32,4,GOLDKEYPIC);
	else
		StatusDrawPic (32,4,NOKEYPIC);

	if (gamestate.keys & 2)
		StatusDrawPic (32,20,SILVERKEYPIC);
	else
		StatusDrawPic (32,20,NOKEYPIC);
#else
	if (gamestate.keys & 1)
		StatusDrawPic (30,4,GOLDKEYPIC);
	else
		StatusDrawPic (30,4,NOKEYPIC);

	if (gamestate.keys & 2)
		StatusDrawPic (30,20,SILVERKEYPIC);
	else
		StatusDrawPic (30,20,NOKEYPIC);
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	bufferofs = temp;
#endif
}



// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
/*
==================
=
= GiveTreasure
=
==================
*/

void GiveTreasure (id0_int_t treasure)
{
	gamestate.treasure += treasure;
	while (gamestate.treasure >= 50)
	{
		gamestate.treasure -= 50;
		GiveExtraMan ();
	}
	gamestate.treasurecount++;
	DrawTreasure ();
}


#endif

/*
==================
=
= GiveWeapon
=
==================
*/

void GiveWeapon (id0_int_t weapon)
{
	GiveAmmo (6);

	if (gamestate.bestweapon<weapon)
		gamestate.bestweapon = gamestate.weapon
		= gamestate.chosenweapon = weapon;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	DrawAmmo ();
#else
	DrawWeapon ();
#endif
}


//===========================================================================

/*
===============
=
= DrawAmmo
=
===============
*/

void	DrawAmmo (void)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_int_t ammo;
	switch (gamestate.weapon)
	{
	case wp_pistol:
	case wp_machinegun:
	case wp_chaingun:
		ammo = gamestate.ammo;
		break;
	case wp_flamethrower:
		ammo = gamestate.gas;
		break;
	case wp_missile:
		ammo = gamestate.missiles;
		break;
	default:
		ammo = 0;
	}
	LatchNumber (28,16,3,
		ammo);
#else
	LatchNumber (27,16,2,gamestate.ammo);
#endif
}


/*
===============
=
= GiveAmmo
=
===============
*/

void	GiveAmmo (id0_int_t ammo)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	gamestate.ammo += ammo;
	if (gamestate.ammo > gamestate.maxammo)
		gamestate.ammo = gamestate.maxammo;
	if (gamestate.weapon == wp_knife)
		if (!gamestate.attackframe &&
			((gamestate.chosenweapon == wp_chaingun) || (gamestate.chosenweapon == wp_machinegun) || (gamestate.chosenweapon == wp_pistol)))
			gamestate.weapon = gamestate.chosenweapon;
#else
	if (!gamestate.ammo)				// knife was out
	{
		if (!gamestate.attackframe)
		{
			gamestate.weapon = gamestate.chosenweapon;
			DrawWeapon ();
		}
	}
	gamestate.ammo += ammo;
	if (gamestate.ammo > 99)
		gamestate.ammo = 99;
#endif
	DrawAmmo ();
}

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
/*
===============
=
= GiveGas
=
===============
*/

void	GiveGas (id0_int_t ammo)
{
	gamestate.gas += ammo;
	if (gamestate.gas > 99)
		gamestate.gas = 99;
	if ((gamestate.weapon == wp_knife) && gamestate.flamethrower)
	{
		if (!gamestate.attackframe)
		{
			gamestate.weapon = wp_flamethrower;
		}
		else
		{
			gamestate.chosenweapon = wp_flamethrower;
		}
	}
	DrawAmmo ();
}


/*
===============
=
= GiveMissile
=
===============
*/

void	GiveMissile (id0_int_t ammo)
{
	gamestate.missiles += ammo;
	if (gamestate.missiles > 99)
		gamestate.missiles = 99;
	if ((gamestate.weapon == wp_knife) && gamestate.missile)
	{
		if (!gamestate.attackframe)
		{
			gamestate.weapon = wp_missile;
		}
		else
		{
			gamestate.chosenweapon = wp_missile;
		}
	}
	DrawAmmo ();
}
#endif

//===========================================================================

/*
==================
=
= GiveKey
=
==================
*/

void GiveKey (id0_int_t key)
{
	gamestate.keys |= (1<<key);
	DrawKeys ();
}



/*
=============================================================================

							MOVEMENT

=============================================================================
*/


/*
===================
=
= GetBonus
=
===================
*/
void GetBonus (statobj_t *check)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	temp;
#endif
	switch (check->itemnumber)
	{
	case	bo_firstaid:
		if (gamestate.health == 100)
			return;

		SD_PlaySound (HEALTH2SND);
		HealSelf (25);
		break;

	case	bo_key1:
	case	bo_key2:
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	case	bo_key3:
	case	bo_key4:
#endif
		GiveKey (check->itemnumber - bo_key1);
		SD_PlaySound (GETKEYSND);
		break;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	case	bo_cross:
		SD_PlaySound (D_BONUSSND);
		GiveTreasure (1);
		break;
	case	bo_chalice:
		SD_PlaySound (D_BONUSSND);
		GiveTreasure (1);
		break;
	case	bo_bible:
		SD_PlaySound (D_BONUSSND);
		GiveTreasure (1);
		break;
	case	bo_crown:
		SD_PlaySound (D_BONUSSND);
		GiveTreasure (1);
		break;
	case	bo_chest:
		SD_PlaySound (D_BONUSSND);
		GiveTreasure (1);
		break;
#else
	case	bo_cross:
		SD_PlaySound (BONUS1SND);
		GivePoints (100);
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		gamestate.treasurecount++;
#endif
		break;
	case	bo_chalice:
		SD_PlaySound (BONUS2SND);
		GivePoints (500);
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		gamestate.treasurecount++;
#endif
		break;
	case	bo_bible:
		SD_PlaySound (BONUS3SND);
		GivePoints (1000);
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		gamestate.treasurecount++;
#endif
		break;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case	bo_crown:
		SD_PlaySound (BONUS4SND);
		GivePoints (5000);
		gamestate.treasurecount++;
		break;
#endif
#endif // GAMEVER_NOAH3D

	case	bo_clip:
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (gamestate.ammo >= gamestate.maxammo)
#else
		if (gamestate.ammo == 99)
#endif
			return;

		SD_PlaySound (GETAMMOSND);
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		GiveAmmo (5);
#else
		GiveAmmo (8);
#endif
		break;
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	case	bo_clip2:
		if (gamestate.ammo == 99)
			return;

		SD_PlaySound (GETAMMOSND);
		GiveAmmo (4);
		break;
#endif

		// *** S3DNA RESTORATION ***
#if (defined SPEAR) || (defined GAMEVER_NOAH3D)
//#ifdef SPEAR
	case	bo_25clip:
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (gamestate.ammo >= gamestate.maxammo)
#else
		if (gamestate.ammo == 99)
#endif
		  return;

		SD_PlaySound (GETAMMOBOXSND);
		GiveAmmo (25);
		break;
#endif

	case	bo_machinegun:
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		gamestate.machinegun = 1;
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		SD_PlaySound (GETGATLINGSND);
#else
		SD_PlaySound (GETMACHINESND);
#endif
		GiveWeapon (wp_machinegun);
		break;
	case	bo_chaingun:
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		gamestate.chaingun = 1;
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		SD_PlaySound (GETMACHINESND);
#else
		SD_PlaySound (GETGATLINGSND);
#endif
		GiveWeapon (wp_chaingun);

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		temp = bufferofs;
		bufferofs = 0;

#endif
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		StatusDrawPic (19,4,GOTGATLINGPIC);
#else
		StatusDrawPic (17,4,GOTGATLINGPIC);
#endif
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		bufferofs = temp;

#endif
		facecount = 0;
		gotgatgun = 1;
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
		break;

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case	bo_fullheal:
		SD_PlaySound (BONUS1UPSND);
		HealSelf (99);
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		GiveAmmo (25);
#endif
		GiveExtraMan ();
		gamestate.treasurecount++;
		break;
#endif

	case	bo_food:
		if (gamestate.health == 100)
			return;

		SD_PlaySound (HEALTH1SND);
		HealSelf (10);
		break;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	case	bo_alpo:
		if (demoplayback)
		{
			HealSelf (4); break;
		}
		if ((gamestate.health >= 100) && (gamestate.ammo >= gamestate.maxammo))
			return;

		if (noquestions)
		{
			HealSelf (4);
			StartBonusFlash ();
		}
		else
		{
			if (AskQuestion (questionnum))
				StartBonusFlash ();
			if (++questionnum >= 99)
				questionnum = 0;
		}
		check->shapenum = -1;
		return;

	case	bo_bandolier:
		gamestate.maxammo += 100;
		if (gamestate.maxammo > 299)
			gamestate.maxammo = 299;
		GiveAmmo (10);
		GiveGas (1);
		GiveMissile (1);
		SD_PlaySound (D_BONUSSND);
		break;

	case	bo_flamethrower:
		SD_PlaySound (D_BONUSSND);
		GiveGas (20);
		GiveWeapon (wp_flamethrower);
		StatusDrawPic (19,4,GOTGATLINGPIC);
		facecount = 0;
		gotgatgun = 1;
		if (!gamestate.flamethrower)
		{
			gamestate.flamethrower = 1;
			gamestate.bestweapon = gamestate.weapon = gamestate.chosenweapon
				= wp_flamethrower;
		}
		break;

	case	bo_gascan:
		if (gamestate.gas >= 99)
			return;

		SD_PlaySound (D_BONUSSND);
		GiveGas (14);
		break;

	case	bo_launcher:
		SD_PlaySound (D_BONUSSND);
		GiveMissile (5);
		GiveWeapon (wp_missile);
		StatusDrawPic (19,4,GOTGATLINGPIC);
		facecount = 0;
		gotgatgun = 1;
		if (!gamestate.missile)
		{
			gamestate.missile = 1;
			gamestate.bestweapon = gamestate.weapon = gamestate.chosenweapon
				= wp_missile;
		}
		break;

	case	bo_missiles:
		if (gamestate.missiles >= 99)
			return;

		SD_PlaySound (D_BONUSSND);
		GiveMissile (5);
		break;

	case	bo_map:
		SD_PlaySound (GETKEYSND);
		gamestate.automap = true;
		break;

	case bo_exit:
		playstate = ex_completed;
		break;

	case bo_secret:
		playstate = ex_secretlevel;
		break;
#else
	case	bo_alpo:
		if (gamestate.health == 100)
			return;

		SD_PlaySound (HEALTH1SND);
		HealSelf (4);
		break;

	case	bo_gibs:
		if (gamestate.health >10)
			return;

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		SD_PlaySound (HEALTH1SND);
#else
		SD_PlaySound (SLURPIESND);
#endif
		HealSelf (1);
		break;
#endif

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
	case	bo_spear:
		spearflag = true;
		spearx = player->x;
		speary = player->y;
		spearangle = player->angle;
		playstate = ex_completed;
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	default:
		sprintf (str,"GetBonus: Invalid item $%02X!\n",check->itemnumber);
		Quit (str);
#endif
	}

	StartBonusFlash ();
	check->shapenum = -1;			// remove from list
}


/*
===================
=
= TryMove
=
= returns true if move ok
= debug: use pointers to optimize
===================
*/

id0_boolean_t TryMove (objtype *ob)
{
	id0_int_t			xl,yl,xh,yh,x,y;
	objtype		*check;
	id0_long_t		deltax,deltay;

	xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
	yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

	xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
	yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

//
// check for solid walls
//
	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			if (actorat[x][y] && actorat[x][y]<refkeen_compat_wl_play_objoffset)
//			check = actorat[x][y];
//			if (check && check<objlist)
				return false;
		}

//
// check for actors
//
	if (yl>0)
		yl--;
	if (yh<MAPSIZE-1)
		yh++;
	if (xl>0)
		xl--;
	if (xh<MAPSIZE-1)
		xh++;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(actorat[x][y]);
			if (/*check > objlist*/
			    actorat[x][y] > refkeen_compat_wl_play_objoffset
			&& (check->flags & FL_SHOOTABLE) )
			{
				deltax = ob->x - check->x;
				if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
					continue;
				deltay = ob->y - check->y;
				if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
					continue;

				return false;
			}
		}

	return true;
}


/*
===================
=
= ClipMove
=
===================
*/

void ClipMove (objtype *ob, id0_long_t xmove, id0_long_t ymove)
{
	id0_long_t	basex,basey;

	basex = ob->x;
	basey = ob->y;

	ob->x = basex+xmove;
	ob->y = basey+ymove;
	if (TryMove (ob))
		return;

	if (noclip && ob->x > 2*TILEGLOBAL && ob->y > 2*TILEGLOBAL &&
	ob->x < (((id0_long_t)(mapwidth-1))<<TILESHIFT)
	&& ob->y < (((id0_long_t)(mapheight-1))<<TILESHIFT) )
		return;		// walk through walls

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	if (!SD_SoundPlaying())
		SD_PlaySound (HITWALLSND);
#endif

	ob->x = basex+xmove;
	ob->y = basey;
	if (TryMove (ob))
		return;

	ob->x = basex;
	ob->y = basey+ymove;
	if (TryMove (ob))
		return;

	ob->x = basex;
	ob->y = basey;
}

//==========================================================================

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
===================
=
= VictoryTile
=
===================
*/

void VictoryTile (void)
{
	// *** S3DNA RESTORATION ***
#if (!defined SPEAR) && (!defined GAMEVER_NOAH3D)
//#ifndef SPEAR
	SpawnBJVictory ();
#endif

	gamestate.victoryflag = true;
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


/*
===================
=
= Thrust
=
===================
*/

void Thrust (id0_int_t angle, id0_long_t speed)
{
	id0_long_t xmove,ymove;
	id0_long_t	slowmax;
	id0_unsigned_t	offset;


	//
	// ZERO FUNNY COUNTER IF MOVED!
	//
	#ifdef SPEAR
	if (speed)
		funnyticount = 0;
	#endif

	thrustspeed += speed;
//
// moving bounds speed
//
	if (speed >= MINDIST*2)
		speed = MINDIST*2-1;

	xmove = FixedByFrac(speed,costable[angle]);
	ymove = -FixedByFrac(speed,sintable[angle]);

	ClipMove(player,xmove,ymove);

	// *** ALPHA RESTORATION ***
	// Similar to code from T_Attack and T_Player
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in id0_unsigned_t
	pluy = player->y >> UNSIGNEDSHIFT;
#endif
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

	// Similar to code from T_Attack and T_Player
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	player->areanumber = *(mapsegs[0] + farmapylookup[player->tiley]+player->tilex) -AREATILE;
#else
	offset = farmapylookup[player->tiley]+player->tilex;
	player->areanumber = *(mapsegs[0] + offset) -AREATILE;
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (*(mapsegs[1] + offset) == EXITTILE)
		VictoryTile ();
#endif
}


/*
=============================================================================

								ACTIONS

=============================================================================
*/


/*
===============
=
= Cmd_Fire
=
===============
*/

void Cmd_Fire (void)
{
	buttonheld[bt_attack] = true;

	gamestate.weaponframe = 0;

	player->state = &s_attack;

	gamestate.attackframe = 0;
	gamestate.attackcount =
		attackinfo[gamestate.weapon][gamestate.attackframe].tics;
	gamestate.weaponframe =
		attackinfo[gamestate.weapon][gamestate.attackframe].frame;
}

//===========================================================================

/*
===============
=
= Cmd_Use
=
===============
*/

void Cmd_Use (void)
{
	objtype 	*check;
	// *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_int_t			checkx,checky,doornum;
#else
	id0_int_t			checkx,checky,doornum,dir;
#endif
#ifndef GAMEVER_NOAH3D
	id0_boolean_t		elevatorok;
#endif


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	buttonheld[bt_use] = true;
#endif
//
// find which cardinal direction the player is facing
//
	if (player->angle < ANGLES/8 || player->angle > 7*ANGLES/8)
	{
		checkx = player->tilex + 1;
		checky = player->tiley;
		// *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		dir = di_east;
#endif
#ifndef GAMEVER_NOAH3D
		elevatorok = true;
#endif
	}
	else if (player->angle < 3*ANGLES/8)
	{
		checkx = player->tilex;
		checky = player->tiley-1;
		// *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		dir = di_north;
#endif
#ifndef GAMEVER_NOAH3D
		elevatorok = false;
#endif
	}
	else if (player->angle < 5*ANGLES/8)
	{
		checkx = player->tilex - 1;
		checky = player->tiley;
		// *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		dir = di_west;
#endif
#ifndef GAMEVER_NOAH3D
		elevatorok = true;
#endif
	}
	else
	{
		checkx = player->tilex;
		checky = player->tiley + 1;
		// *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		dir = di_south;
#endif
#ifndef GAMEVER_NOAH3D
		elevatorok = false;
#endif
	}

	doornum = tilemap[checkx][checky];
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	doornum &= 0xFFDF;
#endif
	// *** ALPHA RESTORATION **/
	// Somewhat different code
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if ((doornum > 0) && (doornum < AREATILE))
	{
		if (*(mapsegs[1]+farmapylookup[checky]+checkx) == PUSHABLETILE)
		{
		//
		// secret wall
		//
			*(mapsegs[0]+farmapylookup[checky]+checkx) = player->areanumber + AREATILE;
			tilemap[checkx][checky] = 0;
			actorat[checkx][checky] = NULL;
			SD_PlaySound (OPENDOORSND);
		}
	}
	if (doornum == ELEVATORTILE && elevatorok)
	{
	//
	// use elevator
	//
		playstate = ex_completed;
		SD_PlaySound (LEVELDONESND);
	}
	else if (doornum & 0x80)
		OperateDoor (doornum & ~0x80);
#else // GAMEVER_WOLFREV > GV_WR_WL920312
	if (*(mapsegs[1]+farmapylookup[checky]+checkx) == PUSHABLETILE)
	{
	//
	// pushable wall
	//

		PushWall (checkx,checky,dir);
		return;
	}
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	if (!buttonheld[bt_use] && doornum == ELEVATORTILE && elevatorok)
	{
	//
	// use elevator
	//
		buttonheld[bt_use] = true;

		tilemap[checkx][checky]++;		// flip switch
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
		if (*(mapsegs[0]+farmapylookup[player->tiley]+player->tilex) == ALTELEVATORTILE)
			playstate = ex_secretlevel;
		else
#endif
			playstate = ex_completed;
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		SD_PlaySound (LEVELDONESND);
#endif
		SD_WaitSoundDone();
	}
	else
#endif // GAMEVER_NOAH3D
		if (!buttonheld[bt_use] && doornum & 0x80)
	{
		buttonheld[bt_use] = true;
		OperateDoor (doornum & ~0x80);
	}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
	else
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		SD_PlaySound (NOWAYSND);
#else
		SD_PlaySound (DONOTHINGSND);
#endif

}

/*
=============================================================================

						   PLAYER CONTROL

=============================================================================
*/



/*
===============
=
= SpawnPlayer
=
===============
*/

void SpawnPlayer (id0_int_t tilex, id0_int_t tiley, id0_int_t dir)
{
	player->obclass = playerobj;
	player->active = true;
	player->tilex = tilex;
	player->tiley = tiley;
	player->areanumber =
		*(mapsegs[0] + farmapylookup[player->tiley]+player->tilex);
	player->x = ((id0_long_t)tilex<<TILESHIFT)+TILEGLOBAL/2;
	player->y = ((id0_long_t)tiley<<TILESHIFT)+TILEGLOBAL/2;
	player->state = &s_player;
	player->angle = (1-dir)*90;
	if (player->angle<0)
		player->angle += ANGLES;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	player->flags = FL_NEVERMARK;
#endif
	Thrust (0,0);				// set some variables

	InitAreas ();
}


//===========================================================================

/*
===============
=
= T_KnifeAttack
=
= Update player hands, and try to do damage when the proper frame is reached
=
===============
*/

void	KnifeAttack (objtype *ob)
{
	objtype *check,*closest;
	id0_long_t	dist;

	SD_PlaySound (ATKKNIFESND);
// actually fire
	dist = 0x7fffffff;
	closest = NULL;
	for (check=ob->next ; check ; check=check->next)
		if ( (check->flags & FL_SHOOTABLE)
		&& (check->flags & FL_VISABLE)
		&& abs (check->viewx-centerx) < shootdelta
		)
		{
			if (check->transx < dist)
			{
				dist = check->transx;
				closest = check;
			}
		}

	if (!closest || dist> 0x18000l)
	{
	// missed

		return;
	}

// hit something
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	DamageActor (closest,US_RndT()&3);
#else
	DamageActor (closest,US_RndT() >> 4);
#endif
}



void	GunAttack (objtype *ob)
{
	objtype *check,*closest,*oldclosest;
	id0_int_t		damage;
	id0_int_t		dx,dy,dist;
	id0_long_t	viewdist;

	switch (gamestate.weapon)
	{
	case wp_pistol:
		SD_PlaySound (ATKPISTOLSND);
		break;
	case wp_machinegun:
		SD_PlaySound (ATKMACHINEGUNSND);
		break;
	case wp_chaingun:
		SD_PlaySound (ATKGATLINGSND);
		break;
	}

	madenoise = true;

//
// find potential targets
//
	viewdist = 0x7fffffffl;
	closest = NULL;

	while (1)
	{
		oldclosest = closest;

		for (check=ob->next ; check ; check=check->next)
			if ( (check->flags & FL_SHOOTABLE)
			&& (check->flags & FL_VISABLE)
			&& abs (check->viewx-centerx) < shootdelta
			)
			{
				if (check->transx < viewdist)
				{
					viewdist = check->transx;
					closest = check;
				}
			}

		if (closest == oldclosest)
			return;						// no more targets, all missed

	//
	// trace a line from player to enemey
	//
		if (CheckLine(closest))
			break;

	}

//
// hit something
//
	dx = abs(closest->tilex - player->tilex);
	dy = abs(closest->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (dist<2)
		damage = US_RndT() & 0xf;
	else
	{
		damage = US_RndT() & 7;

		if ((dist >= 4) && ( (US_RndT() / 12) < dist))		// missed
			return;
	}
#else
	if (dist<2)
		damage = US_RndT() / 4;
	else if (dist<4)
		damage = US_RndT() / 6;
	else
	{
		if ( (US_RndT() / 12) < dist)		// missed
			return;
		damage = US_RndT() / 6;
	}
#endif

	DamageActor (closest,damage);
}

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void FlameAttack (objtype *ob)
{
	id0_int_t x, y;

	SD_PlaySound (D_COCTHRSND);
	madenoise = true;
	gamestate.gas--;
	DrawAmmo ();
	GetNewActor ();
	newobj->state = &s_flame;
	newobj->obclass = flameobj;
	newobj->ticcount = 1;
	x = costable[ob->angle];
	y = -sintable[ob->angle];
	newobj->x = ob->x + (x>>2);
	newobj->y = ob->y + (y>>2);
	newobj->tilex = newobj->x >> TILESHIFT;
	newobj->tiley = newobj->y >> TILESHIFT;
	newobj->areanumber = ob->areanumber;
	newobj->dir = ob->dir;
	newobj->angle = ob->angle;
	newobj->speed = 0x3000;
	newobj->flags = FL_NEVERMARK;
	newobj->active = ac_yes;
}
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void MissileAttack (objtype *ob)
{
	id0_int_t x, y;

	SD_PlaySound (D_WATTHRSND);
	madenoise = true;
	gamestate.missiles--;
	DrawAmmo ();
	GetNewActor ();
	newobj->state = &s_missile;
	newobj->obclass = missileobj;
	newobj->ticcount = 1;
	x = costable[ob->angle];
	y = -sintable[ob->angle];
	newobj->x = ob->x + (x>>2);
	newobj->y = ob->y + (y>>2);
	newobj->tilex = newobj->x >> TILESHIFT;
	newobj->tiley = newobj->y >> TILESHIFT;
	newobj->areanumber = ob->areanumber;
	newobj->dir = ob->dir;
	newobj->angle = ob->angle;
	newobj->speed = 0x3000;
	newobj->flags = FL_NEVERMARK;
	newobj->active = ac_yes;
}
#endif

//===========================================================================

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
===============
=
= VictorySpin
=
===============
*/

void VictorySpin (void)
{
	id0_long_t	desty;

	if (player->angle > 270)
	{
		player->angle -= tics * 3;
		if (player->angle < 270)
			player->angle = 270;
	}
	else if (player->angle < 270)
	{
		player->angle += tics * 3;
		if (player->angle > 270)
			player->angle = 270;
	}

	desty = (((id0_long_t)player->tiley-5)<<TILESHIFT)-0x3000;

	if (player->y > desty)
	{
		player->y -= tics*4096;
		if (player->y < desty)
			player->y = desty;
	}
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


//===========================================================================

/*
===============
=
= T_Attack
=
===============
*/

void	T_Attack (objtype *ob)
{
	struct	atkinf	*cur;

	UpdateFace ();

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (gamestate.victoryflag)		// watching the BJ actor
	{
		VictorySpin ();
		return;
	}
#endif

	if ( buttonstate[bt_use] && !buttonheld[bt_use] )
		buttonstate[bt_use] = false;

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		buttonstate[bt_attack] = false;

	ControlMovement (ob);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (gamestate.victoryflag)		// watching the BJ actor
		return;
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in id0_unsigned_t
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;
#endif

//
// change frame and fire
//
	gamestate.attackcount -= tics;
	while (gamestate.attackcount <= 0)
	{
		cur = &attackinfo[gamestate.weapon][gamestate.attackframe];
		switch (cur->attack)
		{
		case -1:
			ob->state = &s_player;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if ((!gamestate.gas && (gamestate.weapon == wp_flamethrower))
			    || (!gamestate.missiles && (gamestate.weapon == wp_missile)))
			{
				if (gamestate.ammo)
				{
					if (gamestate.chaingun)
						gamestate.weapon = gamestate.chosenweapon = wp_chaingun;
					else if (gamestate.machinegun)
						gamestate.weapon = gamestate.chosenweapon = wp_machinegun;
					else
						gamestate.weapon = gamestate.chosenweapon = wp_pistol;
					DrawAmmo ();
				}
				else
					gamestate.weapon = wp_knife;
				DrawAmmo ();
			}
			else if (!gamestate.ammo)
			{
				if (gamestate.missile && gamestate.missiles)
					gamestate.weapon = gamestate.chosenweapon = wp_missile;
				else if (gamestate.flamethrower && gamestate.gas)
					gamestate.weapon = gamestate.chosenweapon = wp_flamethrower;
				else
					gamestate.weapon = wp_knife;
				DrawAmmo ();
			}
			else
			{
				if (gamestate.weapon != gamestate.chosenweapon)
				{
					gamestate.weapon = gamestate.chosenweapon;
					DrawAmmo ();
				}
			};
#else
			if (!gamestate.ammo)
			{
				gamestate.weapon = wp_knife;
				DrawWeapon ();
			}
			else
			{
				if (gamestate.weapon != gamestate.chosenweapon)
				{
					gamestate.weapon = gamestate.chosenweapon;
					DrawWeapon ();
				}
			};
#endif
			gamestate.attackframe = gamestate.weaponframe = 0;
			return;

		case 4:
			if (!gamestate.ammo)
				break;
			if (buttonstate[bt_attack])
				gamestate.attackframe -= 2;
		case 1:
			if (!gamestate.ammo)
			{	// can only happen with chain gun
				gamestate.attackframe++;
				break;
			}
			GunAttack (ob);
			gamestate.ammo--;
			DrawAmmo ();
			break;

		case 2:
			KnifeAttack (ob);
			break;

		case 3:
			if (gamestate.ammo && buttonstate[bt_attack])
				gamestate.attackframe -= 2;
			break;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		case 5:
			if (gamestate.gas)
				FlameAttack (ob);
			break;
		case 6:
			if (gamestate.missiles)
				MissileAttack (ob);
			break;
#endif
		}

		gamestate.attackcount += cur->tics;
		gamestate.attackframe++;
		gamestate.weaponframe =
			attackinfo[gamestate.weapon][gamestate.attackframe].frame;
	}

}



//===========================================================================

/*
===============
=
= T_Player
=
===============
*/

void	T_Player (objtype *ob)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (gamestate.victoryflag)		// watching the BJ actor
	{
		VictorySpin ();
		return;
	}
#endif

	UpdateFace ();
	CheckWeaponChange ();

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if ( buttonstate[bt_use] && !buttonheld[bt_use])
#else
	if ( buttonstate[bt_use] )
#endif
		Cmd_Use ();

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (buttonstate[bt_attack])

	    if (!buttonheld[bt_attack] || (gamestate.weapon == wp_knife) || (gamestate.weapon == wp_pistol))
#else
	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
#endif
		Cmd_Fire ();

	ControlMovement (ob);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (gamestate.victoryflag)		// watching the BJ actor
		return;


	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in id0_unsigned_t
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;
#endif
}


