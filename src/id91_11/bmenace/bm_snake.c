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

#include "bm_def.h"
//#pragma hdrstop

REFKEEN_NS_B

/////////////////////////////////////////////////////////////////////////////
// local macros:
/////////////////////////////////////////////////////////////////////////////

// This autofire condition is used many times, so I made it a macro to make
// the code easier to read.
#ifdef BETA
#define AUTOFIRE_ALLOWED (gamestate.ammo > 0)
#else
#define AUTOFIRE_ALLOWED (gamestate.ammo > 0 && gamestate.autofire == true)
#endif

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

static Sint16 opposite[] = {2, 3, 0, 1, 6, 7, 4, 5};
static boolean justtransformed = false;
Sint16 hintstate = 0;
boolean teleported = false;
#ifdef BETA
boolean bossActive;
#else
boolean bossActive = false;
#endif
boolean snakedied = false;

#ifndef BETA
static Sint16 turncount = 0;
static Sint16 upcount = 0;
Sint16 cooldown = 0;
static Sint16 shieldtime = 0;
#endif

/////////////////////////////////////////////////////////////////////////////
// state declarations
/////////////////////////////////////////////////////////////////////////////

FARSTATE s_snakedie1 = {SNAKEDIE1SPR, SNAKEDIE1SPR, think, false, push_none, 0, 0, 0, SnakeDieThink, NULL, SnakeDieReact, &s_snakedie2};
FARSTATE s_snakedie2 = {SNAKEDIE2SPR, SNAKEDIE2SPR, step, false, push_none, 50, 0, 0, SnakeDeadThink, NULL, DrawReact, &s_snakedie2};
#ifdef BETA
FARSTATE s_snakeskeleton = {SNAKEDIE1SPR, SNAKEDIE1SPR, step, false, push_none, 15, 0, 0, NULL, NULL, DrawReact, &s_snakestand};
#else
FARSTATE s_snakeskeleton = {SNAKEDIE1SPR, SNAKEDIE1SPR, step, false, push_down, 20, 0, 0, NULL, SnakeStandContact, DrawReact, &s_snakestand};
FARSTATE s_snakeshield1 = {SNAKESHIELD1LSPR, SNAKESHIELD1RSPR, stepthink, false, push_down, 10, 0, 0, SnakePauseThink, SnakeShieldContact, DrawReact, &s_snakeshield2};
FARSTATE s_snakeshield2 = {SNAKESHIELD2LSPR, SNAKESHIELD2RSPR, stepthink, false, push_down, 10, 0, 0, SnakePauseThink, SnakeShieldContact, DrawReact, &s_snakeshield1};
FARSTATE s_snakeUNUSED1 = {SNAKEDIE1SPR, SNAKEDIE1SPR, step, false, push_none, 15, 0, 0, NULL, NULL, DrawReact, &s_snakestand}; // REFKEEN: Unused, but keep if will be relevant for saved games
#endif
FARSTATE s_snakestand = {SNAKESTANDLSPR, SNAKESTANDRSPR, stepthink, false, push_down, 4, 0, 16, SnakePauseThink, SnakeStandContact, SnakeStandReact, &s_snakestand};
#if (EPISODE == 2)
FARSTATE s_snakewormstand = {SNAKEWORM1LSPR, SNAKEWORM1RSPR, stepthink, false, push_down, 4, 0, 16, SnakeWormStandThink, SnakeStandContact, SnakeWormStandReact, &s_snakewormstand};
#endif
FARSTATE s_snakestanduse = {SNAKESTANDLSPR, SNAKESTANDRSPR, step, false, push_down, 1, 0, 0, SnakeUseThink, NULL, SnakeStandReact, &s_snakestand};
FARSTATE s_snakeUNUSED2 = {SNAKESTANDLSPR, SNAKESTANDRSPR, think, false, push_none, 0, 0, 0, SnakeSlideThink, NULL, DrawReact, NULL}; // REFKEEN: Unused, but keep for saved games
FARSTATE s_snakeslide = {SNAKEUSESPR, SNAKEUSESPR, think, false, push_none, 0, 0, 0, SnakeSlideThink, NULL, DrawReact, NULL};
#ifdef BETA
void SnakeShuttleThink(objtype *ob);
extern FARSTATE s_snakeshuttle;
FARSTATE s_snakeshuttle = {SNAKESHUTTLESPR, SNAKESHUTTLESPR, think, false, push_none, 4, 0, 32, SnakeShuttleThink, SnakeStandContact, SnakeStandReact, &s_snakeshuttle};
#endif
FARSTATE s_snaketeleport1 = {SNAKEUSESPR, SNAKEUSESPR, step, false, push_none, 9, 0, 0, NULL, NULL, DrawReact, &s_snaketeleport2};
#ifdef BETA
FARSTATE s_snaketeleport2 = {-1, -1, step, false, push_none, 9, 0, 0, SnakeTeleportThink, NULL, DrawReact, &s_snakestand};
#else
FARSTATE s_snaketeleport2 = {-1, -1, step, false, push_none, 1, 0, 0, SnakeTeleportThink, NULL, DrawReact, &s_snakestand};
#endif
FARSTATE s_snakeuse1 = {SNAKEUSESPR, SNAKEUSESPR, step, false, push_down, 8, 0, 0, SnakeUseThink, NULL, SnakeStandReact, &s_snakeuse2};
FARSTATE s_snakeuse2 = {SNAKEUSESPR, SNAKEUSESPR, step, false, push_down, 8, 0, 0, NULL, NULL, SnakeStandReact, &s_snakestand};
FARSTATE s_snakeopendoor1 = {SNAKEUSESPR, SNAKEUSESPR, step, false, push_down, 30, 0, 0, NULL, SnakeStandContact, SnakeStandReact, &s_snakeopendoor2};
FARSTATE s_snakeopendoor2 = {SNAKEUSESPR, SNAKEUSESPR, step, false, push_down, 1, 0, 0, SnakeOpenDoorThink, SnakeStandContact, SnakeStandReact, &s_snakestand};
#ifdef BETA
FARSTATE s_snakeduck = {SNAKEDUCKFIRE1LSPR, SNAKEDUCKFIRE1RSPR, think, false, push_down, 0, 0, 0, SnakePauseThink, SnakeStandContact, SnakeStandReact, &s_snakeduck};
#else
FARSTATE s_snakeduck = {SNAKEDUCKFIRE1LSPR, SNAKEDUCKFIRE1RSPR, stepthink, false, push_down, 4, 0, 16, SnakePauseThink, SnakeStandContact, SnakeStandReact, &s_snakeduck};
FARSTATE s_snakeplacemine = {SNAKEDUCKFIRE1LSPR, SNAKEDUCKFIRE1RSPR, step, false, push_down, 30, 0, 0, SnakeThrow, NULL, DrawReact, &s_snakestand};
#endif
FARSTATE s_snakeladder = {SNAKECLIMB1SPR, SNAKECLIMB1SPR, think, false, push_none, 0, 0, 0, SnakeLadderThink, SnakeStandContact, DrawReact, &s_snakeladder};
FARSTATE s_snakeclimb1 = {SNAKECLIMB1SPR, SNAKECLIMB1SPR, slidethink, false, push_none, 8, 0, 16, SnakeClimbThink, SnakeStandContact, DrawReact, &s_snakeclimb2};
FARSTATE s_snakeclimb2 = {SNAKECLIMB2SPR, SNAKECLIMB2SPR, slidethink, false, push_none, 8, 0, 16, SnakeClimbThink, SnakeStandContact, DrawReact, &s_snakeclimb1};
FARSTATE s_snakewalk1 = {SNAKEWALK1LSPR, SNAKEWALK1RSPR, slidethink, true, push_down, 6, 24, 0, SnakeWalkThink, SnakeStandContact, SnakeWalkReact, &s_snakewalk2};
FARSTATE s_snakewalk2 = {SNAKEWALK2LSPR, SNAKEWALK2RSPR, slidethink, true, push_down, 6, 24, 0, SnakeWalkThink, SnakeStandContact, SnakeWalkReact, &s_snakewalk3};
FARSTATE s_snakewalk3 = {SNAKEWALK3LSPR, SNAKEWALK3RSPR, slidethink, true, push_down, 6, 24, 0, SnakeWalkThink, SnakeStandContact, SnakeWalkReact, &s_snakewalk4};
FARSTATE s_snakewalk4 = {SNAKEWALK4LSPR, SNAKEWALK4RSPR, slidethink, true, push_down, 6, 24, 0, SnakeWalkThink, SnakeStandContact, SnakeWalkReact, &s_snakewalk1};
#if (EPISODE == 2)
FARSTATE s_snakewormwalk1 = {SNAKEWORM1LSPR, SNAKEWORM1RSPR, slidethink, true, push_down,  6, 24, 0, SnakeWormWalkThink, SnakeStandContact, SnakeWormWalkReact, &s_snakewormwalk2};
FARSTATE s_snakewormwalk2 = {SNAKEWORM2LSPR, SNAKEWORM2RSPR, slidethink, true, push_down, 10, 24, 0, SnakeWormWalkThink, SnakeStandContact, SnakeWormWalkReact, &s_snakewormwalk1};
#endif
FARSTATE s_snakethrow1 = {SNAKESTANDTHROW1LSPR, SNAKESTANDTHROW1RSPR, step, true, push_down, 20, 0, 0, NULL, SnakeStandContact, SnakeStandReact, &s_snakethrow2};
FARSTATE s_snakethrow2 = {SNAKESTANDTHROW2LSPR, SNAKESTANDTHROW2RSPR, step, false, push_down, 10, 0, 0, SnakeThrow, SnakeStandContact, SnakeStandReact, &s_snakethrow3};
FARSTATE s_snakethrow3 = {SNAKESTANDTHROW1LSPR, SNAKESTANDTHROW1RSPR, step, true, push_down, 6, 0, 0, NULL, SnakeStandContact, SnakeStandReact, &s_snakestand};
#ifdef BETA
FARSTATE s_snakeairthrow1 = {SNAKEAIRTHROW1LSPR, SNAKEAIRTHROW1RSPR, stepthink, false, push_none, 20, 0, 0, ProjectileThink, SnakeContact, SnakeAirReact, &s_snakeairthrow2};
FARSTATE s_snakeairthrow2 = {SNAKEAIRTHROW2LSPR, SNAKEAIRTHROW2RSPR, step, false, push_none, 6, 0, 0, SnakeThrow, SnakeContact, SnakeAirReact, &s_snakeairthrow3};
FARSTATE s_snakeairthrow3 = {SNAKEAIRTHROW1LSPR, SNAKEAIRTHROW1RSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, SnakeContact, SnakeAirReact, &s_snakejump1};
FARSTATE s_snakejump1 = {SNAKEJUMPLSPR, SNAKEJUMPRSPR, think, false, push_none, 0, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakejump2};
FARSTATE s_snakejump2 = {SNAKEJUMPLSPR, SNAKEJUMPRSPR, think, false, push_none, 0, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakejump3};
FARSTATE s_snakejump3 = {SNAKEJUMPLSPR, SNAKEJUMPRSPR, think, false, push_none, 0, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakestand};
#else
FARSTATE s_snakeairthrow1 = {SNAKEAIRTHROW1LSPR, SNAKEAIRTHROW1RSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, SnakeContact, SnakeAirReact, &s_snakeairthrow2};
FARSTATE s_snakeairthrow2 = {SNAKEAIRTHROW2LSPR, SNAKEAIRTHROW2RSPR, step, false, push_none, 3, 0, 0, SnakeThrow, SnakeContact, SnakeAirReact, &s_snakeairthrow3};
FARSTATE s_snakeairthrow3 = {SNAKEAIRTHROW1LSPR, SNAKEAIRTHROW1RSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, SnakeContact, SnakeAirReact, &s_snakejump1};
FARSTATE s_snakejump1 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, think, false, push_none, 0, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakejump2};
FARSTATE s_snakejump2 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, think, false, push_none, 0, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakejump3};
FARSTATE s_snakejump3 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, think, false, push_none, 0, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakestand};
#endif
#if (EPISODE == 2)
FARSTATE s_snakewormair1 = {SNAKEWORMJUMPLSPR, SNAKEWORMJUMPRSPR, think, false, push_none, 0, 0, 0, SnakeWormAirThink, SnakeContact, SnakeWormAirReact, &s_snakewormair2};
FARSTATE s_snakewormair2 = {SNAKEWORMJUMPLSPR, SNAKEWORMJUMPRSPR, think, false, push_none, 0, 0, 0, SnakeWormAirThink, SnakeContact, SnakeWormAirReact, &s_snakewormair3};
FARSTATE s_snakewormair3 = {SNAKEWORMJUMPLSPR, SNAKEWORMJUMPRSPR, think, false, push_none, 0, 0, 0, SnakeWormAirThink, SnakeContact, SnakeWormAirReact, &s_snakewormstand};
FARSTATE s_snakewormattack = {SNAKEWORMATTACKLSPR, SNAKEWORMATTACKRSPR, step, false, push_down, 20, 0, 0, NULL, SnakeStandContact, DrawReact, &s_snakewormstand};
FARSTATE s_snakewormairattack = {SNAKEWORMATTACKLSPR, SNAKEWORMATTACKRSPR, stepthink, false, push_none, 20, 0, 0, ProjectileThink, SnakeStandContact, SnakeWormAirReact, &s_snakewormair1};
#endif
FARSTATE s_snakestandfire1 = {SNAKESTANDFIRE1LSPR, SNAKESTANDFIRE1RSPR, step, false, push_down, 5, 0, 0, SnakeAttackThink, SnakeStandContact, DrawReact, &s_snakestandfire2};
FARSTATE s_snakestandfire2 = {SNAKESTANDFIRE2LSPR, SNAKESTANDFIRE2RSPR, step, false, push_down, 5, 0, 0, SnakeFireThink, SnakeStandContact, DrawReact, &s_snakestandfire3};
FARSTATE s_snakestandfire3 = {SNAKESTANDFIRE1LSPR, SNAKESTANDFIRE1RSPR, step, false, push_down, 5, 0, 0, NULL, SnakeStandContact, DrawReact, &s_snakestand};
#ifdef BETA
FARSTATE s_snakeairfire1 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, stepthink, false, push_none, 5, 0, 0, ProjectileThink, SnakeContact, SnakeAirReact, &s_snakeairfire2};
FARSTATE s_snakeairfire2 = {SNAKEAIRFIRE2LSPR, SNAKEAIRFIRE2RSPR, step, false, push_none, 1, 0, 0, SnakeFireThink, SnakeContact, SnakeAirReact, &s_snakeairfire3};
FARSTATE s_snakeairfire3 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, stepthink, false, push_none, 9, 0, 0, ProjectileThink, SnakeContact, SnakeAirReact, &s_snakejump1};
#else
FARSTATE s_snakeairfire1 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, stepthink, false, push_none, 9, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakeairfire2};
FARSTATE s_snakeairfire2 = {SNAKEAIRFIRE2LSPR, SNAKEAIRFIRE2RSPR, step, false, push_none, 1, 0, 0, SnakeFireThink, SnakeContact, SnakeAirReact, &s_snakeairfire3};
FARSTATE s_snakeairfire3 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, stepthink, false, push_none, 5, 0, 0, SnakeAirThink, SnakeContact, SnakeAirReact, &s_snakejump1};
#endif
FARSTATE s_snakeduckfire1 = {SNAKEDUCKFIRE1LSPR, SNAKEDUCKFIRE1RSPR, step, false, push_down, 5, 0, 0, SnakeAttackThink, SnakeStandContact, DrawReact, &s_snakeduckfire2};
FARSTATE s_snakeduckfire2 = {SNAKEDUCKFIRE2LSPR, SNAKEDUCKFIRE2RSPR, step, false, push_down, 5, 0, 0, SnakeDuckFireThink, SnakeStandContact, DrawReact, &s_snakeduckfire3};
FARSTATE s_snakeduckfire3 = {SNAKEDUCKFIRE1LSPR, SNAKEDUCKFIRE1RSPR, step, false, push_down, 5, 0, 0, NULL, SnakeStandContact, DrawReact, &s_snakeduck};
FARSTATE s_snakestandautofire1 = {SNAKESTANDFIRE1LSPR, SNAKESTANDFIRE1RSPR, step, false, push_down, 2, 0, 0, NULL, SnakeContact, DrawReact, &s_snakestandautofire2};
FARSTATE s_snakestandautofire2 = {SNAKESTANDFIRE2LSPR, SNAKESTANDFIRE2RSPR, step, false, push_down, 2, 0, 0, SnakeAutofireThink, SnakeContact, DrawReact, &s_snakestandautofire1};
FARSTATE s_snakeduckautofire1 = {SNAKEDUCKFIRE1LSPR, SNAKEDUCKFIRE1RSPR, step, false, push_down, 2, 0, 0, NULL, SnakeContact, DrawReact, &s_snakeduckautofire2};
FARSTATE s_snakeduckautofire2 = {SNAKEDUCKFIRE2LSPR, SNAKEDUCKFIRE2RSPR, step, false, push_down, 2, 0, 0, SnakeDuckAutofireThink, SnakeContact, DrawReact, &s_snakeduckautofire1};
FARSTATE s_snakeairautofire1 = {SNAKEAIRFIRE1LSPR, SNAKEAIRFIRE1RSPR, stepthink, false, push_none, 3, 0, 0, SnakeAirNofireThink, SnakeContact, SnakeAirReact, &s_snakeairautofire2};
FARSTATE s_snakeairautofire2 = {SNAKEAIRFIRE2LSPR, SNAKEAIRFIRE2RSPR, step, false, push_none, 1, 0, 0, SnakeAirAutofireThink, SnakeContact, SnakeAirReact, &s_snakeairautofire1};

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

static Sint16 polexspeed[] = {-8, 0, 8};

#if (EPISODE == 2)
Sint16 slopespeed[8] = {0, 0, 4, 4, 8, -4, -4, -8};	//left over from Keen
#endif
#if (EPISODE != 1)
Uint16 bonusscores[10] = {
	100, 200, 500, 800, 1000, 1500, 2000, 5000, 20000, 50000
};
#endif

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

#ifdef __BSS_CHEAT__
__snakebssvarstype __BSS_CHEAT__;
#else
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
#endif

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// SNAKE LOGAN
//
// temp1: x position to slide to when using switches/doors / worm idle time
// temp2: jump state change flag
//
// temp4: x tile coordinate for current ladder
//
//////////////////////////////////////////////////////////////////////////////

/*
=================
=
= SpawnPlayer
=
=================
*/

void SpawnPlayer(Uint16 x, Uint16 y, Sint16 xdir)
{
	player->obclass = playerobj;
	player->active = yes;
	player->needtoclip = cl_midclip;
#ifdef BETA
	player->x = CONVERT_TILE_TO_GLOBAL(x);
	player->y = SPAWN_ADJUST_Y(y, 40);
#else
	respawnx = player->x = CONVERT_TILE_TO_GLOBAL(x);
	respawny = player->y = SPAWN_ADJUST_Y(y, 40);
	// Note: For episode 2, it might be a good idea to set respawnx and respawny
	// to 0 when gamestate.mapon is 3 (Ant Town), since respawning at the start
	// of that level usually means the level is unwinnable.
#endif
	player->xdir = xdir;
	player->ydir = 1;
	// BUG: The beta should ALWAYS initialize player->health here!
	// Entering a new level always resets all objects, so the player would have
	// a health value of 0 here. The health must be stored in the gamestate
	// struct to be carried over into the next level, like in the later versions.
	if (gamestate.mapon == 0 || snakedied == true)
	{
		// Note: The snakedied check was left over from when the game didn't mark
		// the player's spawn point as a respawn point yet. If no respawn point
		// is set, the RespawnPlayer function in BM_GAME.C will return false and
		// the current level will be restarted (gamestate.health would be either
		// the value the current level was started with or the value set by the
		// last First Aid Kit) and therefore it might be necessary to give the
		// player full health in here. The decision to make the spawn point act
		// as a respawn point might have been made pretty late in the development
		// process, because it can make E2L4 (Ant Town) impossible to beat after
		// respawning at the beginning of the level and I can't imagine how
		// something like this could have made it past the beta testers.
		snakedied = false;
		switch (gamestate.difficulty)
		{
		case gd_Easy:
			player->health = 8;
			break;
		case gd_Normal:
			player->health = 4;
			break;
		case gd_Hard:
#ifdef BETA
			player->health = 2;
			// Note: The beta's food system allowed the player's health to be
			// increased up to 8 (and maybe even beyond 8 when picking up a meal)
			// on ALL difficulty levels. That means starting with 2 health wouldn't
			// be quite as bad as in the final game. But since the next level
			// change always set the health to 0, this extra health would never
			// last for more than the current level anyway.
#else
			player->health = 4;
			// Note: Respawning after death or picking up a first aid kit will
			// still set the player's health to 2 instead of 4 when playing on hard
			// difficulty. It's not exactly clear if that was a conscious design
			// decision or if letting the player start with 4 health was a late
			// addition and Jim Norwood simply forgot to update all the other code
			// accordingly.
			// If it was an accident, it demonstrates why code like this should be
			// moved into a separate subroutine (or a macro) if it's used several
			// times in the code. If it's in a separate subroutine, you only need
			// to change one part of the code and there are no other instances that
			// you could forget to update.
#endif
			break;
		default:
			player->health = 4;
			break;
		}
#ifndef BETA
		gamestate.health = player->health;
#endif
	}
	// Note: The whole idea of the player's health being carried over into the
	// next level is something that should probably be removed altogether. It
	// was completely broken in the beta and it is useless in the final game
	// because saving the game and re-loading it ALWAYS gives the player full
	// health anyway. You might as well let the player start each level at full
	// health without having to go through the save and load procedures.
#ifndef BETA
	player->health = gamestate.health;
#endif
	invincibility = 0;
	shuttlecooldown = 0;
	NewState(player, &s_snakestand);
	if (gamestate.mapon == 12)
	{
		godmode = true;
	}
	else if (!DebugOk)
	{
		godmode = false;
	}
}

//==========================================================================


/*
=================
=
= CheckInteractiveTiles
=
=================
*/

boolean CheckInteractiveTiles(objtype *ob)
{
	Uint16 temp, intileinfo;
	
	intileinfo = tinf[FORESPOT(ob->tilemidx, ob->tiletop+1)+INTILE];
	if (intileinfo == INTILE_KEYCARDSLOT       // 3
		|| intileinfo == INTILE_PLATFORMSWITCH  //19
		|| intileinfo == INTILE_BRIDGESWITCH    //18
		|| intileinfo == INTILE_BLUESLOT        // 9
		|| intileinfo == INTILE_GREENSLOT       //10
		|| intileinfo == INTILE_REDSLOT         //11
		|| intileinfo == INTILE_CYANSLOT        //12
		|| intileinfo == INTILE_NUMBEREDSWITCH  //23
		|| intileinfo == INTILE_NUKESOCKET      //14
		|| intileinfo == INTILE_HOSTAGEKEYSLOT) //15
	{
		temp = CONVERT_TILE_TO_GLOBAL(ob->tilemidx) - 4*PIXGLOBAL;
		if (ob->x != temp)
		{
			ob->temp1 = temp;
			ob->state = &s_snakeslide;
		}
		else
		{
			ob->state = &s_snakeuse1;
		}
		upheld = true;
		return true;
	}
	if (intileinfo == INTILE_TELEPORT) //7
	{
		temp = CONVERT_TILE_TO_GLOBAL(ob->tilemidx) - 4*PIXGLOBAL;
		if (ob->x != temp)
		{
			ob->temp1 = temp;
			ob->state = &s_snakeslide;
		}
		else
		{
			ob->state = &s_snaketeleport1;
			ob->priority = 0;
		}
		upheld = true;
		return true;
	}
	if (intileinfo == INTILE_GREENTRIANGLE //4
		|| intileinfo == INTILE_DOOR //6
		|| intileinfo == INTILE_BETADOOR) //5
	{
#ifndef BETA
		if (intileinfo == INTILE_GREENTRIANGLE)
		{
			if (gamestate.trianglekey <= 0)
			{
				SD_PlaySound(ACCESSDENIEDSND);
				return true;
			}
			gamestate.trianglekey--;
		}
		else
#endif
		{
			if (!gamestate.keys.keys)
			{
#ifndef BETA
				HelpMessage("You don't have a key for this!\n");
#endif
				SD_PlaySound(ACCESSDENIEDSND);
				return true;
			}
			gamestate.keys.keys--;
		}
		ChangeState(ob, &s_snakeopendoor1);
		upheld = true;
		return true;
	}
	if (intileinfo == INTILE_SECRETDOOR) //8
	{
		SD_PlaySound(ACCESSDOORSND);
		if (!gamestate.specialkeys)
		{
#ifndef BETA
			HelpMessage("You need a SPECIAL key.\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return true;
		}
		gamestate.specialkeys = 0;
		ChangeState(ob, &s_snakeopendoor1);
		upheld = true;
		return true;
	}
	if (intileinfo == (INTILE_TURBOSHUTTLE | INTILE_FOREGROUND))
	{
		return true;
	}
#if (EPISODE == 2) && !(defined BETA)
	if (intileinfo == INTILE_BOSSTRIGGER) //21
	{
		ob->state = &s_snakestanduse;
		upheld = true;
		return true;
	}
	if (intileinfo == INTILE_HINTTRIGGER && hintstate == 0) //22
	{
		ob->state = &s_snakestanduse;
		upheld = true;
		return true;
	}
#else
	if (intileinfo == INTILE_BOSSTRIGGER    //21
		|| intileinfo == INTILE_HINTTRIGGER) //22
	{
		ob->state = &s_snakestanduse;
		upheld = true;
		return true;
	}
#endif
	return false;
}

//==========================================================================


/*
=================
=
= CheckGrabPole
=
= Returns true when grabbed a pole/ladder
=
=================
*/

boolean CheckGrabPole(objtype *ob)
{
	Uint16 id0_far *map;
	Uint16 midx;

	// REFKEEN - Looks like these are unsigned comparisons in original EXE
	if (SD_GetTimeCount() < (Uint32)leavepoletime)
	{
		leavepoletime = 0;
	}
	else if (SD_GetTimeCount() - (Uint32)leavepoletime < 19)
	{
		return false;
	}
	if (c.yaxis == -1)
	{
		map = &FORESPOT(0, CONVERT_GLOBAL_TO_TILE(ob->top+6*PIXGLOBAL));
	}
	else
	{
		map = &FORESPOT(0, ob->tilebottom);
	}
	midx = CONVERT_GLOBAL_TO_TILE(ob->left + (ob->right-ob->left)/2);
	map += midx;
	if (INTILETYPE(*map) == INTILE_POLE)
	{
		xtry = CONVERT_TILE_TO_GLOBAL(midx) + PIXGLOBAL*-8 - ob->x;
		ytry = c.yaxis << 5;
		ob->temp4 = midx;
		ob->needtoclip = cl_midclip;	// would have used cl_noclip in Keen 4-6
		ob->state = &s_snakeclimb1;
		return true;
	}
	return false;
}

//==========================================================================


/*
=================
=
= PoleActions
=
= Returns true if jumped off the pole/ladder
=
=================
*/

boolean PoleActions(objtype *ob)
{
	if (c.xaxis)
	{
		ob->xdir = c.xaxis;
	}
	if (c.button0 && !button0held)
	{
		SD_PlaySound(SNAKEJUMPSND);
		ob->xspeed = polexspeed[c.xaxis + 1];
		ob->yspeed = -40;
		ob->needtoclip = cl_midclip;	// leftover from Keen (Keen is noclipping on poles)
		jumptime = 16;
		ob->state = &s_snakejump1;
		ob->ydir = 1;
		button0held = true;
		leavepoletime = SD_GetTimeCount();
		return true;
	}
	return false;
}

//==========================================================================


/*
=================
=
= SnakeSlideThink
=
=================
*/

void SnakeSlideThink(objtype *ob)
{
	Sint16 deltax;
	
	deltax = ob->temp1-ob->x;
	if (deltax < 0)
	{
		xtry = xtry - (tics << 4);
		if (xtry > deltax)
		{
			return;
		}
	}
	else if (deltax > 0)
	{
		xtry = xtry + (tics << 4);
		if (xtry < deltax)
		{
			return;
		}
	}
	xtry = deltax;
	ob->temp1 = 0;
	if (!CheckInteractiveTiles(ob))
	{
		ob->state = &s_snakestand;
	}
}

//==========================================================================


/*
=================
=
= SnakeThrow
=
=================
*/

void SnakeThrow(objtype *ob)
{
	if (ob->xdir > 0)
	{
		ThrowPower(ob->x + 24*PIXGLOBAL, ob->y, dir_East);
	}
	else
	{
		ThrowPower(ob->x, ob->y, dir_West);
	}
#ifdef BETA
	if (ob->state == &s_snakethrow2)
	{
		return;
	}
#else
	if (ob->state == &s_snakethrow2 || ob->state == &s_snakeplacemine)
	{
		return;
	}
#endif
	if (ob->state == &s_snakeairthrow2)
	{
		newobj->xspeed += ob->xspeed/2;
		newobj->yspeed += ob->yspeed/2;
		return;
	}
	Quit("SnakeThrow: Bad state!");
}

//==========================================================================


/*
=================
=
= SnakeTeleportThink
=
=================
*/

void SnakeTeleportThink(objtype *ob)
{
	Uint16 id0_far *map;
	Uint16 info;

	if (teleported)
	{
		return;
	}

	teleported = true;
	map = &INFOSPOT(ob->tileleft, ob->tilebottom);
	info = *map;
	ob->y = (CONVERT_TILE_TO_GLOBAL(info & 0xFF) - TILEGLOBAL) + 0x8F;
	ob->x = CONVERT_TILE_TO_GLOBAL(info >> 8);
	ob->priority = 1;
	ob->needtoclip = cl_noclip;
	ChangeState(ob, ob->state->nextstate);
	ob->needtoclip = cl_midclip;
	CenterActor(ob);
	ytry = 15;
}

//==========================================================================


/*
=================
=
= SnakeUseThink
=
=================
*/

void SnakeUseThink(objtype *ob)
{
	Uint16 intileinfo, tilem, nexttile, infoval, info_hi, info_lo, planeoffset;
	Uint16 id0_far *map;
	Uint16 mapval, x, y;
	Sint8 manim;
	objtype *obj;
	
	planeoffset = mapbwidthtable[ob->tiletop+1]/2 + ob->tilemidx;
	tilem = mapsegs[1][planeoffset];
	nexttile = tilem + (Sint8)tinf[tilem+MANIM];
	infoval = mapsegs[2][planeoffset];
	info_hi = infoval >> 8;
	info_lo = infoval & 0xFF;
	intileinfo = tinf[tilem+INTILE];
	switch(intileinfo)
	{
	case INTILE_BRIDGESWITCH:   //18:
	case INTILE_PLATFORMSWITCH: //19:
	case INTILE_NUMBEREDSWITCH: //23:
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		SD_PlaySound(CLICKSND);
		break;
		
	case INTILE_KEYCARDSLOT: //3:
		if (!gamestate.keys.keycards)
		{
#ifndef BETA
			HelpMessage("You don't have a key for this!\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		gamestate.keys.keycards--;
		SD_PlaySound(ACCESSDOORSND);
		break;
		
	case INTILE_BLUESLOT: //9:
		if (!gamestate.gotshard[0])
		{
#ifndef BETA
			HelpMessage("You don't have the right shard!\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		gamestate.gotshard[0] = 0;
		SD_PlaySound(CRYSTALSND);
		break;
		
	case INTILE_GREENSLOT: //10:
		if (!gamestate.gotshard[1])
		{
#ifndef BETA
			HelpMessage("You don't have the right shard!\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		gamestate.gotshard[1] = 0;
		SD_PlaySound(CRYSTALSND);
		break;
		
	case INTILE_REDSLOT: //11:
		if (!gamestate.gotshard[2])
		{
#ifndef BETA
			HelpMessage("You don't have the right shard!\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		gamestate.gotshard[2] = 0;
		SD_PlaySound(CRYSTALSND);
		break;
		
	case INTILE_CYANSLOT: //12:
		if (!gamestate.gotshard[3])
		{
#ifndef BETA
			HelpMessage("You don't have the right shard!\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		gamestate.gotshard[3] = 0;
		SD_PlaySound(CRYSTALSND);
		break;
		
	case INTILE_HOSTAGEKEYSLOT: //15:
		if (!gamestate.got_exitkey)
		{
#ifndef BETA
			HelpMessage("You must rescue the hostage first!\n");
#endif
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		gamestate.got_exitkey = false;
		SD_PlaySound(CRYSTALSND);
		break;
		
	case INTILE_NUKESOCKET: //14:
#ifdef BETA
		if (gamestate.nukestate == 0)
		{
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
#else
		if (gamestate.nukestate != 1)
		{
			HelpMessage("You don't have a nuclear bomb!\n");
			SD_PlaySound(ACCESSDENIEDSND);
			return;
		}
#endif
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		SD_PlaySound(MANGLESND);
#ifdef BETA
		gamestate.nukestate = 0;
#else
		gamestate.nukestate = 2;
#endif
		VW_FixRefreshBuffer();
		US_CenterWindow(35, 3);
		PrintY += 2;
		US_CPrint("The bomb is in place and activated!\n Now you must leave before it explodes!\n");
		VW_UpdateScreen();
		VW_WaitVBL(60);
		IN_ClearKeysDown();
		IN_Ack();
		RF_ForceRefresh();
		break;
		
	case INTILE_BOSSTRIGGER:
#if (EPISODE == 3)
#define BOSSOBJCLASS cainobj
#else
#define BOSSOBJCLASS bossobj
#endif
		RF_MemToMap(&nexttile, 1, ob->tilemidx, ob->tiletop+1, 1, 1);
		if (bossActive)
		{
			return;
		}
		for (obj=player->next; obj; obj=obj->next)
		{
			if (obj->obclass == BOSSOBJCLASS)
			{
				goto found_boss;
			}
		}
		return;
found_boss:
		SD_PlaySound(MANGLESND);
		BossDialog();
		//Note: the loops below are useless. obj already points to the first boss object!
#ifdef BETA
		if (mapon != 11)
#else
		if (gamestate.mapon != 11)
#endif
		{
			for (obj=player->next; obj; obj=obj->next)
			{
				if (obj->obclass == BOSSOBJCLASS)
				{
					RemoveObj(obj);
					return;
				}
			}
		}
		else
		{
			for (obj=player->next; obj; obj=obj->next)
			{
				if (obj->obclass == BOSSOBJCLASS)
				{
#ifdef BETA
					obj->shootable = true;
					obj->health = 50;
					bossActive = true;
					return;
#elif (EPISODE == 1)
					obj->state = &s_manglejump;
					obj->shootable = true;
					bossActive = true;
					return;
#elif (EPISODE == 2)
					obj->shootable = true;
					bossActive = true;
					obj->health = bosshealth = oldhealthbarlength = 100;
					bossdiv = bosshealth / 20;
					return;
#elif (EPISODE == 3)
					obj->state = &s_cainstand;
					obj->shootable = true;
					bossActive = true;
					return;
#endif
				}
			}
		}
		break;
		
	case INTILE_HINTTRIGGER: //22:
		if (hintstate == 3)
		{
			return;
		}
		if (hintstate != 0 && !Keyboard[sc_UpArrow])
		{
			return;
		}
#ifdef BETA
		SD_PlaySound(APOGEESND);
#endif
		hintstate = 1;
		HintDialog();
		break;
		
	default:
		return;
	}
	
	if (intileinfo == INTILE_BRIDGESWITCH //18
		|| intileinfo == INTILE_GREENSLOT) //10
	{
#ifndef BETA
		if (!gamestate.hint_switchbridge && intileinfo == INTILE_BRIDGESWITCH)
		{
			HelpMessage("This switch activates a bridge.\n");
			gamestate.hint_switchbridge = true;
		}
#endif
		for (y=info_lo; y<info_lo+1; y++)	//leftover from KEEN (the loop is only run once here, so we don't need the loop construct here)
		{
			map = &FORESPOT(info_hi - (y!=info_lo), y);
			for (x=info_hi-(y!=info_lo); x<mapwidth; x++)
			{
				mapval = *(map++);
				manim = tinf[mapval+MANIM];
				if (!manim)
				{
					break;
				}
				mapval += manim;
				if (tinf[mapval+INTILE] == (INTILE_KILLPLAYER | INTILE_FOREGROUND))
				{
					mapval = 0;
				}
				RF_MemToMap(&mapval, 1, x, y, 1, 1);
			}
		}
	}
	else if (intileinfo == INTILE_KEYCARDSLOT //3
		|| intileinfo == INTILE_BLUESLOT //9
		|| intileinfo == INTILE_CYANSLOT //12
		|| intileinfo == INTILE_HOSTAGEKEYSLOT) //15
	{
		for (y=info_lo; y<mapheight; y++)
		{
			map = &FORESPOT(info_hi, y);
			mapval = *map;
			manim = tinf[mapval+MANIM];
			if (!manim)
			{
				break;
			}
			mapval = 0;
			RF_MemToMap(&mapval, 1, info_hi, y, 1, 1);
		}
#if (EPISODE == 1)
		if (gamestate.mapon == 10 && (intileinfo == INTILE_BLUESLOT || intileinfo == INTILE_CYANSLOT))
		{
			infoval = INFOSPOT(info_hi, info_lo-1);
			x = infoval >> 8;
			y = infoval & 0xFF;
			RF_MapToMap(x, y, info_hi-1, info_lo-5, 3, 2);
		}
#elif (EPISODE == 3)
		if (gamestate.mapon == 3 && (intileinfo == INTILE_BLUESLOT || intileinfo == INTILE_CYANSLOT))	// BUG: the check should only include INTILE_BLUESLOT, not INTILE_CYANSLOT in episode 3
		{
			infoval = INFOSPOT(info_hi, info_lo-1);
			x = infoval >> 8;
			y = infoval & 0xFF;
			RF_MapToMap(x, y, info_hi-1, info_lo-5, 3, 2);
		}
#endif
	}
	else if (intileinfo == INTILE_PLATFORMSWITCH //19
		|| intileinfo == INTILE_REDSLOT) //11
	{
#ifndef BETA
		if (!gamestate.hint_switchplat && intileinfo == INTILE_PLATFORMSWITCH)
		{
			HelpMessage("This switch activates a platform.\n");
			gamestate.hint_switchplat = true;
		}
#endif
		map = &INFOSPOT(info_hi, info_lo);
		mapval = *map;
		if (mapval >= DIRARROWSTART && mapval < DIRARROWEND)
		{
			*map = opposite[mapval-DIRARROWSTART] + DIRARROWSTART;	//change to opposite direction
			return;
		}
		else
		{
			*map = mapval ^ STOPICON;	//toggle between "stop" infotile and nothing (0)
		}
	}
	else if (intileinfo == INTILE_NUMBEREDSWITCH) //23
	{
#ifndef BETA
		if (!gamestate.hint_colorsequence)
		{
			HelpMessage("Find the color sequence in this level.\n");
			gamestate.hint_colorsequence = true;
		}
#endif
		map = &INFOSPOT(info_hi, info_lo);
		mapval = *map;
		colorseqnum++;
		if (info_hi-colorseqnum == 0 && colorseqnum != 5)
		{
			return;
		}
		else
		{
			if (colorseqnum == 5)
			{
				planeoffset = mapbwidthtable[ob->tilebottom]/2 + ob->tilemidx;
				infoval = mapsegs[2][planeoffset];
				info_hi = infoval >> 8;
				info_lo = infoval & 0xFF;
			}
			else
			{
				planeoffset = mapbwidthtable[ob->tiletop]/2 + ob->tilemidx;
				infoval = mapsegs[2][planeoffset];
				info_hi = infoval >> 8;
				info_lo = infoval & 0xFF;
				colorseqnum = 0;
			}
			for (y=info_lo; y<info_lo+1; y++)	//leftover from KEEN (the loop is only run once here, so we don't need the loop construct here)
			{
				map = &FORESPOT(info_hi - (y!=info_lo), y);
				for (x=info_hi-(y!=info_lo); x<mapwidth; x++)
				{
					mapval = *(map++);
					manim = tinf[mapval+MANIM];
					if (!manim)
					{
						break;
					}
					mapval += manim;
					RF_MemToMap(&mapval, 1, x, y, 1, 1);
				}
			}
			return;
		}
	}
}

//==========================================================================


/*
=================
=
= SnakeOpenDoorThink
=
=================
*/

//#pragma argsused
void SnakeOpenDoorThink(objtype *ob)
{
	Uint16 srcx, srcy, info;

#ifdef BETA
	if (!doorx || !doory)
	{
		return;
	}
#else
	if (doorx <= 0 || doory <= 0)
	{
		return;
	}
#endif
	if (FORESPOT(doorx, doory) != 0)
	{
		SD_PlaySound(OPENDOORSND);
		info = INFOSPOT(doorx, doory);
		srcx = info >> 8;
		srcy = info & 0xFF;
		RF_MapToMap(srcx, srcy, doorx-1, doory-1, 2, 3);
		info = INFOSPOT(doorx, doory);
		srcx = info >> 8;
		srcy = info & 0xFF;
		if (srcx == 1 && srcy != 0)
		{
			SpawnBonus(doorx, doory-1, srcy);
		}
		doorx = 0;
		doory = 0;
	}
}

//==========================================================================


/*
=================
=
= SnakeFireThink
=
=================
*/

void SnakeFireThink(objtype *ob)
{
#ifdef BETA
	if (gamestate.difficulty == gd_Easy)
	{
		gamestate.autofire = true;
		gamestate.ammo = 3;
		if (!ob->hitnorth)
		{
			ChangeState(player, &s_snakeairautofire1);
		}
		else
		{
			ChangeState(player, &s_snakestandautofire1);
		}
		return;
	}
	if (ob->xdir == 1)
	{
		DoShooting(ob->x+32*PIXGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->x+24*PIXGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
#else
	if (gamestate.difficulty == gd_Easy && gamestate.weapon == 0)
	{
		gamestate.autofire = true;
		gamestate.ammo = 3;
		if (!ob->hitnorth)
		{
			ChangeState(player, &s_snakeairautofire1);
		}
		else
		{
			ChangeState(player, &s_snakestandautofire1);
		}
		return;
	}
	if (ob->xdir == 1)
	{
		DoShooting(ob->right, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->left, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
#endif
}

//==========================================================================


/*
=================
=
= SnakeDuckFireThink
=
=================
*/

void SnakeDuckFireThink(objtype *ob)
{
#ifdef BETA
	if (gamestate.difficulty == gd_Easy)
	{
		gamestate.autofire = true;
		gamestate.ammo = 3;
		ChangeState(player, &s_snakeduckautofire1);
		return;
	}
	if (ob->xdir == 1)
	{
		DoShooting(ob->x+32*PIXGLOBAL, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->x+24*PIXGLOBAL, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
#else
	if (gamestate.difficulty == gd_Easy && gamestate.weapon == 0)
	{
		gamestate.autofire = true;
		gamestate.ammo = 3;
		ChangeState(player, &s_snakeduckautofire1);
		return;
	}
	if (ob->xdir == 1)
	{
		DoShooting(ob->right, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->left, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
#endif
}

//==========================================================================


/*
=================
=
= SnakeAutofireThink
=
=================
*/

void SnakeAutofireThink(objtype *ob)
{
	if (gamestate.ammo <= 0)
	{
		ChangeState(player, &s_snakestand);
		if (gamestate.difficulty == gd_Easy)
		{
			gamestate.ammo = 3;
		}
		return;
	}
	if (c.xaxis)
	{
		ob->xdir = c.xaxis;
	}
	if (!button1held)
	{
		ChangeState(player, &s_snakestand);
	}
	else if (c.button0 && !button0held)
	{
		if (justtransformed)
		{
			justtransformed = false;
		}
		ob->xspeed = 0;
		ob->yspeed = -41;
		xtry = 0;
		ytry = 0;
		jumptime = 16;
		ob->state = &s_snakeairautofire2;
		button0held = true;
		gamestate.riding = NULL;
		return;
	}
	else if (c.yaxis == 1)
	{
		ChangeState(player, &s_snakeduckautofire2);
	}
#ifdef BETA
	if (ob->xdir == 1)
	{
		DoShooting(ob->x+32*PIXGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->x+24*PIXGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
#else
	if (ob->xdir == 1)
	{
		DoShooting(ob->right, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->left, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
#endif
}

//==========================================================================


/*
=================
=
= SnakeAirAutofireThink
=
=================
*/

void SnakeAirAutofireThink(objtype *ob)
{
	if (gamestate.ammo <= 0)
	{
		ChangeState(player, &s_snakejump1);
		if (gamestate.difficulty == gd_Easy)
		{
			gamestate.ammo = 3;
		}
		return;
	}
	if (!button1held)
	{
		ChangeState(player, &s_snakejump1);
	}
#ifdef BETA
	if (ob->xdir == 1)
	{
		DoShooting(ob->x+32*PIXGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->x+24*PIXGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
#else
	if (ob->xdir == 1)
	{
		DoShooting(ob->right, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->left, ob->y+12*PIXGLOBAL, ob->xdir, 1);
	}
#endif
}

//==========================================================================


/*
=================
=
= SnakeDuckAutofireThink
=
=================
*/

void SnakeDuckAutofireThink(objtype *ob)
{
	if (gamestate.ammo <= 0)
	{
		ChangeState(player, &s_snakeduck);
		if (gamestate.difficulty == gd_Easy)
		{
			gamestate.ammo = 3;
		}
		return;
	}
	if (!button1held)
	{
		ChangeState(player, &s_snakeduck);
	}
	else if (c.button0 && !button0held)
	{
		if (justtransformed)
		{
			justtransformed = false;
		}
		ob->xspeed = 0;
		ob->yspeed = -41;
		xtry = 0;
		ytry = 0;
		jumptime = 16;
		ob->state = &s_snakeairautofire2;
		button0held = true;
		gamestate.riding = NULL;
		return;
	}
	else if (c.yaxis != 1)
	{
		ChangeState(player, &s_snakestandautofire2);
	}
	if (c.xaxis)
	{
		ob->xdir = c.xaxis;
	}
#ifdef BETA
	if (ob->xdir == 1)
	{
		DoShooting(ob->x+32*PIXGLOBAL, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->x+24*PIXGLOBAL, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
#else
	if (ob->xdir == 1)
	{
		DoShooting(ob->right, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
	else
	{
		DoShooting(ob->left, ob->y+28*PIXGLOBAL, ob->xdir, 1);
	}
#endif
}

//==========================================================================


/*
=================
=
= SnakeAirNofireThink
=
=================
*/

void SnakeAirNofireThink(objtype *ob)
{
	if (jumptime)
	{
		if (jumptime < tics)
		{
			ytry = ob->yspeed*jumptime;
			jumptime = 0;
		}
		else
		{
			ytry = ob->yspeed*tics;
			if (!jumpcheat)
			{
				jumptime = jumptime - tics;
			}
		}
		if (!c.button0)
		{
			jumptime = 0;
		}
	}
	else
	{
		DoGravity(ob);
	}
	if (c.xaxis)
	{
		ob->xdir = c.xaxis;
		AccelerateX(ob, c.xaxis*2, 24);
	}
	else
	{
		FrictionX(ob);
	}
}

//==========================================================================


/*
=================
=
= PollControls
=
=================
*/

void PollControls(void)
{
	upKeyDown = downKeyDown = leftKeyDown = rightKeyDown = false;
	IN_ReadControl(0, &c);
	if (c.xaxis == -1)
	{
		leftKeyDown = true;
	}
	if (c.xaxis == 1)
	{
		rightKeyDown = true;
	}
	if (c.yaxis == -1)
	{
		upKeyDown = true;
	}
	if (c.yaxis == 1)
	{
		downKeyDown = true;
	}
	button0down = c.button0;
	button1down = c.button1;
#ifndef BETA
	if (DemoMode != demo_Playback)
	{
		grenadeKeyDown = (Keyboard[grenadescan] && !g_keybind_used_grenade) || g_binding_value_grenade;
	}
#endif
	if (!upKeyDown)
	{
		upheld = false;
	}
	if (!button0down)
	{
		button0held = false;
	}
	if (!button1down)
	{
		button1held = false;
	}
#ifndef BETA
	if (!grenadeKeyDown)
	{
		throwkeyheld = false;
	}
#endif
}

//==========================================================================

#if (EPISODE == 2)

/*
=================
=
= SnakeWormStandActions
=
=================
*/

void SnakeWormStandActions(objtype *ob)
{
	boolean move;

	if (teleported)
	{
		teleported = false;
	}
	move = false;
	if (leftKeyDown)
	{
		if (ob->xdir == -1)
		{
			move = true;
		}
		else
		{
			ob->xdir = -1;
		}
	}
	if (rightKeyDown)
	{
		if (ob->xdir == 1)
		{
			move = true;
		}
		else
		{
			ob->xdir = 1;
		}
	}
	if (move)
	{
		xtry = ob->xdir << 4;
		ob->state = &s_snakewormwalk1;
	}
	else if (c.button0 && !button0held)
	{
		if (justtransformed)
		{
			justtransformed = false;
		}
		SD_PlaySound(CHOMPWALKSND);
		ob->xspeed = 0;
		ob->yspeed = -41;
		xtry = 0;
		ytry = 0;
		jumptime = 16;
		ob->state = &s_snakewormair1;
		button0held = true;
		gamestate.riding = NULL;
	}
	else if (c.button1 && !button1held)
	{
		ob->state = &s_snakewormattack;
		SD_PlaySound(BUGSCREAMSND);
		button1held = true;
		return;
	}
}

/*
=================
=
= SnakeWormStandThink
=
=================
*/

void SnakeWormStandThink(objtype *ob)
{
	if (c.dir != 8 || c.button0 || c.button1)
	{
		ob->temp1 = ob->temp2 = 0;
		ob->state = &s_snakewormstand;
		SnakeWormStandActions(ob);
	}
	else if ((ob->hitnorth & ~SLOPEMASK) != 24)	// if NOT riding a platform (*)
	{
		ob->temp1 = ob->temp1 + tics;
	}
	// (*) The Keen 4-6 code sets hitnorth to 25 when landing on an object,
	// therefore masking off the slope bits would yield 24 as the result.
	// BioMenace uses 23 instead of 25, which means (ob->hitnorth & ~SLOPEMASK)
	// will NEVER be 24.
}

/*
=================
=
= SnakeWormWalkThink
=
=================
*/

void SnakeWormWalkThink(objtype *ob)
{
	boolean move = false;

	if (leftKeyDown)
	{
		move = true;
		ob->xdir = -1;
	}
	if (rightKeyDown)
	{
		move = true;
		ob->xdir = 1;
	}
	if (!move)
	{
		ob->state = &s_snakewormstand;
	}
	else
	{
		if (c.button0 && !button0held)
		{
			if (justtransformed)
			{
				justtransformed = false;
			}
			SD_PlaySound(CHOMPWALKSND);
			ob->xspeed = ob->xdir << 4;
			ob->yspeed = -41;
			xtry = 0;
			ytry = 0;
			button0held = true;
			jumptime = 16;
			ob->state = &s_snakewormair1;
		}
		if (c.button1 && !button1held)
		{
			ob->state = &s_snakewormattack;
			SD_PlaySound(BUGSCREAMSND);
			button1held = true;
			return;
		}
	}
}

/*
=================
=
= SnakeWormAirThink
=
=================
*/

void SnakeWormAirThink(objtype *ob)
{
	if (jumptime)
	{
		if (jumptime < tics)
		{
			ytry = ob->yspeed * jumptime;
			jumptime = 0;
		}
		else
		{
			ytry = ob->yspeed * tics;
			if (!jumpcheat)
			{
				jumptime = jumptime - tics;
			}
		}
		if (!c.button0)
		{
			jumptime = 0;
		}
		if (!jumptime)
		{
			ob->temp2 = 0;
			ob->state = ob->state->nextstate;
		}
	}
	else
	{
		DoGravity(ob);
		if (ob->yspeed > 0 && ob->temp2 == 0)
		{
			ob->state = ob->state->nextstate;
			ob->temp2 = 1;
		}
	}
	if (c.xaxis)
	{
		ob->xdir = c.xaxis;
		AccelerateX(ob, c.xaxis<<1, 24);
	}
	else
	{
		FrictionX(ob);
	}
	if (c.button1 && !button1held)
	{
		ob->state = &s_snakewormairattack;
		SD_PlaySound(BUGSCREAMSND);
		button1held = true;
	}
}

#endif	// (EPISODE == 2)

//==========================================================================


/*
=================
=
= SnakeStartJump
=
=================
*/

void SnakeStartJump(objtype *ob)
{
	justtransformed = false;
	SD_PlaySound(SNAKEJUMPSND);
	ob->xspeed = 0;
	ob->yspeed = -41;
	xtry = 0;
	ytry = 0;
	jumptime = 16;
	button0held = true;
	gamestate.riding = NULL;
	ob->state = &s_snakejump1;
#ifdef BETA
	button0held = true;	// redundant
#endif
}

//==========================================================================


/*
=================
=
= SnakeShuttleThink
=
=================
*/

#ifdef BETA
void SnakeShuttleThink(objtype *ob)
{
	if (ob->hitnorth == PLATFORMEDGE && ob->state != &s_snakeshuttle)
	{
		ob->state = &s_snakeshuttle;
	}
	if (c.xaxis && CheckInteractiveTiles(ob))
	{
		gamestate.riding = NULL;
		shuttlecooldown = 50;
		ob->xdir = c.xaxis;
		ob->state = &s_snakewalk1;
		xtry = ob->xdir*s_snakewalk1.xmove*tics;
		SnakeWalkThink(ob);
	}
		
}
#endif

//==========================================================================


/*
=================
=
= HurtPlayer
=
=================
*/

void HurtPlayer(objtype *ob, Sint16 damage)
{
	if (godmode)
	{
		return;
	}
#if (EPISODE == 3)
	if (ob->health <= 0)
	{
		return;
	}
#endif
	if (gamestate.got_robopal)
	{
		gamestate.got_robopal = false;
	}
#ifndef BETA
	if (player->state == &s_snakedie1 || player->state == &s_snakedie2)
	{
		return;
	}
#endif
	ob->health = ob->health - damage;
	if (ob->health < 0)
	{
		ob->health = 0;
	}
	if (ob->health <= 0)
	{
		ChunkBloom(ob, ob->x, ob->y, 0);
		ChunkBloom(ob, ob->x, ob->y, 4);
		ChunkBloom(ob, ob->x, ob->y, 2);
		ChunkBloom(ob, ob->x, ob->y, 6);
		KillPlayer();
	}
	else
	{
		invincibility = 50;
		SD_PlaySound(SNAKEHITSND);
	}
}

//==========================================================================


/*
=================
=
= SnakeDieThink
=
=================
*/

void SnakeDieThink(objtype *ob)
{
	if (jumptime)
	{
		if (jumptime < tics)
		{
			ytry = ob->yspeed*jumptime;
			jumptime = 0;
		}
		else
		{
			ytry = ob->yspeed*tics;
			jumptime = jumptime - tics;
		}
	}
	else
	{
		DoGravity(ob);
	}
}

//==========================================================================


/*
=================
=
= SnakeDeadThink
=
=================
*/

//#pragma argsused
void SnakeDeadThink(objtype *ob)
{
	playstate = ex_died;
}

//==========================================================================


/*
=================
=
= SnakeShieldContact
=
=================
*/

#ifndef BETA
void SnakeShieldContact(objtype *ob, objtype *hit)
{
	if (hit->shootable && !hit->white
		&& hit->obclass != pushableobj	//BUG: should also handle blockedobj here!
		&& hit->obclass != fallingbrickobj)
	{
		HurtObject(hit, 5);
		SD_PlaySound(GRENADEXPLODESND);
		hit->white = 25;
	}
	ob++;	//probably just here to avoid "Parameter is never used" warning
}
#endif

//==========================================================================


/*
=================
=
= SnakePauseThink
=
=================
*/

void SnakePauseThink(objtype *ob)
{
	boolean move;
	Sint16 i;

	if (teleported)
	{
		teleported = false;
	}
#ifndef BETA
	if (shieldtime > 0)
	{
		SD_PlaySound(BLOBSND);
		ob->obclass = explosionobj;
		shieldtime--;
		if (shieldtime <= 0)
		{
			ChangeState(ob, &s_snakestand);
			ob->obclass = playerobj;
		}
		else
		{
			return;
		}
	}
#endif
	move = false;
	if (leftKeyDown)
	{
		if (ob->xdir == -1)
		{
			move = true;
		}
		else
		{
#ifndef BETA
			turncount++;
			upcount = 0;
			cooldown = 10;
#endif
			ob->xdir = -1;
		}
	}
	if (rightKeyDown)
	{
		if (ob->xdir == 1)
		{
			move = true;
		}
		else
		{
#ifndef BETA
			turncount++;
			upcount = 0;
			cooldown = 10;
#endif
			ob->xdir = 1;
		}
	}
#ifndef BETA
	if (upKeyDown && !upheld)
	{
		upcount++;
		if (upcount >= 75)
		{
			SD_PlaySound(BLOBSND);
			upcount = 75;
		}
		turncount = 0;
		cooldown = 3;
		if (button0down && !button0held)
		{
			cooldown = 25;
		}
	}
	if (downKeyDown && upcount >= 35)
	{
		invincibility = 99;
		cooldown = 0;
		upcount = 0;
	}
	if (cooldown <= 0)
	{
		turncount = 0;
		upcount = 0;
		cooldown = 0;
	}
	if (grenadeKeyDown && !throwkeyheld)
	{
		throwkeyheld = true;
		if (grenadeKeyDown)
		{
			if (gamestate.grenades.landmines > 0)
			{
				if (!gamestate.riding)
				{
					ob->state = &s_snakeplacemine;
				}
			}
			else
			{
				ob->state = &s_snakethrow1;
			}
			return;
		}
	}
#endif
	if (button0down && ! button0held && button1down && !button1held)
	{
#ifndef BETA
		upcount = 0;
#endif
		SnakeStartJump(ob);
		if (move)
		{
			ob->xspeed = ob->xdir << 4;
		}
		if (AUTOFIRE_ALLOWED)
		{
			ob->state = &s_snakeairautofire1;
		}
		else
		{
			ob->state = &s_snakeairfire1;
		}
		return;
	}
	if (button1down && ! button1held)
	{
		button1held = true;
#ifndef BETA
		if (turncount >= 5)
		{
			for (i=0; i<3; i++)
			{
				FragBloom(ob->midx, ob->y+8*PIXGLOBAL, FRAG_SIDEWAYS+dir_East);
				FragBloom(ob->midx, ob->y+8*PIXGLOBAL, FRAG_SIDEWAYS+dir_West);
				SD_PlaySound(LASERSND);
			}
			ChangeState(ob, &s_snakeskeleton);
			cooldown = 0;
			turncount = 0;
			return;
		}
		if (upcount >= 75)
		{
			if (ob->xdir == 1)
			{
				SpawnSuperLaserShot(ob->right, ob->y+12*PIXGLOBAL, ob->xdir);
			}
			else
			{
				SpawnSuperLaserShot(ob->left-3*TILEGLOBAL, ob->y+12*PIXGLOBAL, ob->xdir);
			}
			HurtPlayer(ob, 1);
			cooldown = 0;
			upcount = 0;
			return;
		}
		else
		{
			upcount = 0;
		}
#endif
		if (downKeyDown)
		{
			if (AUTOFIRE_ALLOWED)
			{
				ob->state = &s_snakeduckautofire1;
			}
			else
			{
				ob->state = &s_snakeduckfire1;
			}
			return;
		}
		if (upKeyDown)
		{
#ifndef BETA
			if (gamestate.grenades.landmines > 0)
			{
				if (!gamestate.riding)
				{
					ob->state = &s_snakeplacemine;
				}
			}
			else
#endif
			{
				ob->state = &s_snakethrow1;
			}
			return;
		}
		if (AUTOFIRE_ALLOWED)
		{
			ob->state = &s_snakestandautofire1;
		}
		else
		{
			ob->state = &s_snakestandfire1;
		}
		return;
	}
	if (button0down && !button0held)
	{
#ifndef BETA
		if (turncount >= 5)
		{
			ChangeState(ob, &s_snakeshield1);
			cooldown = 0;
			shieldtime = 45;
			turncount = 0;
			return;
		}
		upcount = 0;
#endif
		SnakeStartJump(ob);
		if (move)
		{
			ob->xspeed = ob->xdir << 4;
		}
		return;
	}
	if (upKeyDown)
	{
		if (CheckGrabPole(ob))
		{
			return;
		}
	}
	if (upKeyDown && !upheld)
	{
		if (CheckInteractiveTiles(ob))
		{
			upheld = true;
			return;
		}
	}
	if (move)
	{
#ifndef BETA
		upcount = 0;
#endif
		ob->state = &s_snakewalk1;
		xtry = ob->xdir << 4;
		return;
	}
	if (downKeyDown)
	{
#ifndef BETA
		upcount = 0;
#endif
		if (CheckGrabPole(ob))
		{
			return;
		}
		ob->state = &s_snakeduck;
		return;
	}
	ob->state = &s_snakestand;
}

//==========================================================================


/*
=================
=
= SnakeWalkThink
=
=================
*/

void SnakeWalkThink(objtype *ob)
{
	boolean move;

	if (teleported)
	{
		teleported = false;
	}
	move = false;
	if (leftKeyDown)
	{
		move = true;
		ob->xdir = -1;
	}
	if (rightKeyDown)
	{
		move = true;
		ob->xdir = 1;
	}
#ifndef BETA
	if (grenadeKeyDown && !throwkeyheld)
	{
		throwkeyheld = true;
		if (grenadeKeyDown)
		{
			if (gamestate.grenades.landmines > 0)
			{
				if (!gamestate.riding)
				{
					ob->state = &s_snakeplacemine;
				}
			}
			else
			{
				ob->state = &s_snakethrow1;
			}
			return;
		}
	}
#endif
	if (button0down && !button0held && button1down && !button1held)
	{
		SnakeStartJump(ob);
		ob->xspeed = ob->xdir << 4;
		if (AUTOFIRE_ALLOWED)
		{
			ob->state = &s_snakeairautofire1;
		}
		else
		{
			ob->state = &s_snakeairfire1;
		}
		return;
	}
	if (button1down && !button1held)
	{
		button1held = true;
		if (downKeyDown)
		{
			if (AUTOFIRE_ALLOWED)
			{
				ob->state = &s_snakeduckautofire1;
			}
			else
			{
				ob->state = &s_snakeduckfire1;
			}
			return;
		}
		if (upKeyDown)
		{
#ifndef BETA
			if (gamestate.grenades.landmines > 0)
			{
				if (!gamestate.riding)
				{
					ob->state = &s_snakeplacemine;
				}
			}
			else
#endif
			{
				ob->state = &s_snakethrow1;
			}
			return;
		}
		if (AUTOFIRE_ALLOWED)
		{
			ob->state = &s_snakestandautofire1;
		}
		else
		{
			ob->state = &s_snakestandfire1;
		}
		return;
	}
	if (button0down && !button0held)
	{
		SnakeStartJump(ob);
		if (move)
		{
			ob->xspeed = ob->xdir << 4;
		}
		return;
	}
	if (upKeyDown && !upheld)
	{
		if (CheckInteractiveTiles(ob))
		{
			upheld = true;
			return;
		}
	}
	if (!move)
	{
		if (downKeyDown)
		{
			ob->state = &s_snakeduck;
		}
		else
		{
			ob->state = &s_snakestand;
		}
		return;
	}
}

//==========================================================================


/*
=================
=
= SnakeAirThink
=
=================
*/

void SnakeAirThink(objtype *ob)
{
	if (jumptime)
	{
		if (jumptime < tics)
		{
			ytry = ob->yspeed * jumptime;
			jumptime = 0;
		}
		else
		{
			ytry = ob->yspeed * tics;
			if (!jumpcheat)
			{
				jumptime = jumptime - tics;
			}
		}
		if (!c.button0)
		{
			jumptime = 0;
		}
		if (!jumptime)
		{
			ob->temp2 = 0;
			ob->state = ob->state->nextstate;
		}
	}
	else
	{
		DoGravity(ob);
		if (ob->yspeed > 0 && ob->temp2 == 0)
		{
			ob->state = ob->state->nextstate;
			ob->temp2 = 1;
		}
	}
	if (c.xaxis)
	{
		ob->xdir = c.xaxis;
		AccelerateX(ob, c.xaxis*2, 24);
	}
	else
	{
		FrictionX(ob);
	}
#ifndef BETA
	if (grenadeKeyDown && !throwkeyheld)
	{
		throwkeyheld = true;
		if (grenadeKeyDown)
		{
			if (gamestate.grenades.landmines <= 0)
			{
				ob->state = &s_snakeairthrow1;
			}
			return;
		}
	}
#endif
	if (button1down && !button1held)
	{
		button1held = true;
		if (c.yaxis == -1)
		{
#ifdef BETA
			ob->state = &s_snakeairthrow1;
#else
			if (gamestate.grenades.landmines <= 0)
			{
				ob->state = &s_snakeairthrow1;
			}
			else
			{
				return;
			}
#endif
		}
		else if (AUTOFIRE_ALLOWED)
		{
			ob->state = &s_snakeairautofire1;
		}
		else
		{
			ob->state = &s_snakeairfire1;
		}
	}
	if (c.yaxis)
	{
		CheckGrabPole(ob);
	}
}

//==========================================================================


/*
=================
=
= SnakeLadderThink
=
=================
*/

void SnakeLadderThink(objtype *ob)
{
	Uint16 id0_far *map;
	
	if (PoleActions(ob))
	{
		return;
	}
	map = &FORESPOT(ob->temp4, ob->tiletop);
	switch (c.yaxis)
	{
	case -1:
		if (INTILETYPE(*map) != INTILE_POLE)
		{
			ob->state = &s_snakeclimb1;
		}
		else
		{
			ob->state = &s_snakeclimb2;
		}
		ob->ydir = -1;
		break;
	case 1:
		ob->state = &s_snakeclimb2;
		ob->ydir = 1;
		CheckPole(ob);
		break;
	}
}

//==========================================================================


/*
=================
=
= SnakeClimbThink
=
=================
*/

void SnakeClimbThink(objtype *ob)
{
	Uint16 id0_far *map;
	
	if (PoleActions(ob))
	{
		return;
	}
	map = &FORESPOT(ob->temp4, ob->tiletop);
	if (INTILETYPE(*map) != INTILE_POLE && c.yaxis != 1)
	{
		ytry = 0;
		ob->state = &s_snakeladder;
		return;
	}
	switch (c.yaxis)
	{
	case -1:
		ob->ydir = -1;
		break;
	case 0:
		ob->state = &s_snakeladder;
		ob->ydir = 0;
		break;
	case 1:
		ob->ydir = 1;
		CheckPole(ob);
		break;
	}
}

//==========================================================================


/*
=================
=
= CheckPole
=
=================
*/

void CheckPole(objtype *ob)
{
	Uint16 id0_far *map;

	if (PoleActions(ob))
	{
		return;
	}
	map = &FORESPOT(ob->temp4, ob->tilebottom);
	if (INTILETYPE(*map) != INTILE_POLE)
	{
		ob->state = &s_snakejump3;
		jumptime = 0;
		ob->temp2 = 1;
		ob->xspeed = polexspeed[c.xaxis+1];
		ob->yspeed = 0;
		ob->needtoclip = cl_midclip;	// leftover from Keen (Keen is noclipping on poles)
		ob->tilebottom--;
		return;
	}
}

//==========================================================================


/*
=================
=
= SnakeAttackThink
=
=================
*/

void SnakeAttackThink(objtype *ob)
{
	if (leftKeyDown)
	{
		ob->xdir = -1;
	}
	if (rightKeyDown)
	{
		ob->xdir = 1;
	}
	if (button0down && !button0held)
	{
		SnakeStartJump(ob);
		ob->state = &s_snakeairfire2;
		return;
	}
	if (downKeyDown)
	{
		ob->state = &s_snakeduckfire2;
		return;
	}
	ob->state = &s_snakestandfire2;
}

//==========================================================================


/*
=================
=
= KillPlayer
=
=================
*/

void KillPlayer(void)
{
	if (godmode)
	{
		return;
	}
#ifndef BETA
	if (player->state == &s_snakedie1 || player->state == &s_snakedie2)
	{
		return;
	}
#endif
	SD_PlaySound(SNAKEDIESND);
	gamestate.lives--;
	player->xspeed = 0;
	player->yspeed = -41;
	xtry = 0;
	ytry = 0;
	jumptime = 16;
	button0held = 1;
	gamestate.riding = NULL;
	snakedied = true;
	ChangeState(player, &s_snakedie1);
}

//==========================================================================


/*
=================
=
= SnakeStandContact
=
=================
*/

void SnakeStandContact(objtype *ob, objtype *hit)
{
	SnakeContact(ob, hit);
}

//==========================================================================

#if (EPISODE != 1)

/*
=================
=
= SnakeContact
=
=================
*/

void SnakeContact(objtype *ob, objtype *hit)
{
	switch (hit->obclass)
	{
	case bonusobj:
		switch (hit->temp1)	//temp1 is the infoplane icon number of the bonus object
		{
		case 9:
			SD_PlaySound(POWERUPSND);
			hit->shapenum = GRENADEGREEN1SPR;
			gamestate.grenades.green++;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
#ifndef BETA
			if (!gamestate.hint_grenade)
			{
				HelpMessage("Throw grenades to blow up monsters!\n");
				gamestate.hint_grenade = true;
			}
#endif
			break;
			
		case 16:
			SD_PlaySound(POWERUPSND);
			hit->shapenum = ITEMMACHINEGUN1SPR;
#ifdef BETA
			gamestate.autofire = true;
			if (gamestate.ammo > 0)
			{
				gamestate.clips++;
			}
			else if (gamestate.difficulty == gd_Hard)
			{
				gamestate.ammo = 45;
			}
			else
			{
				gamestate.ammo = 90;
			}
#else
			if (gamestate.ammo > 0 || gamestate.weapon != 0)
			{
				gamestate.clips++;
			}
			else
			{
				gamestate.autofire = true;
				if (gamestate.difficulty == gd_Hard)
				{
					gamestate.ammo = 45;
				}
				else
				{
					gamestate.ammo = 90;
				}
			}
#endif
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
#ifndef BETA
			if (!gamestate.hint_machinegun)
			{
				HelpMessage("Collect machine guns for automatic fire.\n");
				gamestate.hint_machinegun = true;
			}
#endif
			break;
			
		case 17:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMKEYCARD1SPR;
				gamestate.keys.keycards++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
#ifndef BETA
				if (!gamestate.hint_keycard)
				{
					HelpMessage("Use keycards to turn off electric beams.\n");
					gamestate.hint_keycard = true;
				}
#endif
			}
			break;
			
		case 18:
			SD_PlaySound(GETKEYSND);
			hit->shapenum = ITEMKEY1SPR;
			gamestate.keys.keys++;
			hit->obclass = decoobj;
			hit->priority = 3;
#ifndef BETA
			if (!gamestate.hint_key)
			{
				HelpMessage("Push <UP> and use key to open door.\n");
				gamestate.hint_key = true;
			}
#endif
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
			if (c.yaxis == -1)
			{
				SD_PlaySound(POWERUPSND);
				hit->shapenum = hit->temp1 + BONUS100SPR - 19;
				GivePoints(bonusscores[hit->temp1 - 19]);
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
			}
			break;
			
		case 25:
			if (c.yaxis == -1)
			{
				SD_PlaySound(EXTRASNAKESND);
				hit->shapenum = BONUS1UPSPR;
#ifdef BETA
				gamestate.lives++;
#else
				if (gamestate.lives < 9)	//BUG? you can't get more than 9 lives by picking up extra lives, but you can get more than 9 lives by collecting gems and getting points
				{
					gamestate.lives++;
				}
#endif
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
			}
			break;
			
		case 26:
#ifdef BETA
			if (c.yaxis == -1 || gamestate.mapon == 11)
#elif (EPISODE == 2)
			if (c.yaxis == -1 || gamestate.mapon == 6 || gamestate.mapon == 11)
#else
			if (c.yaxis == -1 || gamestate.mapon == 4 || gamestate.mapon == 11)
#endif
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMBLUESHARD1SPR;
				gamestate.gotshard[0]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
#ifndef BETA
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
#endif
			}
			break;
			
		case 27:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMGREENSHARD1SPR;
				gamestate.gotshard[1]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
#ifndef BETA
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
#endif
			}
			break;
			
		case 28:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMREDSHARD1SPR;
				gamestate.gotshard[2]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
#ifndef BETA
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
#endif
			}
			break;
			
		case 29:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMCYANSHARD1SPR;
				gamestate.gotshard[3]++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
#ifndef BETA
				if (!gamestate.hint_crystalshard)
				{
					HelpMessage("Find out where these crystal shards go.\n");
					gamestate.hint_crystalshard = true;
				}
#endif
			}
			break;
			
		case 30:
			if (c.yaxis == -1)
			{
				SD_PlaySound(GETKEYSND);
				hit->shapenum = ITEMSPECIALKEY1SPR;
				gamestate.specialkeys++;
				hit->obclass = decoobj;
				hit->priority = 3;
				ChangeState(hit, &s_bonusrise);
#ifndef BETA
				HelpMessage("Hmm....What a strange key!\n");
#endif
			}
			break;
			
#ifndef BETA
		case 36:
			hit->shapenum = ITEMREDGRENADE1SPR;
			SD_PlaySound(POWERUPSND);
			gamestate.grenades.red++;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_grenade)
			{
				HelpMessage("Throw grenades to blow up monsters!\n");
				gamestate.hint_grenade = true;
			}
			break;
			
		case 37:
			SD_PlaySound(GETKEYSND);
			hit->shapenum = ITEMSUPERGUN1SPR;
			gamestate.autofire = true;
			gamestate.weapon = 3;
			if (gamestate.ammo > 0)
			{
				gamestate.clips++;
			}
			if (gamestate.difficulty == gd_Hard)
			{
				gamestate.ammo = 45;
			}
			else
			{
				gamestate.ammo = 90;
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_supergun)
			{
				HelpMessage("Super Gun!  Bullets do 5x the damage!\n");	//BUG? super gun only does 2 damage
				gamestate.hint_supergun = true;
			}
			break;
			
		case 38:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMLANDMINES1SPR;
			if (gamestate.difficulty == gd_Hard)
			{
				gamestate.grenades.landmines = 5;
			}
			else
			{
				gamestate.grenades.landmines = 10;
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_landmine)
			{
				HelpMessage("Cool!  Land mines!\n");
				gamestate.hint_landmine = true;
			}
			break;
			
		case 39:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMPLASMABOLTS1SPR;
			gamestate.autofire = false;
			gamestate.weapon = 4;
			if (gamestate.ammo > 0)
			{
				gamestate.clips++;
			}
			if (gamestate.difficulty == gd_Hard)
			{
				gamestate.ammo = 10;
			}
			else
			{
				gamestate.ammo = 20;
			}
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_plasmabolts)
			{
				HelpMessage("Plasma Bolts!  Burn holes through enemies!\n");
				gamestate.hint_plasmabolts = true;
			}
			break;
#endif
		case 52:
			SD_PlaySound(YUMSND);
#ifdef BETA
			hit->shapenum = ITEMBEER1SPR;
			gamestate.food++;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (gamestate.food >= 10)
			{
				SD_PlaySound(LIFEPOINTSND);
				ob->health++;
				if (ob->health > 8)
				{
					ob->health = 8;
				}
				gamestate.food = 0;
			}
#else
			hit->shapenum = ITEMGEM1SPR;
			gamestate.gems++;
			hit->obclass = decoobj;
			hit->priority = 3;
			if (gamestate.gems == 50)
			{
				gamestate.lives++;
				gamestate.gems = 0;
				SD_PlaySound(EXTRASNAKESND);
				hit->shapenum = BONUS1UPSPR;
			}
			ChangeState(hit, &s_bonusrise);
			if (!gamestate.hint_gem)
			{
				HelpMessage("Collect 50 gems for an extra life!\n");
				gamestate.hint_gem = true;
			}
#endif
			break;
			
		case 53:
			SD_PlaySound(YUMSND);	//probably useless, except in the beta
#ifdef BETA
			hit->shapenum = ITEMMEAL1SPR;
			gamestate.food += 2;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			if (gamestate.food >= 10)
			{
				SD_PlaySound(LIFEPOINTSND);
				ob->health++;
				gamestate.food = 0;
			}
#else
			hit->shapenum = ITEMFIRSTAID1SPR;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			SD_PlaySound(LIFEPOINTSND);
			switch (gamestate.difficulty)
			{
			case gd_Easy:
				player->health = 8;
				break;
			case gd_Normal:
				player->health = 4;
				break;
			case gd_Hard:
				player->health = 2;	//BUG: player starts with 4 health on hard mode, so picking up a first aid kit might reduce player's health!
				break;
			default:
				player->health = 4;
				break;
			}
			gamestate.health = player->health;
			if (!gamestate.hint_firstaid)
			{
				HelpMessage("First Aid Kit restores you to full health.\n");
				gamestate.hint_firstaid = true;
			}
#endif
			break;
			
#ifndef BETA
		case 54:
			SD_PlaySound(EXTRASNAKESND);
			hit->shapenum = ITEMWARPGEM1SPR;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			gamestate.got_warpgem = true;
			if (!gamestate.hint_warpgem)
			{
				HelpMessage("You found a secret level warp gem!\n");
				gamestate.hint_warpgem = true;
			}
			break;
#endif
		case 75:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMNUKE1SPR;
			gamestate.nukestate = 1;
			hit->obclass = decoobj;
			hit->priority = 3;
#ifdef BETA
			VW_FixRefreshBuffer();
			US_CenterWindow(35, 2);
			PrintY += 2;
			US_CPrint("Oh look! What a cute little nuclear bomb!\n");
			VW_UpdateScreen();
			VW_WaitVBL(60);
			IN_ClearKeysDown();
			IN_Ack();
			RF_ForceRefresh();
#else
			HelpMessage("Oh look! What a cute little nuclear bomb!\n");
#endif
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 76:
			SD_PlaySound(YUMSND);
			hit->shapenum = ITEMANTIRADIATIONPILL1SPR;
			gamestate.got_pill = true;
			hit->obclass = decoobj;
			hit->priority = 3;
#ifdef BETA
			VW_FixRefreshBuffer();
			US_CenterWindow(30, 3);
			PrintY += 2;
			US_CPrint("This must be the anti-radiation pill!\n");
			VW_UpdateScreen();
			VW_WaitVBL(60);
			IN_ClearKeysDown();
			IN_Ack();
			RF_ForceRefresh();
#else
			HelpMessage("It's an anti-radiation pill!\n");
#endif
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 77:
			SD_PlaySound(EGGSND);
			hit->shapenum = ITEMINVINCIBILITYPOTION1SPR;
			gamestate.potions++;
#ifdef BETA
			if (gamestate.potions > 24)
			{
				gamestate.potions = 0;
				invincibility = 1500;
				SD_PlaySound(INVISOSND);
			}
#else
			if (gamestate.potions >= 1)
			{
				gamestate.potions = 0;
				invincibility = 1500;
				StartMusic(MUS_INVINCIBLE);
			}
#endif
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
#ifndef BETA
			if (!gamestate.hint_invincible)
			{
				HelpMessage("Now you're invincible to monster attacks!\n");
				gamestate.hint_invincible = true;
			}
#endif
			break;
			
		case 78:
			SD_PlaySound(ICON5000SND);
			hit->shapenum = BONUS5000SPR;
			GivePoints(5000);
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 79:
			SD_PlaySound(ICON5000SND);
			hit->shapenum = BONUS50000SPR;
			GivePoints(50000);
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			break;
			
		case 80:
			SD_PlaySound(GETKEYSND);
			gamestate.got_exitkey = true;
			hit->shapenum = ITEMEXITKEY1SPR;
			hit->obclass = decoobj;
			hit->priority = 3;
			ChangeState(hit, &s_bonusrise);
			break;
		}
		break;	//end of case bonusobj
		
	case platformobj:
		if (!gamestate.riding)
		{
			ClipToSpriteTop(ob, hit);
		}
		break;
		
#ifdef BETA
	case shuttleobj:
		if (!gamestate.riding && !shuttlecooldown)
		{
			// I don't think this works correctly. I would call ClipToSpriteTop()
			// first and check if that sets gamestate.riding to hit. If it does,
			// then I would adjust the x position BEFORE calling ChangeState().
			ChangeState(ob, &s_snakeshuttle);
			ob->x = hit->x+12*PIXGLOBAL;
			ClipToSpriteTop(ob, hit);
		}
		break;
#endif

	case hostageobj:
		gamestate.savedhostage = true;
		HostageDialog();
		SD_PlaySound(POWERUPSND);
		hit->shapenum = BONUS5000SPR;
		GivePoints(bonusscores[BONUS5000SPR-BONUS100SPR]);
		hit->obclass = bonusobj;
		hit->ydir = -1;
		ChangeState(hit, &s_bonusrise);
		SpawnBonus(ob->tilemidx, ob->tiletop, 80);	//spawn exit key
		break;
		
#ifndef BETA
	case triangleobj:	//never actually used!
		gamestate.trianglekey++;
		SD_PlaySound(POWERUPSND);
		hit->shapenum = ITEMGREENTRIANGLE2SPR;
		hit->obclass = bonusobj;	//BUG: this should probably be set to 'decoobj'
		hit->ydir = -1;
		ChangeState(hit, &s_bonusrise);
		break;
#endif

	case robopalobj:
		if (!gamestate.got_robopal && (!invincibility || invincibility > 50))
		{
			SD_PlaySound(ICON5000SND);
			gamestate.got_robopal = true;
			ChangeState(hit, &s_robopalfollow1);
		}
#ifndef BETA
		if (!gamestate.hint_robopal)
		{
			HelpMessage("RoboPal gives you extra fire power!\n");
			gamestate.hint_robopal = true;
		}
#endif
		break;
		
	case tankobj:
#if (EPISODE == 3)
	case caterpillarobj:
	case goliathobj:
	case cainobj:
#endif
		HurtPlayer(ob, 50);	//kill the player
		break;
		
	case pushableobj:
#ifdef BETA
		ClipPushSprite(ob, hit, false);
#else
		if (gamestate.mapon != 13)
		{
			ClipPushSprite(ob, hit, false);
		}
#endif
		break;
		
	case blockedobj:
		ClipToSprite(ob, hit, false);
		break;
		
	case fallingbrickobj:
		ClipToSprite(ob, hit, true);
		break;
		
	case trashtruckobj:
		ClipToSpriteSide(ob, hit);
		break;
		
	case explosionobj:
	case wormobj:
	case kamikazeobj:
	case bugobj:
	case beeobj:
	case bluedevilobj:
#if (EPISODE == 2)
	case trashbossobj:
	case antobj:
	case caterpillarobj:
	case queenobj:
#else
	case hairyobj:
	case jumpbombobj:
	case slimerobj:
	case devilobj:
	case cyborgobj:
	case grenadebotobj:
#endif
	case punchyobj:
	case gorpobj:
	case cobraobj:
	case guardobj:
	case bouncerobj:
	case acidmonsterobj:
	case enemyshotobj:
	case spikeyobj:
#if (EPISODE == 2)
		if ((ob->state == &s_snakewormattack || ob->state == &s_snakewormairattack)
			&& hit->shootable && !hit->white)
		{
			HurtObject(hit, 1);
			break;
		}
#endif
#ifdef BETA
		if (hit->obclass == explosionobj && gamestate.difficulty == gd_Easy)
		{
			break;
		}
#else
		if (hit->spareplayer == true)
		{
			break;
		}
#endif
		if (!invincibility)
		{
			HurtPlayer(ob, 1);
		}
		break;
	}
}
#endif	// (EPISODE != 1)

//==========================================================================


#if (EPISODE == 2)

/*
=================
=
= SnakeWormStandReact
=
=================
*/

void SnakeWormStandReact(objtype *ob)
{
	if (!ob->hitnorth)
	{
		ob->xspeed = ob->xdir * 8;
		ChangeState(ob, &s_snakewormair3);
		ob->temp2 = 1;
		jumptime = 0;
	}
	PLACESPRITE;
}

/*
=================
=
= SnakeWormWalkReact
=
=================
*/

void SnakeWormWalkReact(objtype *ob)
{
	if (!ob->hitnorth)
	{
		ob->xspeed = ob->xdir * 8;
		ob->yspeed = 0;
		ChangeState(ob, &s_snakewormair3);
		ob->temp2 = 1;
		jumptime = 0;
	}
	if (ob->hiteast || ob->hitwest)
	{
		ChangeState(ob, &s_snakewormstand);
	}
	PLACESPRITE;
}

/*
=================
=
= SnakeWormAirReact
=
=================
*/

void SnakeWormAirReact(objtype *ob)
{
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = 0;
	}
	if (ob->hitsouth)
	{
		if (ob->hitsouth > 1)
		{
			ob->yspeed += 16;
			if (ob->yspeed < 0)
			{
				ob->yspeed = 0;
			}
		}
		else
		{
			ob->yspeed = 0;
		}
		jumptime = 0;
	}
	if (ob->hitnorth && (ob->hitnorth != PLATFORMEDGE || jumptime == 0))
	{
		ob->temp1 = ob->temp2 = 0;
		ChangeState(ob, &s_snakewormstand);
	}
	PLACESPRITE;
}

#endif	// (EPISODE == 2)

//==========================================================================


/*
=================
=
= SnakeStandReact
=
=================
*/

void SnakeStandReact(objtype *ob)
{
	if (!ob->hitnorth)
	{
		ob->xspeed = ob->xdir*8;
		ChangeState(ob, &s_snakejump3);
		ob->temp2 = 1;
		jumptime = 0;
	}
	PLACESPRITE;
}

/*
=================
=
= SnakeWalkReact
=
=================
*/

void SnakeWalkReact(objtype *ob)
{
	if (!ob->hitnorth)
	{
		ob->xspeed = ob->xdir * 8;
		ob->yspeed = 0;
		ChangeState(ob, &s_snakejump3);
		ob->temp2 = 1;
		jumptime = 0;
	}
	else if (ob->hiteast || ob->hitwest)
	{
		ChangeState(ob, &s_snakestand);
	}
	PLACESPRITE;
}

/*
=================
=
= SnakeAirReact
=
=================
*/

void SnakeAirReact(objtype *ob)
{
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = 0;
	}
	if (ob->hitsouth)
	{
		if (ob->hitsouth > 1)
		{
			ob->yspeed += 16;
			if (ob->yspeed < 0)
			{
				ob->yspeed = 0;
			}
		}
		else
		{
			ob->yspeed = 0;
		}
		jumptime = 0;
	}
	if (ob->hitnorth)
	{
		if (ob->hitnorth != PLATFORMEDGE || jumptime == 0)
		{
			ob->temp1 = ob->temp2 = 0;
			if (ob->state == &s_snakeairthrow1)
			{
				ChangeState(ob, &s_snakethrow1);
			}
			else if (ob->state == &s_snakeairthrow2)
			{
				ChangeState(ob, &s_snakethrow2);
			}
			else if (ob->state == &s_snakeairthrow3)
			{
				ChangeState(ob, &s_snakethrow3);
			}
			else if (ob->state == &s_snakeairfire1)
			{
				ChangeState(ob, &s_snakestandfire1);
			}
			else if (ob->state == &s_snakeairfire2)
			{
				ChangeState(ob, &s_snakestandfire2);
			}
			else if (ob->state == &s_snakeairfire3)
			{
				ChangeState(ob, &s_snakestandfire3);
			}
			else if (ob->state == &s_snakeairautofire1)
			{
				ChangeState(ob, &s_snakestandautofire1);
			}
			else if (ob->state == &s_snakeairautofire2)
			{
				ChangeState(ob, &s_snakestandautofire2);
			}
			else
			{
				ChangeState(ob, &s_snakestand);
			}
		}
	}
	PLACESPRITE;
}

/*
=================
=
= SnakeDieReact
=
=================
*/

void SnakeDieReact(objtype *ob)
{
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_snakedie2);
	}
	PLACESPRITE;
}

//==========================================================================


/*
=================
=
= CheckInTiles
=
=================
*/

void CheckInTiles(objtype *ob)
{
	Uint16 tilex, tiley;
	Uint16 id0_far *map;
	Uint16 skipwidth, intileinfo;
	
#ifndef BETA
	if (player->health <= 0)
	{
		return;
	}
#endif
	map = &FORESPOT(ob->tileleft, ob->tiletop);
	skipwidth = mapwidth-(ob->tileright-ob->tileleft+1);
	for (tiley=ob->tiletop; tiley<=ob->tilebottom; tiley++, map+=skipwidth)
	{
		for (tilex=ob->tileleft; tilex<=ob->tileright; tilex++, map++)
		{
			intileinfo = INTILETYPE(*map);
			if (intileinfo == 0)
			{
				continue;
			}
			switch (intileinfo)
			{
			case INTILE_GREENTRIANGLE: //4:
			case INTILE_BETADOOR: //5:
			case INTILE_DOOR: //6:
			case INTILE_SECRETDOOR: //8:
#ifndef BETA
				if (!gamestate.hint_opendoor)
				{
					HelpMessage("Push <UP> to enter or open a door.\n");
					gamestate.hint_opendoor = true;
				}
#endif
				doorx = tilex;
				doory = tiley;
				break;
				
			case INTILE_HURTPLAYER: //2:
				if (ob->health > 0)
				{
					if (!invincibility)
					{
						HurtPlayer(ob, 1);
					}
				}
				else
				{
					return;
				}
				break;
				
			case INTILE_KILLPLAYER: //16:
				if (ob->health > 0)
				{
					HurtPlayer(ob, 50);	//kill the player
				}
				else
				{
					return;
				}
				break;
				
#ifdef BETA
			case INTILE_TOWORM: //24:
				if (ob->obclass == playerwormobj && ob->state == &s_snakewormair3 && !justtransformed)
				{
					ob->obclass = playerobj;
					ChangeState(ob, &s_snakestand);
					SD_PlaySound(INVISOSND);
					justtransformed = true;
				}
				else if (ob->state == &s_snakejump3 && !justtransformed)
				{
					ob->obclass = playerwormobj;
					ChangeState(ob, &s_snakewormstand);
					SD_PlaySound(INVISOSND);
					justtransformed = true;
				}
				break;
				
#elif (EPISODE == 2)
			case INTILE_TOHUMAN: //26:
				if (ob->obclass == playerwormobj && !justtransformed)
				{
					ob->obclass = playerobj;
					ChangeState(ob, &s_snakestand);
					SD_PlaySound(INVISOSND);
					justtransformed = true;
				}
				break;
				
			case INTILE_TOWORM: //24:
				if (ob->obclass == playerobj && !justtransformed)
				{
					ob->obclass = playerwormobj;
					ChangeState(ob, &s_snakewormstand);
					SD_PlaySound(INVISOSND);
					justtransformed = true;
				}
				break;
#endif
				
			case INTILE_RADIATION: //13:
				if (gamestate.got_pill == true)
				{
					return;
				}
				if (ob->health > 0)
				{
					HurtPlayer(ob, 50);	//kill the player
				}
				else
				{
					return;
				}
				break;
				
			case INTILE_BOSSTRIGGER: //21:
				if (ob->hitnorth == 1)
				{
					CheckInteractiveTiles(ob);
				}
				break;
				
			case INTILE_HINTTRIGGER: //22:
				if (hintstate == 3)
				{
					return;
				}
				if (ob->hitnorth == 1 && (hintstate == 0 || Keyboard[sc_UpArrow]))
				{
					CheckInteractiveTiles(ob);
				}
				break;
				
			case INTILE_SPIKETRIGGER: //25:
				if (crusherstate != -1)
				{
					crusherstate = 1;
				}
				break;
			}
		}
	}
}

//==========================================================================


/*
=================
=
= HandleRiding
=
=================
*/

void HandleRiding(objtype *ob)
{
	objtype *plat = gamestate.riding;

	if (ob->right < plat->left || ob->left > plat->right)
	{
		//
		// player is too far left or right and no longer standing on the platform:
		//
		gamestate.riding = NULL;
		return;
	}
	if (ob->ymove < 0)
	{
		//
		// player has moved up, probably jumped off the platform:
		//
		gamestate.riding = NULL;
		if (plat->ymove < 0)
		{
			xtry = 0;
			ytry = plat->ymove;
			PushObj(ob);
		}
		return;
	}
	
	//
	// player is still on the platform:
	//
	xtry = plat->xmove;
	ytry = plat->top - ob->bottom - 16;
	PushObj(ob);

	// The following code aligns the player's position with the platform's
	// position. The screen scrolls in global units (fractions of a pixel),
	// which could cause the player and the platform sprite to move back and
	// forth in relation to each other when the player stands on a platform and
	// the movement causes the screen to scroll sideways. Aligning the positions
	// on the same fraction of on-screen movement avoids this problem.
	
	// BUG: This code always applies the alignment, even when the player is
	// walking left/right on the platform or the platform didn't move sideways,
	// which may interfere with the player's movement -- especially in NOPAN
	// mode.
	
	if (nopan)	//condition is always false in v1.1
	{
		// The NOPAN parameter is an undocumented feature in Keen 4-6 v1.4 (and
		// Keen 4 v1.2) that allows these games to run a little less glitchy on
		// systems that don't support pixel panning in hardware (an EXTREMELY
		// rare situation). Sprites can only move in multiples of 8 pixels left/
		// right when the NOPAN parameter was used in Keen 4-6, but that behavior
		// isn't implemented in BioMenace's version of ID_RF.C. This made using
		// NOPAN pretty much pointless for BioMenace, which could explain why the
		// parameter was removed and replaced with LATCHPEL in v1.1.
		ob->x &= ~0x7F;
		ob->x |= plat->x & 0x7F;
	}
	else
	{
		// EGA sprites can only move in multiples of 2 pixels left/right
		// (assuming the sprite's shift values are set to 4).
		ob->x |= plat->x & 0x1F;
		// BUG: The platform's alignment is applied without clearing the bits of
		// the object's x position first. This doesn't cause any major problems
		// in the original games, but it would need to be fixed when modifying
		// the game to run at up to 70 fps.
	}
	
	//
	// make ob fall off if the platform pushed it against the ceiling:
	//
	if (ob->hitsouth)
	{
		gamestate.riding = NULL;
		return;
	}
	ob->hitnorth = PLATFORMEDGE;
}

REFKEEN_NS_E
