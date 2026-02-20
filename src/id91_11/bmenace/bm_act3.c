/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
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
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

#if (EPISODE == 1)
Sint16 skullmanstate = -2;
#endif

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// BIG EXPLOSION
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_bigexplosion1 = {BIGEXPLOSION1SPR, BIGEXPLOSION1SPR, step, false, push_none, 15, 0, 0, NULL, NULL, DrawReact, &s_bigexplosion2};
FARSTATE s_bigexplosion2 = {BIGEXPLOSION2SPR, BIGEXPLOSION2SPR, step, false, push_none, 12, 0, 0, NULL, NULL, DrawReact, &s_bigexplosion3};
FARSTATE s_bigexplosion3 = {BIGEXPLOSION3SPR, BIGEXPLOSION3SPR, step, false, push_none, 5, 0, 0, NULL, NULL, DrawReact, NULL};

/*
==================
=
= ExplodeThink
=
==================
*/

void ExplodeThink(objtype *ob)
{
	Uint16 posx, posy;

	if (--ob->temp1 > 0)
	{
		if (US_RndT() < 175)
		{
			posx = ob->midx - CONVERT_PIXEL_TO_GLOBAL(US_RndT() / 16);
		}
		else
		{
			posx = ob->midx + CONVERT_PIXEL_TO_GLOBAL(US_RndT() / 16);
		}
		posy = ob->top + CONVERT_PIXEL_TO_GLOBAL(US_RndT() / 16);
		if (US_RndT() < 175)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
		SpawnBigExplosion(posx, posy);
		ChunkBloom(ob, ob->midx, ob->y, 2);
		ChunkBloom(ob, ob->midx, ob->y, 6);
		SD_PlaySound(RAYHITSND);
		ob->nothink = US_RndT() / 16;
	}
	else
	{
		SD_PlaySound(GROWLSND);
		SpawnBigExplosion(ob->midx, ob->y + TILEGLOBAL);
#if (EPISODE == 3)
		if (ob->obclass == cainobj)
		{
			ob->temp7 = 100;	// leftover from earlier versions that used CainDeadThink
			SD_PlaySound(GROWLSND);	//redundant (already played above)
			ob->state = &s_caindead1;
			ob->obclass = decoobj;
			ob->hitnorth = 1;
			ob->y += 2*TILEGLOBAL;
			StartMusic(MUS_VICTORY);
			// Note: The original IMF music data contains some instructions that
			// interfere with the playback of AdLib sound effects. The IMF data
			// overwrites the instrument data for the first OPL channel (the one
			// reserved for AdLib sound effects), which means any AdLib sound
			// effect that is playing when a music track was just (re-)started will
			// use the wrong instrument data for the rest of its duration. Since
			// this code starts a new music track right after playing a sound
			// effect, the music can prevent that effect from being played
			// correctly when the game is set to AdLib sound effects. The same
			// problem also occurs when the music loops around to the start, so
			// this is really a bug in the data, not a bug in the code.
			return;
		}
#endif
		RemoveObj(ob);
	}
}

/*
==================
=
= SpawnBigExplosion
=
==================
*/

void SpawnBigExplosion(Uint16 x, Uint16 y)
{
	GetNewObj(true);
	newobj->obclass = decoobj;
	newobj->active = allways;
	newobj->x = x;
	newobj->y = y;
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->needtoclip = cl_noclip;
	newobj->priority = 3;
	NewState(newobj, &s_bigexplosion1);
}

/*
==================
=
= SpawnEnemyShot
=
==================
*/

Sint16 SpawnEnemyShot(Uint16 x, Uint16 y, FARSTATE *state)
{
	if (GetNewObj(true) == -1)
	{
		return -1;
	}
	newobj->x = x;
	newobj->y = y;
	newobj->obclass = enemyshotobj;
	newobj->active = allways;
	NewState(newobj, state);
	if (!CheckPosition(newobj))
	{
		RemoveObj(newobj);
		return -1;
	}
	return 0;
}

#if (EPISODE == 2)
//////////////////////////////////////////////////////////////////////////////
//
// ANT
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_antwalk1   = {ANTWALK1LSPR, ANTWALK1RSPR, stepthink, false, push_down, 5, 64, 0, AntWalkThink, NULL, AntWalkReact, &s_antwalk2};
FARSTATE s_antwalk2   = {ANTWALK2LSPR, ANTWALK2RSPR, stepthink, false, push_down, 5, 64, 0, AntWalkThink, NULL, AntWalkReact, &s_antwalk3};
FARSTATE s_antwalk3   = {ANTWALK3LSPR, ANTWALK3RSPR, stepthink, false, push_down, 5, 64, 0, AntWalkThink, NULL, AntWalkReact, &s_antwalk4};
FARSTATE s_antwalk4   = {ANTWALK2LSPR, ANTWALK2RSPR, stepthink, false, push_down, 5, 64, 0, AntWalkThink, NULL, AntWalkReact, &s_antwalk1};
FARSTATE s_antattack1 = {ANTATTACK1LSPR, ANTATTACK1RSPR, step, true, push_down, 15, 0, 0, NULL, NULL, DrawReact, &s_antattack2};
FARSTATE s_antattack2 = {ANTATTACK2LSPR, ANTATTACK2RSPR, step, true, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_antjump};
FARSTATE s_antjump    = {ANTWALK1LSPR, ANTWALK1RSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, AntJumpReact, &s_antwalk1};

/*
==================
=
= SpawnAnt
=
==================
*/

void SpawnAnt(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = antobj;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 40);
	newobj->xdir = 1;
	newobj->ydir = 1;
	NewState(newobj, &s_antwalk1);
	newobj->hitnorth = 1;
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 3;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 6;
		break;
	default:
		newobj->health = 3;
		break;
	}
#endif
}

/*
==================
=
= AntWalkReact
=
==================
*/

void AntWalkReact(objtype *ob)
{
	if (ob->xdir == 1 && ob->hitwest)
	{
		ob->xdir = -1;
	}
	else if (ob->xdir == -1 && ob->hiteast)
	{
		ob->xdir = 1;
	}
	else if (!ob->hitnorth)
	{
		if (ob->top > player->bottom || ob->bottom < player->top)
		{
			if (ob->xdir == 1)
			{
				ob->xdir = -1;
			}
			else
			{
				ob->xdir = 1;
			}
			return;	//BUG? doesn't place a new sprite!
		}
		ob->x -= ob->xmove;
		ob->y -= ob->ymove;
		ob->yspeed = -((US_RndT()/32) * 48);
		if (ob->yspeed < -60)
		{
			ob->yspeed = -60;
		}
		if (ob->yspeed > -30)
		{
			ob->yspeed = -30;
		}
		ob->xspeed = ob->xdir << 5;
		ChangeState(ob, &s_antjump);
	}
	PLACESPRITE;
}

/*
==================
=
= AntWalkThink
=
==================
*/

void AntWalkThink(objtype *ob)
{
	Sint16 xdist;

	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	if (ob->xdir == -1)
	{
		xdist = ob->left - player->right;
		if (xdist > 4*TILEGLOBAL)
		{
			return;
		}
		if (xdist < -TILEGLOBAL)
		{
			ob->xdir = 1;
			ob->yspeed = -48 - US_RndT()/16;
			ob->xspeed = 32;
			return;
		}
	}
	else
	{
		xdist = player->left - ob->right;
		if (xdist > 4*TILEGLOBAL)
		{
			return;
		}
		if (xdist < -TILEGLOBAL)
		{
			ob->xdir = -1;
			ob->yspeed = -48 - US_RndT()/16;
			ob->xspeed = -32;
			return;
		}
	}
	if (player->right < ob->left || player->left > ob->right
		|| player->top > ob->bottom || player->bottom < ob->top)
	{
		return;
	}
	ob->state = &s_antattack1;
}

/*
==================
=
= AntJumpReact
=
==================
*/

void AntJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_antwalk1);
		ob->nothink = 12;
	}
	PLACESPRITE;
}
#endif	// (EPISODE == 2)

//////////////////////////////////////////////////////////////////////////////
//
// TANK
//
// temp1: number of explosions to spawn during death animation
// temp7: turn cooldown for WalkReact
//
// SMALL ROCKET
//
// temp1: delay until starting to fall / delay until it explodes
//
// SMALL FIRE
//
// temp7: time to live
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_tankdie          = {TANK1SPR, TANK1SPR, stepthink, false, push_down, 6, 0, 0, ExplodeThink, NULL, DrawReact, &s_tankdie};
FARSTATE s_tankstand        = {TANK1SPR, TANK1SPR, step, false, push_down, 10, 0, 0, TankStandThink, NULL, DrawReact, &s_tankstand};
FARSTATE s_tankwalk1        = {TANK1SPR, TANK1SPR, slide, false, push_down, 20, 8, 0, NULL, NULL, WalkReact, &s_tankwalk2};
FARSTATE s_tankwalk2        = {TANK2SPR, TANK2SPR, slide, false, push_down, 20, 8, 0, TankWalkThink, NULL, WalkReact, &s_tankwalk1};
FARSTATE s_tankattack1      = {TANK3SPR, TANK3SPR, step, false, push_down, 7, 0, 0, TankAttack1Think, NULL, DrawReact, &s_tankstand};
FARSTATE s_tankattack2      = {TANK1SPR, TANK1SPR, step, false, push_down, 7, 0, 0, TankAttack2Think, NULL, DrawReact, &s_tankstand};
FARSTATE s_smallrocketfly   = {SMALLROCKETFLYSPR, SMALLROCKETFLYSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, SmallrocketFlyReact, &s_smallrocketfly};
FARSTATE s_smallrocketfall  = {SMALLROCKETFALLSPR, SMALLROCKETFALLSPR, stepthink, false, push_none, 5, 8, -32, NULL, NULL, SmallrocketFallReact, &s_smallrocketfall};
FARSTATE s_tankshot         = {TANKSHOTSPR, TANKSHOTSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, TankshotReact, &s_tankshot};
FARSTATE s_tankshotexplode1 = {MEDIUMEXPLOSION1SPR, MEDIUMEXPLOSION1SPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, &s_tankshotexplode2};
FARSTATE s_tankshotexplode2 = {MEDIUMEXPLOSION2SPR, MEDIUMEXPLOSION2SPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, NULL};
FARSTATE s_smallfire1       = {SMALLFIRE1SPR, SMALLFIRE1SPR, step, true, push_down, 5, 0, 0, SmallfireThink, NULL, DrawReact, &s_smallfire2};
FARSTATE s_smallfire2       = {SMALLFIRE2SPR, SMALLFIRE2SPR, step, true, push_down, 5, 0, 0, SmallfireThink, NULL, DrawReact, &s_smallfire3};
FARSTATE s_smallfire3       = {SMALLFIRE3SPR, SMALLFIRE3SPR, step, true, push_down, 5, 0, 0, SmallfireThink, NULL, DrawReact, &s_smallfire1};

/*
==================
=
= SpawnTank
=
==================
*/

void SpawnTank(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = tankobj;
	newobj->active = allways;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 72);
	newobj->xdir = 1;
	newobj->ydir = 1;
	NewState(newobj, &s_tankstand);
	newobj->hitnorth = 1;
	newobj->ticcount = US_RndT() / 32;
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 100;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 50;
		break;
	default:
		newobj->health = 25;
		break;
	}
#endif
}

/*
==================
=
= TankStandThink
=
==================
*/

void TankStandThink(objtype *ob)
{
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT() / 8;
		ob->shootable = false;
		ChangeState(ob, &s_tankdie);
		return;
	}
	if (US_RndT() < 160)
	{
		ob->state = &s_tankwalk1;
	}
}

/*
==================
=
= TankWalkThink
=
==================
*/

void TankWalkThink(objtype *ob)
{
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT() /8 ;
		ob->shootable = false;
		ob->state = &s_tankdie;
		return;
	}
	if (US_RndT() < 20)
	{
		ob->state = &s_tankstand;
		return;
	}
	if (US_RndT() < 100)
	{
		ob->state = &s_tankattack1;
	}
	if (US_RndT() > 200)
	{
		ob->state = &s_tankattack2;
	}
}

/*
==================
=
= TankReact		(never used!)
=
==================
*/

void TankReact(objtype *ob)
{
	if (ob->xdir == 1 && ob->hitwest)
	{
		ob->x -= ob->xmove;
		ob->xdir = -1;
		ChangeState(ob, ob->state);
	}
	else if (ob->xdir == -1 && ob->hiteast)
	{
		ob->x -= ob->xmove;
		ob->xdir = 1;
		ChangeState(ob, ob->state);
	}
	else if (!ob->hitnorth)
	{
		ob->x -= ob->xmove*2;
		ob->y -= ob->ymove;
		ob->xdir = -ob->xdir;
		ChangeState(ob, ob->state);
	}
	if (ob->health <= 0)
	{
		ob->state = &s_tankdie;
	}
	PLACESPRITE;
}

/*
==================
=
= TankAttack1Think
=
==================
*/

void TankAttack1Think(objtype *ob)
{
	GetNewObj(true);
	newobj->obclass = enemyshotobj;
	newobj->active = allways;
	newobj->x = ob->x + 8*PIXGLOBAL;
	newobj->y = ob->y + 23*PIXGLOBAL;
	newobj->xdir = -1;
	newobj->ydir = 1;
	newobj->xspeed = (newobj->xdir*132) + (US_RndT() >> 4);
	newobj->yspeed = -3;
	NewState(newobj, &s_tankshot);
	SD_PlaySound(RAYHITSND);
}

/*
==================
=
= TankAttack2Think
=
==================
*/

void TankAttack2Think(objtype *ob)
{
	SD_PlaySound(RICOCHET1SND);
	GetNewObj(true);
	newobj->obclass = enemyshotobj;
	newobj->active = allways;
	newobj->x = ob->x + 40*PIXGLOBAL;
	newobj->y = ob->y + 8*PIXGLOBAL;
	newobj->xdir = -1;
	newobj->ydir = -1;
	newobj->xspeed = (newobj->xdir*80) + (US_RndT() >> 4);
	newobj->yspeed = -80;
	newobj->temp1 = US_RndT() / 4;
	NewState(newobj, &s_smallrocketfly);
}

/*
==================
=
= TankshotReact
=
==================
*/

void TankshotReact(objtype *ob)
{
	PLACESPRITE;
	if (ob->hiteast || ob->hitwest || ob->hitsouth)
	{
		ChangeState(ob, &s_tankshotexplode1);
		return;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_tankshotexplode1);
		GetNewObj(true);
		newobj->obclass = enemyshotobj;
		newobj->active = allways;
		newobj->x = ob->x;
		newobj->y = ob->y + 8*PIXGLOBAL;
		newobj->temp7 = 100;
		NewState(newobj, &s_smallfire1);
	}
}

/*
==================
=
= SmallfireThink
=
==================
*/

void SmallfireThink(objtype *ob)
{
	if (ob->temp7 > 0 && ob->hitnorth)
	{
		ob->temp7--;
		return;
	}
	else
	{
		RemoveObj(ob);
	}
}

/*
==================
=
= SmallrocketFlyReact
=
==================
*/

void SmallrocketFlyReact(objtype *ob)
{
#if (EPISODE == 3)
	if (ob->hitnorth || ob->hitsouth || ob->hiteast || ob->hitwest)
	{
		ob->xspeed = 0;
		ob->yspeed = 0;
	}
#endif
	if (ob->temp1 > 0)
	{
		ob->temp1--;
	}
	if (ob->temp1 <= 0 || ob->yspeed > 0)
	{
		ob->temp1 = US_RndT() / 8;
		ChangeState(ob, &s_smallrocketfall);
	}
	PLACESPRITE;
}

/*
==================
=
= SmallrocketFallReact
=
==================
*/

void SmallrocketFallReact(objtype *ob)
{
	if (ob->temp1 > 0 && !ob->hitnorth)
	{
		if (US_RndT() < 100)
		{
			ob->xdir = -ob->xdir;
		}
		ob->temp1--;
	}
	else
	{
		ChangeState(ob, &s_tankshotexplode1);
		FragBloom(ob->x, ob->y, dir_North);
		FragBloom(ob->x, ob->y, dir_South);
		FragBloom(ob->x, ob->y, dir_East);
		FragBloom(ob->x, ob->y, dir_West);
		SD_PlaySound(LASERSND);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// GUARD (MUTANT or ROBOT)
//
// temp1: shot burst counter / number of explosions during death animation
// temp2: delay before next attack
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_guardstand   = {GUARD1LSPR, GUARD1RSPR, step, false, push_down, 6, 0, 0, GuardStandThink, NULL, DrawReact, &s_guardstand};
FARSTATE s_guardattack1 = {GUARD1LSPR, GUARD1RSPR, step, true, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_guardattack2};
FARSTATE s_guardattack2 = {GUARD1LSPR, GUARD1RSPR, step, true, push_down, 4, 0, 0, NULL, NULL, DrawReact, &s_guardattack3};
FARSTATE s_guardattack3 = {GUARD2LSPR, GUARD2RSPR, step, false, push_down, 6, 0, 0, GuardAttackThink, NULL, DrawReact, &s_guardattack2};
FARSTATE s_guarddie     = {GUARD1LSPR, GUARD1RSPR, stepthink, false, push_down, 6, 0, 0, ExplodeThink, NULL, DrawReact, &s_guarddie};

/*
==================
=
= SpawnGuard
=
==================
*/

void SpawnGuard(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = guardobj;
	newobj->active = allways;
	newobj->needtoreact = true;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
#if (EPISODE == 1)
	newobj->y = SPAWN_ADJUST_Y(y, 56);
#else
	newobj->y = SPAWN_ADJUST_Y(y, 48);
#endif
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->hitnorth = 1;
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 20;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 40;
		break;
	default:
		newobj->health = 20;
		break;
	}
#endif
	NewState(newobj, &s_guardstand);
}

/*
==================
=
= GuardStandThink
=
==================
*/

void GuardStandThink(objtype *ob)
{
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		// This code will only get executed in episode 3 and in the beta. Even
		// though the guard is a robot in episodes 2 and 3, the code in episode 2
		// (and episode 1) will remove the guard as soon as its health reaches 0
		// (see HurtObject in BM_SNAK2.C).
		ob->temp1 = US_RndT() / 0x10;
		ob->shootable = false;
		ChangeState(ob, &s_guarddie);
		return;
	}
	if (player->bottom > ob->top && player->top < ob->bottom)
	{
		if (ob->temp2 <= 0)
		{
			if (ob->x > player->x)
			{
				ob->xdir = -1;
			}
			else
			{
				ob->xdir = 1;
			}
#ifdef BETA
			ob->temp1 = 7;	//number of shots to fire
#else
			ob->temp1 = 2;	//number of shots to fire
#endif
#if (EPISODE == 2)
			ob->temp2 = US_RndT() >> 4;	//delay for next attack
#else
			ob->temp2 = 30;	//delay for next attack
#endif
			ob->state = &s_guardattack1;
		}
		else
		{
			ob->temp2--;
		}
	}
}

/*
==================
=
= GuardAttackThink
=
==================
*/

void GuardAttackThink(objtype *ob)
{
	Uint16 shotx;

	ob->needtoreact = true;
	if (--ob->temp1 == 0)
	{
		ob->state = &s_guardstand;
	}
	if (ob->xdir == 1)
	{
#if (EPISODE == 1)
		shotx = ob->x + 3*TILEGLOBAL;
#else
		shotx = ob->x + 4*TILEGLOBAL;
#endif
	}
	else
	{
		shotx = ob->x;
	}
#if (EPISODE == 1)
	if (SpawnEnemyShot(shotx, ob->y + 26*PIXGLOBAL, &s_enemyshot) == -1)
#else
	if (SpawnEnemyShot(shotx, ob->y + 12*PIXGLOBAL, &s_enemyshot) == -1)
#endif
	{
		return;
	}
	newobj->xspeed = ob->xdir * 60;
	if (US_RndT() < 70)
	{
		newobj->yspeed = 0;
	}
	else if (ob->temp1 & 1)
	{
		newobj->yspeed = 4;
	}
	else
	{
		newobj->yspeed = -4;
	}
	SD_PlaySound(LASERSND);
}

//////////////////////////////////////////////////////////////////////////////
//
// ENEMY PROJECTILES
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_enemyshot    = {ENEMYSHOTSPR, ENEMYSHOTSPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_enemyshot};
FARSTATE s_explosion1   = {MEDIUMEXPLOSION1SPR, MEDIUMEXPLOSION1SPR, step, true, push_none, 10, 0, 0, NULL, NULL, DrawReact, &s_explosion2};
FARSTATE s_explosion2   = {MEDIUMEXPLOSION2SPR, MEDIUMEXPLOSION2SPR, step, true, push_none, 10, 0, 0, NULL, NULL, DrawReact, NULL};
#if (EPISODE == 1)
FARSTATE s_bossshot1    = {MEDIUMEXPLOSION1SPR, MEDIUMEXPLOSION1SPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_bossshot2};
FARSTATE s_bossshot2    = {MEDIUMEXPLOSION2SPR, MEDIUMEXPLOSION2SPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_bossshot1};
#elif (EPISODE == 3)
FARSTATE s_cyborgshot   = {CYBORGSHOTSPR, CYBORGSHOTSPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_cyborgshot};
FARSTATE s_goliathshot1 = {GOLIATHSHOT1SPR, GOLIATHSHOT1SPR, stepthink, true, push_none, 15, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_goliathshot2};
FARSTATE s_goliathshot2 = {GOLIATHSHOT2SPR, GOLIATHSHOT2SPR, stepthink, true, push_none, 15, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_goliathshot1};
FARSTATE s_cainshot1    = {CAINSHOT1SPR, CAINSHOT1SPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_cainshot2};
FARSTATE s_cainshot2    = {CAINSHOT2SPR, CAINSHOT3SPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_cainshot3};
FARSTATE s_cainshot3    = {CAINSHOT2SPR, CAINSHOT3SPR, stepthink, true, push_none, 8, 0, 0, VelocityThink, EnemyshotContact, EnemyshotReact, &s_cainshot1};
#endif

/*
==================
=
= EnemyshotContact
=
==================
*/

void EnemyshotContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj || hit->obclass == pushableobj)	//BUG: should also handle blockedobj here!
	{
		ob->y -= 8*PIXGLOBAL;
		ChangeState(ob, &s_explosion1);
		// BUG? This doesn't change the shot's obclass value into a harmless type,
		// which means the exploded shot can still hurt the player. That's usually
		// not a huge problem, because the game makes the player invincible for a
		// short while after taking damage. But it can be a bit unfair if a shot
		// hits the player right before the invincibility expires.
		return;
	}
}

/*
==================
=
= EnemyshotReact
=
==================
*/

void EnemyshotReact(objtype *ob)
{
	if (ob->hitnorth || ob->hiteast || ob->hitsouth || ob->hitwest)
	{
		SD_PlaySound(STUNRAYSND);
		ob->y -= 8*PIXGLOBAL;
		ChangeState(ob, &s_explosion1);
		ob->obclass = decoobj;
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// PUSHABLE BLOCK
//
// temp7: countdown for slide sound
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_block     = {BLOCKSPR, BLOCKSPR, stepthink, false, push_down, 6, 0, 1, NULL, BlockContact, BlockReact, &s_block};
FARSTATE s_blockfall = {BLOCKSPR, BLOCKSPR, stepthink, false, push_none, 0, 0, 0, ProjectileThink, BlockContact, BlockFallingReact, &s_block};

/*
==================
=
= SpawnPushableBlock
=
==================
*/

void SpawnPushableBlock(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = pushableobj;
	newobj->active = allways;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 48);
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->hitnorth = 1;
	newobj->needtoclip = cl_midclip;	// redundant (already set by GetNewObj)
	newobj->shootable = true;
	newobj->temp7 = 15;
	NewState(newobj, &s_block);
}

/*
==================
=
= BlockReact
=
==================
*/

void BlockReact(objtype *ob)
{
	if (!ob->hitnorth)
	{
		ChangeState(ob, &s_blockfall);
	}
	PLACESPRITE;
}

/*
==================
=
= BlockFallingReact
=
==================
*/

void BlockFallingReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_block);
	}
	PLACESPRITE;
}

/*
==================
=
= BlockContact
=
==================
*/

void BlockContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj && player->bottom >ob->top)
	{
		if (--ob->temp7 <= 0)
		{
			ob->temp7 = 15;
			SD_PlaySound(ROCKSLIDESND);
		}
	}
	if (hit->obclass == pushableobj)
	{
		ClipToSprite(ob, hit, false);
		ob->obclass = blockedobj;
		hit->obclass = blockedobj;
		/*
		Note: This code will never be executed in the original games. None of the
		levels have more than one instance of the pushable block. That's a good
		thing, since most of the other code only handles pushableobj, but not the
		blockedobj setting. For example, blocked pushable blocks will explode into
		a pile of gibs when hit by the player's shots or an explosion.
		*/
		return;
	}
	if (hit->obclass == explosionobj)
	{
		ClipToSprite(hit, ob, false);
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// FALLING BRICK
//
// temp7: fall countdown
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_brick     = {BRICKSPR, BRICKSPR, stepthink, false, push_none, 1, 0, 0, BrickThink, BrickContact, DrawReact, &s_brick};
FARSTATE s_brickfall = {BRICKSPR, BRICKSPR, stepthink, false, push_none, 0, 0, 0, ProjectileThink, NULL, BrickFallingReact, &s_brickfall};

/*
==================
=
= SpawnFallingBrick
=
==================
*/

void SpawnFallingBrick(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = fallingbrickobj;
	newobj->active = removable;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	newobj->xdir = 0;
	newobj->ydir = -1;
	newobj->shootable = true;
	newobj->temp7 = 35;
	NewState(newobj, &s_brick);
}

/*
==================
=
= BrickThink
=
==================
*/

void BrickThink(objtype *ob)
{
	if (ob->temp7 != 35)
	{
		if (--ob->temp7 <= 0)
		{
			ChangeState(ob, &s_brickfall);
			SD_PlaySound(FALLBLOCKSND);
			ob->needtoclip = cl_noclip;
		}
	}
}

/*
==================
=
= BrickFallingReact
=
==================
*/

void BrickFallingReact(objtype *ob)
{
	PLACESPRITE;
}

/*
==================
=
= BrickContact
=
==================
*/

void BrickContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj && ob->temp7 == 35)
	{
		ob->temp7--;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// CRUSHER (spiked brick)
//
// temp7: sound countdown
//
//////////////////////////////////////////////////////////////////////////////

Sint16 crusherstate = 0;

FARSTATE s_crusher     = {CRUSHERSPR, CRUSHERSPR, stepthink, false, push_none, 6, 0, 0, CrusherThink, NULL, DrawReact, &s_crusher};
#ifdef BETA
FARSTATE s_crushersink = {CRUSHERSPR, CRUSHERSPR, stepthink, false, push_none, 5, 0, 8, CrusherSinkThink, NULL, CrusherSinkReact, &s_crushersink};
#else
FARSTATE s_crushersink = {CRUSHERSPR, CRUSHERSPR, stepthink, false, push_none, 5, 0, 8, CrusherSinkThink, CrusherSinkContact, CrusherSinkReact, &s_crushersink};
#endif

/*
==================
=
= SpawnCrusher
=
==================
*/

void SpawnCrusher(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = fallingbrickobj;
	newobj->priority = 2;
	newobj->active = allways;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->shootable = true;
	newobj->needtoclip = cl_fullclip;
	newobj->temp7 = 15;
	NewState(newobj, &s_crusher);
}

/*
==================
=
= CrusherThink
=
==================
*/

void CrusherThink(objtype *ob)
{
	if (crusherstate == 1)
	{
		ChangeState(ob, &s_crushersink);
	}
}

/*
==================
=
= CrusherSinkThink
=
==================
*/

void CrusherSinkThink(objtype *ob)
{
	if (--ob->temp7 <= 0)
	{
		ob->temp7 = 15;
		SD_PlaySound(CRUSHERSND);
	}
}

/*
==================
=
= CrusherSinkReact
=
==================
*/

void CrusherSinkReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		crusherstate = -1;
		ChangeState(ob, &s_crusher);
		SD_PlaySound(CHUNKSPLATSND);
	}
	PLACESPRITE;
}

#ifndef BETA
/*
==================
=
= CrusherSinkContact
=
==================
*/

void CrusherSinkContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj && ob->bottom > hit->top)
	{
		jumptime = 0;
		if (!snakedied)
		{
			KillPlayer();
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// BOUNCER
//
// temp1: jumptime (time without gravity)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_bouncer1 = {BOUNCER1SPR, BOUNCER1SPR, stepthink, false, push_none, 10, 0, 0, BouncerThink, NULL, BouncerReact, &s_bouncer2};
FARSTATE s_bouncer2 = {BOUNCER3SPR, BOUNCER3SPR, stepthink, false, push_none, 10, 0, 0, BouncerThink, NULL, BouncerReact, &s_bouncer3};
FARSTATE s_bouncer3 = {BOUNCER4SPR, BOUNCER4SPR, stepthink, false, push_none, 10, 0, 0, BouncerThink, NULL, BouncerReact, &s_bouncer4};
FARSTATE s_bouncer4 = {BOUNCER5SPR, BOUNCER5SPR, stepthink, false, push_none, 10, 0, 0, BouncerThink, NULL, BouncerReact, &s_bouncer1};
FARSTATE s_bouncer_ground = {BOUNCER2SPR, BOUNCER2SPR, stepthink, false, push_none, 15, 0, 0, BouncerThink, NULL, BouncerReact, &s_bouncer1};

/*
==================
=
= SpawnBouncer
=
==================
*/

void SpawnBouncer(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = bouncerobj;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 40);
	newobj->xdir = 1;
	newobj->xspeed = 8;
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 5;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 10;
		break;
	default:
		newobj->health = 5;
		break;
	}
#endif
	NewState(newobj, &s_bouncer1);
}

/*
==================
=
= BouncerThink
=
==================
*/

void BouncerThink(objtype *ob)
{
	Sint16 dist;

	AccelerateX(ob, ob->xdir, 8);
	// Note: The following gravity and jumptime handling code appears to have
	// come directly from TomatBounceThink in KEEN DREAMS.
	if (ob->temp1)
	{
		// BUG: BioMenace is not Keen Dreams. The amount the object should move
		// must be stored in the global variable ytry, not in the object's ymove
		// field. This basically makes the Bouncer not move up/down at all when
		// temp1 is not 0.
		if (ob->temp1 < tics)
		{
			ob->ymove = ob->yspeed * ob->temp1;
			ob->temp1 = 0;
		}
		else
		{
			ob->ymove = ob->yspeed * tics;
			ob->temp1 = ob->temp1 - tics;
		}
	}
	else
	{
		DoGravity(ob);
	}
	
	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	
	if (ob->xdir == -1)
	{
		dist = ob->left - player->right;
		if (dist > 4*TILEGLOBAL)
		{
			return;
		}
		ob->xdir = 1;
		return;
	}
	else
	{
		dist = player->left - ob->right;
		if (dist > 4*TILEGLOBAL)
		{
			return;
		}
		ob->xdir = -1;
		return;
	}
}

/*
==================
=
= BouncerReact		(based on TomatReact from KEEN DREAMS)
=
==================
*/

void BouncerReact(objtype *ob)
{
	if (ob->hiteast || ob->hitwest)
	{
		ob->xdir = -ob->xdir;
		ob->xspeed = -ob->xspeed;
	}
	if (ob->hitsouth)
	{
		if (ob->tileright >= originxtile && ob->tileleft <= originxtilemax
			&& ob->tiletop >= originytile && ob->tilebottom <= originytilemax)
		{
			SD_PlaySound(JUMPBUGSND);
		}
		ob->yspeed = -ob->yspeed;
	}
	if (ob->hitnorth)
	{
		if (ob->tileright >= originxtile && ob->tileleft <= originxtilemax
			&& ob->tiletop >= originytile && ob->tilebottom <= originytilemax)
		{
			SD_PlaySound(JUMPBUGSND);
		}
		ob->yspeed = -64 - (US_RndT() >> 4);
		ob->temp1 = 2;
		ChangeState(ob, &s_bouncer_ground);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// ACID MONSTER
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_acidmonster1 = {ACIDMONSTER1SPR, ACIDMONSTER1SPR, step, false, push_none, 50, 0, 0, NULL, NULL, DrawReact, &s_acidmonster2};
FARSTATE s_acidmonster2 = {ACIDMONSTER2SPR, ACIDMONSTER2SPR, step, false, push_none, 50, 0, 0, NULL, NULL, DrawReact, &s_acidmonster3};
FARSTATE s_acidmonster3 = {ACIDMONSTER3SPR, ACIDMONSTER3SPR, step, false, push_none, 1, 0, 0, AcidmonsterThink, NULL, DrawReact, &s_acidmonster1};
FARSTATE s_acid1 = {ACIDDROP1SPR, ACIDDROP1SPR, step, false, push_none, 5, 0, 0, NULL, NULL, DrawReact, &s_acid2};
FARSTATE s_acid2 = {ACIDDROP2SPR, ACIDDROP2SPR, step, false, push_none, 5, 0, 0, AcidThink, NULL, DrawReact, &s_acid1};
FARSTATE s_acidfall = {ACIDDROP3SPR, ACIDDROP3SPR, stepthink, false, push_none, 0, 0, 0, ProjectileThink, NULL, AcidFallReact, &s_acidfall};
FARSTATE s_acidland1 = {ACIDLAND1SPR, ACIDLAND1SPR, step, false, push_down, 15, 0, 0, NULL, NULL, DrawReact, &s_acidland2};
FARSTATE s_acidland2 = {ACIDLAND2SPR, ACIDLAND2SPR, step, false, push_down, 15, 0, 0, NULL, NULL, DrawReact, &s_acidland3};
FARSTATE s_acidland3 = {ACIDLAND3SPR, ACIDLAND3SPR, step, false, push_down, 15, 0, 0, NULL, NULL, DrawReact, &s_acidland4};
FARSTATE s_acidland4 = {ACIDLAND4SPR, ACIDLAND4SPR, step, false, push_down, 15, 0, 0, NULL, NULL, DrawReact, NULL};

/*
==================
=
= SpawnDrool
=
==================
*/

void SpawnDrool(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = acidmonsterobj;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	newobj->xdir = 1;
	newobj->ydir = -1;
	NewState(newobj, &s_acidmonster1);
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 50;
#else
	newobj->health = 10;
#endif
}

/*
==================
=
= AcidmonsterThink
=
==================
*/

void AcidmonsterThink(objtype *ob)
{
	SpawnAcid(ob);
}

/*
==================
=
= SpawnAcid
=
==================
*/

void SpawnAcid(objtype *ob)
{
	GetNewObj(true);
	newobj->obclass = acidmonsterobj;
	newobj->x = ob->x + 8*PIXGLOBAL;
	newobj->y = ob->bottom;
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->active = removable;
	newobj->priority = 3;
	NewState(newobj, &s_acid1);
#ifndef BETA
	SD_PlaySound(ACIDSND);
#endif
}

/*
==================
=
= AcidThink
=
==================
*/

void AcidThink(objtype *ob)
{
	if (US_RndT() < 100)
	{
		ob->state = &s_acidfall;
	}
}

/*
==================
=
= AcidFallReact
=
==================
*/

void AcidFallReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->state = &s_acidland1;
		ob->priority = 0;
		ob->y -= 8*PIXGLOBAL;
		return;
	}
	PLACESPRITE;
}

#if (EPISODE != 1)
//////////////////////////////////////////////////////////////////////////////
//
// SPIKEY
//
// temp4: set to 50 at spawn, but never used
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_spikey1 = {SPIKEY1SPR, SPIKEY1SPR, stepthink, false, push_none, 2, 0, 24, SpikeyThink, NULL, DrawReact, &s_spikey2};
FARSTATE s_spikey2 = {SPIKEY2SPR, SPIKEY2SPR, stepthink, false, push_none, 2, 0, 24, SpikeyThink, NULL, DrawReact, &s_spikey1};

/*
==================
=
= SpawnSpikey
=
==================
*/

void SpawnSpikey(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = spikeyobj;
	newobj->active = yes;
	newobj->priority = 2;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	newobj->temp4 = 50;	// maybe a leftover from Keen 5's SpawnSlicestarBounce
	newobj->xdir = 0;
	newobj->ydir = 1;
	newobj->yspeed = 24;
	NewState(newobj, &s_spikey1);
}

/*
==================
=
= SpikeyThink
=
==================
*/

void SpikeyThink(objtype *ob)
{
	// Note: hitnorth and hitsouth should generally be checked in the react
	// function, not in the think function.
	if (ob->hitnorth)
	{
		ob->ydir = -1;
	}
	else if (ob->hitsouth)
	{
		ob->ydir = 1;
	}
	
	ob->ymove = ob->yspeed * tics;
	// BUG: yspeed is ALWAYS 24, so this always sets ymove to a positive value
	// (which means: moving down). But this is not Keen Dreams. The amount the
	// object is supposed to move must be stored in the global variable ytry,
	// not in the object's ymove field. That makes this code rather pointless.
}

//////////////////////////////////////////////////////////////////////////////
//
// TRASH TRUCK
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_trashtruck1 = {TRASHTRUCK1LSPR, TRASHTRUCK1RSPR, step, false, push_down, 5, 128, 0, NULL, NULL, WalkReact, &s_trashtruck2};
FARSTATE s_trashtruck2 = {TRASHTRUCK2LSPR, TRASHTRUCK2RSPR, step, false, push_down, 5, 128, 0, NULL, NULL, WalkReact, &s_trashtruck1};

/*
==================
=
= SpawnTrashTruck
=
==================
*/

void SpawnTrashTruck(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = trashtruckobj;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 24);
	newobj->xdir = 1;
	newobj->ydir = 1;
	NewState(newobj, &s_trashtruck1);
	newobj->hitnorth = 1;
	newobj->ticcount = US_RndT()/32;
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 5;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 10;
		break;
	default:
		newobj->health = 5;
		break;
	}
#endif
}
#endif	// (EPISODE != 1)

//////////////////////////////////////////////////////////////////////////////
//
// TURRET
//
// temp1: shot burst counter
// temp2: delay before next attack
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_turret1       = {TURRET1LSPR, TURRET1RSPR, step, false, push_down, 6, 0, 0, TurretThink, NULL, DrawReact, &s_turret1};
FARSTATE s_turretattack1 = {TURRET2LSPR, TURRET2RSPR, step, true, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_turretattack2};
FARSTATE s_turretattack2 = {TURRET2LSPR, TURRET2RSPR, step, true, push_down, 4, 0, 0, NULL, NULL, DrawReact, &s_turretattack3};
FARSTATE s_turretattack3 = {TURRET3LSPR, TURRET3RSPR, step, false, push_down, 6, 0, 0, TurretAttackThink, NULL, DrawReact, &s_turretattack2};

/*
==================
=
= SpawnTurret
=
==================
*/

void SpawnTurret(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = turretobj;
#if (EPISODE == 1)
	newobj->priority = 2;
#endif
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 32);
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->hitnorth = 1;
	newobj->shootable = true;
#ifdef BETA
	newobj->health = 10;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 7;
		break;
	default:
		newobj->health = 4;
		break;
	}
#endif
	newobj->needtoreact = true;
	NewState(newobj, &s_turret1);
}

/*
==================
=
= TurretThink
=
==================
*/

void TurretThink(objtype *ob)
{
	ob->needtoreact = true;
	if (player->bottom > ob->top && player->top <ob->bottom)
	{
		if (ob->temp2 <= 0)
		{
			if (ob->x > player->x)
			{
				ob->xdir = -1;
			}
			else
			{
				ob->xdir = 1;
			}
#ifdef BETA
			ob->temp1 = 3;	// number of shots to fire
#else
			ob->temp1 = 1;	// number of shots to fire
#endif
			ob->temp2 = US_RndT() >> 4;	// delay before next attack
			ob->state = &s_turretattack1;
		}
		else
		{
			ob->temp2--;
		}
	}
}

/*
==================
=
= TurretAttackThink
=
==================
*/

void TurretAttackThink(objtype *ob)
{
	Uint16 shotx;

	if (--ob->temp1 == 0)
	{
		ob->state = &s_turret1;
	}
	if (ob->xdir == 1)
	{
		shotx = ob->x+2*TILEGLOBAL;
	}
	else
	{
		shotx = ob->x;
	}
	if (SpawnEnemyShot(shotx, ob->y+8*PIXGLOBAL, &s_enemyshot) == -1)
	{
		return;
	}
	newobj->xspeed = ob->xdir * 60;
	newobj->yspeed = 0;
	SD_PlaySound(SNAKESHOOTSND);
}

#ifndef BETA
//////////////////////////////////////////////////////////////////////////////
//
// ASTEROID
// (a slightly modified version of the bouncing SLICESTAR from KEEN 5)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_asteroid1 = {ASTEROID1SPR, ASTEROID1SPR, slide, false, push_none, 10, 24, 24, NULL, NULL, LandmineReact, &s_asteroid2};
FARSTATE s_asteroid2 = {ASTEROID2SPR, ASTEROID2SPR, slide, false, push_none, 10, 24, 24, NULL, NULL, LandmineReact, &s_asteroid3};
FARSTATE s_asteroid3 = {ASTEROID3SPR, ASTEROID3SPR, slide, false, push_none, 10, 24, 24, NULL, NULL, LandmineReact, &s_asteroid4};
FARSTATE s_asteroid4 = {ASTEROID4SPR, ASTEROID4SPR, slide, false, push_none, 10, 24, 24, NULL, NULL, LandmineReact, &s_asteroid1};

/*
==================
=
= SpawnAsteroid
=
==================
*/

void SpawnAsteroid(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = decoobj;
	newobj->active = allways;
	newobj->priority = 2;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	switch (US_RndT() / 0x40)
	{
	case 0:
		newobj->xdir = -1;
		newobj->ydir = -1;
		break;
	case 1:
		newobj->xdir = 1;
		newobj->ydir = 1;
		break;
	case 2:
		newobj->xdir = -1;
		newobj->ydir = 1;
		break;
	case 3:
		newobj->xdir = 1;
		newobj->ydir = -1;
		break;
	}
	NewState(newobj, &s_asteroid1);
}

//////////////////////////////////////////////////////////////////////////////
//
// LANDMINE
//
// As you can see here, the landmine was supposed to bounce around like a
// Slicestar at some point during development, similar to the Asteroid.
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_landmine1 = {LANDMINE1SPR, LANDMINE1SPR, slide, false, push_none, 10, 0, 0, NULL, LandmineContact, LandmineReact, &s_landmine2};
FARSTATE s_landmine2 = {LANDMINE2SPR, LANDMINE2SPR, slide, false, push_none, 10, 0, 0, NULL, LandmineContact, LandmineReact, &s_landmine1};

/*
==================
=
= SpawnLandmine
=
==================
*/

//#pragma argsused	//both parameters are ignored!
//void SpawnLandmine(Uint16 x, Uint16 y)
void SpawnLandmine(void)
{
	GetNewObj(true);
	newobj->obclass = grenadeobj;
	newobj->active = allways;
	newobj->needtoclip = cl_fullclip;
	if (player->xdir == 1)
	{
		newobj->x = player->midx-8*PIXGLOBAL;
	}
	else
	{
		newobj->x = player->midx;
	}
	newobj->y = player->bottom-TILEGLOBAL;
	//Note: the switch is useless since the landmine states' xmove and ymove values are 0!
	switch (US_RndT() / 0x40)
	{
	case 0:
		newobj->xdir = player->xdir;
		newobj->ydir = -1;
		break;
	case 1:
		newobj->xdir = player->xdir;
		newobj->ydir = 1;
		break;
	case 2:
		newobj->xdir = player->xdir;
		newobj->ydir = 1;
		break;
	case 3:
		newobj->xdir = player->xdir;
		newobj->ydir = -1;
		break;
	}
	NewState(newobj, &s_landmine1);
}

/*
==================
=
= LandmineContact
=
==================
*/

void LandmineContact(objtype *ob, objtype *hit)
{
	if (hit->shootable && hit->obclass != pushableobj)	//BUG: should also handle blockedobj here!
	{
		FragBloom(ob->x+2*PIXGLOBAL, ob->y-8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_East);
		FragBloom(ob->x-2*PIXGLOBAL, ob->y-8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_West);
		ob->obclass = explosionobj;
		ob->temp6 = 1;	// ??? doesn't appear to be used for anything
		// BUG? this doesn't set ob->spareplayer = true, which means
		// the mine's explosion can hurt the player!
		SD_PlaySound(GRENADEXPLODESND);
		ChangeState(ob, &s_grenadeexplosion1);
		ob->needtoclip = cl_midclip;
		HurtObject(hit, 5);
	}
}

/*
==================
=
= LandmineReact
=
==================
*/

void LandmineReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->ydir = -1;
	}
	else if (ob->hitsouth)
	{
		ob->ydir = 1;
	}
	if (ob->hitwest)
	{
		ob->xdir = -1;
	}
	else if (ob->hiteast)
	{
		ob->xdir = 1;
	}
	PLACESPRITE;
}
#endif	// ifndef BETA

#if (EPISODE == 1)
//////////////////////////////////////////////////////////////////////////////
//
// SKULL MAN
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_skullman1        = {SKULLMAN1SPR, SKULLMAN1SPR, stepthink, false, push_none, 35, 0, 0, SkullmanThink, NULL, DrawReact, &s_skullman1};
FARSTATE s_skullman2        = {SKULLMAN2SPR, SKULLMAN2SPR, stepthink, false, push_none, 35, 0, 0, SkullmanThink, NULL, DrawReact, &s_skullman2};
FARSTATE s_lefthand         = {SKULLMANHANDLSPR, SKULLMANHANDLSPR, stepthink, false, push_none, 10, 0, 0, LeftHandThink, NULL, DrawReact, &s_lefthand};
FARSTATE s_righthand        = {SKULLMANHANDRSPR, SKULLMANHANDRSPR, stepthink, false, push_none, 10, 0, 0, RightHandThink, NULL, DrawReact, &s_righthand};
FARSTATE s_lefthandfly      = {SKULLMANHANDLSPR, SKULLMANHANDLSPR, slide, false, push_none, 10, 24, 24, LeftHandFlyThink, NULL, HandFlyReact, &s_lefthandfly};
FARSTATE s_righthandfly     = {SKULLMANHANDRSPR, SKULLMANHANDRSPR, slide, false, push_none, 10, 24, 24, RightHandFlyThink, NULL, HandFlyReact, &s_righthandfly};
FARSTATE s_lefthandattack1  = {SKULLMANSWIPE1SPR, SKULLMANSWIPE1SPR, stepthink, false, push_none, 20, 0, 0, ProjectileThink, NULL, LeftHandAttackReact, &s_lefthandattack2};
FARSTATE s_lefthandattack2  = {SKULLMANSWIPE2SPR, SKULLMANSWIPE2SPR, stepthink, false, push_none, 20, 0, 0, ProjectileThink, NULL, LeftHandAttackReact, &s_lefthandattack1};
FARSTATE s_righthandattack1 = {SKULLMANSWIPE1SPR, SKULLMANSWIPE1SPR, stepthink, false, push_none, 20, 0, 0, ProjectileThink, NULL, RightHandAttackReact, &s_righthandattack2};
FARSTATE s_righthandattack2 = {SKULLMANSWIPE2SPR, SKULLMANSWIPE2SPR, stepthink, false, push_none, 20, 0, 0, ProjectileThink, NULL, RightHandAttackReact, &s_righthandattack1};

/*
==================
=
= SpawnSkullHand
=
==================
*/

void SpawnSkullHand(Uint16 x, Uint16 y, Uint16 type)
{
	skullmanstate = -2;
	GetNewObj(false);
	newobj->obclass = skullhandobj;
	newobj->needtoreact = true;
	newobj->active = allways;
	newobj->priority = 2;
	newobj->needtoclip = cl_fullclip;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	newobj->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 85;
		break;
	default:
		newobj->health = 65;
		break;
	}
	switch (US_RndT() / 0x40)
	{
	case 0:
		newobj->xdir = -1;
		newobj->ydir = -1;
		break;
	case 1:
		newobj->xdir = 1;
		newobj->ydir = 1;
		break;
	case 2:
		newobj->xdir = -1;
		newobj->ydir = 1;
		break;
	case 3:
		newobj->xdir = 1;
		newobj->ydir = -1;
		break;
	}
	switch (type)
	{
	case 0:
		NewState(newobj, &s_lefthand);
		break;
	case 1:
		NewState(newobj, &s_righthand);
		break;
	}
}

/*
==================
=
= HandFlyReact
=
==================
*/

void HandFlyReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->ydir = -1;
	}
	else if (ob->hitsouth)
	{
		ob->ydir = 1;
	}
	if (ob->hitwest)
	{
		ob->xdir = -1;
	}
	else if (ob->hiteast)
	{
		ob->xdir = 1;
	}
	PLACESPRITE;
}

/*
==================
=
= LeftHandAttackReact
=
==================
*/

void LeftHandAttackReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->state = &s_lefthandfly;
		ob->ydir = -1;
	}
	PLACESPRITE;
}

/*
==================
=
= RightHandAttackReact
=
==================
*/

void RightHandAttackReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->state = &s_righthandfly;
		ob->ydir = -1;
	}
	PLACESPRITE;
}

/*
==================
=
= LeftHandThink
=
==================
*/

void LeftHandThink(objtype *ob)
{
	// left hand wakes up as soon as Skull Man (or a hand) gets shot the first time
	if (skullmanstate == -2)
	{
		return;
	}
	else
	{
		ob->state = &s_lefthandattack1;
	}
}

/*
==================
=
= RightHandThink
=
==================
*/

void RightHandThink(objtype *ob)
{
	// right hand wakes up when Skull Man (or a hand) gets shot a second time
	if (skullmanstate < 0)
	{
		return;
	}
	else
	{
		ob->state = &s_lefthandattack1;	//BUG: I think this was supposed to use &s_righthandattack1
	}
}

/*
==================
=
= LeftHandFlyThink
=
==================
*/

void LeftHandFlyThink(objtype *ob)
{
	if (skullmanstate == 1)	// if Skullman got killed
	{
		RemoveObj(ob);
	}
	if (player->right < ob->left || player->left > ob->right || player->top < ob->bottom)
	{
		return;
	}
	// hand is directly above player, so start the attack
	ob->state = &s_lefthandattack1;
	ob->yspeed = 0;
}

/*
==================
=
= RightHandFlyThink
=
==================
*/

void RightHandFlyThink(objtype *ob)
{
	if (skullmanstate == 1)	// if Skullman got killed
	{
		RemoveObj(ob);
	}
	if (player->right < ob->left || player->left > ob->right || player->top < ob->bottom)
	{
		return;
	}
	// hand is directly above player, so start the attack
	ob->state = &s_righthandattack1;
	ob->yspeed = 0;
}

/*
==================
=
= SpawnSkullman
=
==================
*/

void SpawnSkullman(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = skullheadobj;
	newobj->active = allways;
	newobj->priority = 2;
	newobj->needtoreact = true;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = CONVERT_TILE_TO_GLOBAL(y);
	newobj->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 200;
		break;
	default:
		newobj->health = 125;
		break;
	}
	bosshealth = newobj->health;
	oldhealthbarlength = -1;
	bossdiv = bosshealth/20 - 1;	//BUG? see comments in UpdateScorebox()
	NewState(newobj, &s_skullman2);
}

/*
==================
=
= SkullmanThink
=
==================
*/

void SkullmanThink(objtype *ob)
{
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
	ob->needtoreact = true;
	if (US_RndT() > 200)
	{
		if (ob->state == &s_skullman1)
		{
			ob->state = &s_skullman2;
		}
		else
		{
			ob->state = &s_skullman1;
		}
	}
}
#endif	// (EPISODE == 1)

#if (EPISODE != 2)
//////////////////////////////////////////////////////////////////////////////
//
// PLASMA SHOOTER
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_plasmashooter1 = {0, 0, step, false, push_none, 50, 0, 0, NULL, NULL, DrawReact, &s_plasmashooter2};
FARSTATE s_plasmashooter2 = {0, 0, step, false, push_none, 1, 0, 0, PlasmashooterThink, NULL, DrawReact, &s_plasmashooter1};
FARSTATE s_plasma1        = {PLASMA1SPR, PLASMA1SPR, stepthink, false, push_none, 8, 0, 0, VelocityThink, NULL, PlasmaReact, &s_plasma2};
FARSTATE s_plasma2        = {PLASMA2SPR, PLASMA2SPR, stepthink, false, push_none, 8, 0, 0, VelocityThink, NULL, PlasmaReact, &s_plasma1};
FARSTATE s_plasmaexplode1 = {MEDIUMEXPLOSION1SPR, MEDIUMEXPLOSION1SPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, &s_plasmaexplode2};
FARSTATE s_plasmaexplode2 = {MEDIUMEXPLOSION2SPR, MEDIUMEXPLOSION2SPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, NULL};

/*
==================
=
= SpawnPlasmashooter
=
==================
*/

void SpawnPlasmashooter(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = decoobj;
	newobj->active = yes;
	newobj->tileright = newobj->tileleft = x;
	newobj->tiletop = newobj->tilebottom = y;
	newobj->x = newobj->left = newobj->right = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = newobj->top = newobj->bottom = CONVERT_TILE_TO_GLOBAL(y);
	NewState(newobj, &s_plasmashooter1);
}

/*
==================
=
= PlasmashooterThink
=
==================
*/

void PlasmashooterThink(objtype *ob)
{
	GetNewObj(true);
	newobj->obclass = enemyshotobj;
	newobj->priority = 2;
	newobj->active = removable;
	newobj->x = ob->x;
	newobj->y = ob->y;
	switch (US_RndT() / 64)
	{
	case 0:
		// shoot straight up:
		newobj->yspeed = -64;
		break;
	case 1:
		// shoot down and right:
		newobj->xspeed = 64;
		newobj->yspeed = 64;
		break;
	case 2:
		// shoot straight down:
		newobj->yspeed = 64;
		break;
	case 3:
		// shoot down and left:
		newobj->xspeed = -64;
		newobj->yspeed = 64;
		break;
	}
	NewState(newobj, &s_plasma1);
	SD_PlaySound(LASERSND);
}

/*
==================
=
= PlasmaReact
=
==================
*/

void PlasmaReact(objtype *ob)
{
	if (ob->hitnorth || ob->hiteast || ob->hitsouth || ob->hitwest)
	{
		SD_PlaySound(STUNRAYSND);
		ChangeState(ob, &s_plasmaexplode1);
	}
	PLACESPRITE;
}
#endif	// (EPISODE != 2)

#if (EPISODE == 3)
//////////////////////////////////////////////////////////////////////////////
//
// GOLIATH
//
// temp1: shot burst counter / explosions to spawn during death animation
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_goliathdie     = {GOLIATH1SPR, GOLIATH1SPR, stepthink, false, push_down, 6, 0, 0, ExplodeThink, NULL, DrawReact, &s_goliathdie};
FARSTATE s_goliathstand   = {GOLIATH1SPR, GOLIATH1SPR, step, false, push_down, 10, 0, 0, GoliathStandThink, NULL, DrawReact, &s_goliathstand};
FARSTATE s_goliathwalk1   = {GOLIATH1SPR, GOLIATH1SPR, slide, false, push_down, 20, 8, 0, NULL, NULL, WalkReact, &s_goliathwalk2};
FARSTATE s_goliathwalk2   = {GOLIATH2SPR, GOLIATH2SPR, slide, false, push_down, 20, 8, 0, GoliathWalkThink, NULL, WalkReact, &s_goliathwalk1};
//FARSTATE s_goliath_unused = {GOLIATH3SPR, GOLIATH3SPR, step, false, push_down, 7, 0, 0, GoliathUNUSEDThink, NULL, DrawReact, &s_goliathstand}; // REFKEEN: Unused state
FARSTATE s_goliathattack1 = {GOLIATH1SPR, GOLIATH1SPR, step, false, push_down, 3, 0, 0, NULL, NULL, DrawReact, &s_goliathattack2};
FARSTATE s_goliathattack2 = {GOLIATH3SPR, GOLIATH3SPR, step, false, push_down, 4, 0, 0, GoliathAttackThink, NULL, DrawReact, &s_goliathattack1};

/*
==================
=
= SpawnGoliath
=
==================
*/

void SpawnGoliath(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = goliathobj;
	newobj->active = allways;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 104);
	newobj->xdir = 1;
	newobj->ydir = 1;
	NewState(newobj, &s_goliathstand);
	newobj->hitnorth = 1;
	newobj->ticcount = US_RndT()/32;
	newobj->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 150;
		break;
	default:
		newobj->health = 100;
		break;
	}
	bosshealth = newobj->health;
	oldhealthbarlength = -1;
	bossdiv = bosshealth / 20;
}

/*
==================
=
= GoliathStandThink
=
==================
*/

void GoliathStandThink(objtype *ob)
{
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT()/8;
		ob->shootable = false;
		ChangeState(ob, &s_goliathdie);
	}
	else if (US_RndT() < 160)
	{
		ob->state = &s_goliathwalk1;
	}
}

/*
==================
=
= GoliathWalkThink
=
==================
*/

void GoliathWalkThink(objtype *ob)
{
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT()/8;
		ob->shootable = false;
		ob->state = &s_goliathdie;
	}
	else if (US_RndT() < 20)
	{
		ob->state = &s_goliathstand;
	}
	else if (US_RndT() > 150)
	{
		ob->state = &s_goliathattack1;
		ob->temp1 = 8;	// number of shots to spawn
	}
}

/*
==================
=
= GoliathReact		(never used)
=
==================
*/

void GoliathReact(objtype *ob)
{
	if (ob->xdir == 1 && ob->hitwest)
	{
		ob->x -= ob->xmove;
		ob->xdir = -1;
		ChangeState(ob, ob->state);
	}
	else if (ob->xdir == -1 && ob->hiteast)
	{
		ob->x -= ob->xmove;
		ob->xdir = 1;
		ChangeState(ob, ob->state);
	}
	else if (!ob->hitnorth)
	{
		ob->x -= ob->xmove << 1;
		ob->y -= ob->ymove;
		ob->xdir = -ob->xdir;
		ChangeState(ob, ob->state);
	}
	if (ob->health <= 0)
	{
		ob->state = &s_goliathdie;
	}
	PLACESPRITE;
}

#if 0 // REFKEEN: Unused empty stub
/*
==================
=
= GoliathUNUSEDThink
=
==================
*/

#pragma argsused
void GoliathUNUSEDThink(objtype *ob)
{
	//empty!
}
#endif

/*
==================
=
= GoliathAttackThink
=
==================
*/

void GoliathAttackThink(objtype *ob)
{
	Sint16 oldxdir;

	ob->needtoreact = true;
	// always shoot to the left:
	oldxdir = ob->xdir;
	if (ob->xdir == 1)
	{
		ob->xdir = -1;
	}
	if (SpawnEnemyShot(ob->x + 24*PIXGLOBAL, ob->y + 40*PIXGLOBAL, &s_goliathshot1) == -1)
	{
		return;
	}
	newobj->xspeed = ob->xdir * 60;
	newobj->priority = 2;
	if (US_RndT() < 70)
	{
		newobj->yspeed = 0;
	}
	else if (ob->temp1 & 1)
	{
		newobj->yspeed = 4;
	}
	else
	{
		newobj->yspeed = -4;
	}
	ob->xdir = oldxdir;
	SD_PlaySound(LASERSND);
	// change into stand state after enough shots have been fired:
	if (--ob->temp1 <= 0)
	{
		ob->state = &s_goliathstand;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// ENEMY LANDMINE & BARREL
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_enemylandmine1 = {BIGLANDMINE1SPR, BIGLANDMINE1SPR, stepthink, false, push_down, 35, 0, 0, NULL, EnemyLandmineContact, DrawReact, &s_enemylandmine2};
FARSTATE s_enemylandmine2 = {BIGLANDMINE2SPR, BIGLANDMINE2SPR, stepthink, false, push_down, 15, 0, 0, NULL, EnemyLandmineContact, DrawReact, &s_enemylandmine3};
FARSTATE s_enemylandmine3 = {BIGLANDMINE3SPR, BIGLANDMINE3SPR, stepthink, false, push_down, 25, 0, 0, NULL, EnemyLandmineContact, DrawReact, &s_enemylandmine1};
FARSTATE s_barrel         = {BARRELSPR, BARRELSPR, stepthink, false, push_down, 0, 0, 0, BarrelThink, BarrelContact, DrawReact, &s_barrel};
FARSTATE s_barrelfall     = {BARRELSPR, BARRELSPR, stepthink, false, push_none, 0, 0, 0, ProjectileThink, BarrelContact, BarrelFallReact, &s_barrelfall};

/*
==================
=
= SpawnEnemyLandmine
=
==================
*/

void SpawnEnemyLandmine(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = decoobj;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 8);
	NewState(newobj, &s_enemylandmine1);
	newobj->hitnorth = 1;
}

/*
==================
=
= SpawnBarrel
=
==================
*/

void SpawnBarrel(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = barrelobj;
	newobj->active = removable;
	newobj->needtoclip = cl_fullclip;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 32);
	NewState(newobj, &s_barrel);
	newobj->shootable = true;
	newobj->health = 1;
}

/*
==================
=
= BarrelThink
=
==================
*/

void BarrelThink(objtype *ob)
{
	if (ob->health <= 0)
	{
		ob->shootable = false;
		ob->obclass = explosionobj;
		ChangeState(ob, &s_grenadeexplosion1);
	}
	else if (!ob->hitnorth)
	{
		ob->state = &s_barrelfall;
	}
}

/*
==================
=
= EnemyLandmineContact
=
==================
*/

void EnemyLandmineContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj || hit->obclass == explosionobj)
	{
		ob->shootable = false;
		ob->obclass = explosionobj;
		ChangeState(ob, &s_grenadeexplosion1);
	}
}

/*
==================
=
= BarrelContact
=
==================
*/

void BarrelContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == explosionobj)
	{
		ob->shootable = false;
		ob->obclass = explosionobj;
		ChangeState(ob, &s_grenadeexplosion1);
	}
}

/*
==================
=
= BarrelFallReact
=
==================
*/

void BarrelFallReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->state = &s_barrel;
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// GRENADE BOT
//
// temp1: shot burst counter / explosions to spawn during death animation
// temp2: delay between attacks
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_grenadebotstand   = {GRENADEBOT1LSPR, GRENADEBOT1RSPR, step, false, push_down, 6, 0, 0, GrenadebotStandThink, NULL, DrawReact, &s_grenadebotstand};
FARSTATE s_grenadebotattack1 = {GRENADEBOT1LSPR, GRENADEBOT1RSPR, step, true, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_grenadebotattack2};
FARSTATE s_grenadebotattack2 = {GRENADEBOT1LSPR, GRENADEBOT1RSPR, step, true, push_down, 4, 0, 0, NULL, NULL, DrawReact, &s_grenadebotattack3};
FARSTATE s_grenadebotattack3 = {GRENADEBOT2LSPR, GRENADEBOT2RSPR, step, false, push_down, 6, 0, 0, GrenadebotAttackThink, NULL, DrawReact, &s_grenadebotattack2};
FARSTATE s_grenadebotdie     = {GRENADEBOT1LSPR, GRENADEBOT1RSPR, stepthink, false, push_down, 6, 0, 0, ExplodeThink, NULL, DrawReact, &s_grenadebotdie};
FARSTATE s_plasmagrenade     = {PLASMAGRENADESPR, PLASMAGRENADESPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, PlasmagrenadeContact, GrenadeReact, &s_plasmagrenade};

/*
==================
=
= SpawnGrenadeBot
=
==================
*/

void SpawnGrenadeBot(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = grenadebotobj;
	newobj->needtoreact = true;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 48);
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->hitnorth = 1;
	newobj->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 40;
		break;
	default:
		newobj->health = 20;
		break;
	}
	NewState(newobj, &s_grenadebotstand);
}

/*
==================
=
= GrenadebotStandThink
=
==================
*/

void GrenadebotStandThink(objtype *ob)
{
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT() / 16;
		ob->shootable = false;
		ChangeState(ob, &s_grenadebotdie);
	}
	else if (ob->temp2 <= 0)
	{
		if (ob->x > player->x)
		{
			ob->xdir = -1;
		}
		else
		{
			ob->xdir = 1;
		}
		ob->temp1 = 2;	// number of grenades to spawn in a row
		ob->temp2 = US_RndT() >> 4;	// number of "think" calls to wait before attacking again (0..15)
		ob->state = &s_grenadebotattack1;
	}
	else
	{
		ob->temp2--;
	}
}

/*
==================
=
= GrenadebotAttackThink
=
==================
*/

void GrenadebotAttackThink(objtype *ob)
{
	Sint16 shotx;

	ob->needtoreact = true;
	if (US_RndT() < 128)
	{
		ob->xdir = -1;
	}
	else
	{
		ob->xdir = 1;
	}
	if (--ob->temp1 == 0)
	{
		ob->state = &s_grenadebotstand;
	}
	if (ob->xdir == 1)
	{
		shotx = ob->x + 3*TILEGLOBAL;
	}
	else
	{
		shotx = ob->x;
	}
	if (SpawnEnemyShot(shotx, ob->y + 28*PIXGLOBAL, &s_plasmagrenade) == -1)
	{
		return;
	}
	newobj->xspeed = ob->xdir * 35;
	if (US_RndT() < 70)
	{
		newobj->yspeed = 0;
	}
	else if (ob->temp1 & 1)
	{
		newobj->yspeed = US_RndT()/16;	// 0..15
	}
	else
	{
		newobj->yspeed = -(US_RndT()/16);	// -15..0
	}
	SD_PlaySound(LASERSND);
}

/*
==================
=
= PlasmagrenadeContact
=
==================
*/

void PlasmagrenadeContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj || hit->obclass == pushableobj)	//BUG: should also handle blockedobj here!
	{
		FragBloom(ob->x+2*PIXGLOBAL, ob->y-8*PIXGLOBAL, dir_East);
		FragBloom(ob->x-2*PIXGLOBAL, ob->y-8*PIXGLOBAL, dir_West);
		ob->obclass = explosionobj;
		SD_PlaySound(GRENADEXPLODESND);
		ChangeState(ob, &s_grenadeexplosion1);
		ob->needtoclip = cl_midclip;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// MASTER CAIN
//
// temp1: shot burst counter / explosions to spawn during death animation
// temp2: delay between attacks
//
// temp4: delay before turning into ghost / bounce counter for ghost form
// temp5: how far to move up when turning into ghost
// temp6: damage done to Cain in current phase
// temp7: ducking flag for shooting, death counter in unused think function
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_cainwait        = {CAINSTANDLSPR, CAINSTANDRSPR, step, false, push_down, 6, 0, 0, NULL, NULL, DrawReact, &s_cainwait};
FARSTATE s_cainstand       = {CAINSTANDLSPR, CAINSTANDRSPR, step, false, push_down, 6, 0, 0, CainStandThink, NULL, DrawReact, &s_cainstand};
FARSTATE s_caindead1       = {CAINDEAD1SPR, CAINDEAD1SPR, step, false, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_caindead2};
FARSTATE s_caindead2       = {CAINDEAD2SPR, CAINDEAD2SPR, step, false, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_caindead1};
FARSTATE s_cainattack1     = {CAINSTANDLSPR, CAINSTANDRSPR, step, true, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_cainattack2};
FARSTATE s_cainattack2     = {CAINSTANDLSPR, CAINSTANDRSPR, step, true, push_down, 4, 0, 0, NULL, NULL, DrawReact, &s_cainattack3};
FARSTATE s_cainattack3     = {CAINSTANDLSPR, CAINSTANDRSPR, step, false, push_down, 6, 0, 0, CainAttackThink, NULL, DrawReact, &s_cainattack2};
FARSTATE s_cainduckattack1 = {CAINDUCKLSPR, CAINDUCKRSPR, step, true, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_cainduckattack2};
FARSTATE s_cainduckattack2 = {CAINDUCKLSPR, CAINDUCKRSPR, step, true, push_down, 4, 0, 0, NULL, NULL, DrawReact, &s_cainduckattack3};
FARSTATE s_cainduckattack3 = {CAINDUCKLSPR, CAINDUCKRSPR, step, false, push_down, 6, 0, 0, CainAttackThink, NULL, DrawReact, &s_cainduckattack2};
FARSTATE s_cainduck        = {CAINDUCKLSPR, CAINDUCKRSPR, step, true, push_down, 15, 0, 0, NULL, NULL, DrawReact, &s_cainstand};
FARSTATE s_cainexplode     = {CAINSTANDLSPR, CAINSTANDRSPR, stepthink, false, push_down, 6, 0, 0, ExplodeThink, NULL, DrawReact, &s_cainexplode};
FARSTATE s_caintransform1  = {CAINSTANDLSPR, CAINSTANDRSPR, stepthink, false, push_none, 10, 0, 0, CainTransformThink, NULL, DrawReact, &s_caintransform2};
FARSTATE s_caintransform2  = {CAINSTANDLSPR, CAINSTANDRSPR, stepthink, false, push_none, 10, 0, 0, CainTransformThink, NULL, DrawReact, &s_caintransform1};
FARSTATE s_cainghost1      = {CAINGHOST1LSPR, CAINGHOST1RSPR, slide, false, push_none, 10, 24, 24, NULL, NULL, CainGhostReact, &s_cainghost2};
FARSTATE s_cainghost2      = {CAINGHOST2LSPR, CAINGHOST2RSPR, slide, false, push_none, 10, 24, 24, NULL, NULL, CainGhostReact, &s_cainghost1};

/*
==================
=
= SpawnCain
=
==================
*/

void SpawnCain(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = cainobj;
	newobj->active = allways;
	newobj->needtoreact = true;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 48);
	newobj->xdir = -1;
	newobj->ydir = 1;
	newobj->hitnorth = 1;
	newobj->temp6 = newobj->temp7 = 0;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		newobj->health = 300;
		break;
	default:
		newobj->health = 250;
		break;
	}
	bosshealth = newobj->health;
	oldhealthbarlength = -1;
	bossdiv = bosshealth / 20;
	if (bossActive)
	{
		NewState(newobj, &s_cainstand);
		newobj->shootable = true;
	}
	else
	{
		NewState(newobj, &s_cainwait);
		newobj->shootable = false;
	}
}

/*
==================
=
= CainStandThink
=
==================
*/

void CainStandThink(objtype *ob)
{
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT()/16;
		ob->shootable = false;
		ChangeState(ob, &s_cainexplode);
	}
	else if (ob->temp6 >= 25 || player->bottom+2*TILEGLOBAL < ob->top)	//temp6 counts the damage done to Cain (see HurtObject())
	{
		ob->state = &s_caintransform1;
		ob->x -= 8*PIXGLOBAL;
		ob->y -= 8*PIXGLOBAL;
		ob->temp5 = ob->y - 4*TILEGLOBAL;
		ob->temp6 = 0;	//reset damage count
	}
	else if (ob->temp2 <= 0)
	{
		if (ob->x > player->x)
		{
			ob->xdir = -1;
		}
		else
		{
			ob->xdir = 1;
		}
		ob->temp1 = 1;	// number of shots to spawn
		ob->temp2 = US_RndT() >> 6;	// number of "think" calls to wait before starting to shoot again (0..3)
		if (US_RndT() > 155)
		{
			ob->temp7 = 1;
			ob->state = &s_cainduckattack1;
		}
		else
		{
			ob->state = &s_cainattack1;
		}
	}
	else
	{
		ob->temp2--;
	}
}

/*
==================
=
= CainAttackThink
=
==================
*/

void CainAttackThink(objtype *ob)
{
	Sint16 shotx;

	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
	ob->needtoreact = true;
	if (--ob->temp1 == 0)
	{
		ob->state = &s_cainstand;
	}
	if (ob->xdir == 1)
	{
		shotx = ob->x + 24*PIXGLOBAL;
	}
	else
	{
		shotx = ob->x - 8*PIXGLOBAL;
	}
	if (ob->temp7)	// if Cain is ducking
	{
		if (SpawnEnemyShot(shotx, ob->y+24*PIXGLOBAL, &s_cainshot1) == -1)
		{
			return;
		}
		ob->temp7 = 0;
		ob->state = &s_cainduck;	// always stop shooting
		// Note: Cain only ever fires one shot at a time, otherwise this state
		// change would prevent Cain from firing the remaining shots.
	}
	else
	{
		if (SpawnEnemyShot(shotx, ob->y+8*PIXGLOBAL, &s_cainshot1) == -1)
		{
			return;
		}
	}
	newobj->priority = 2;
	newobj->xspeed = ob->xdir * 60;
	newobj->yspeed = 0;
	SD_PlaySound(LASERSND);
}

/*
==================
=
= CainTransformThink
=
==================
*/

void CainTransformThink(objtype *ob)
{
	// Note: temp4 will initially be 0 when entering the transform state because
	// it starts at 0 when Cain spawns and CainGhostReact always sets it back to
	// 0 when turning into Cain's normal form again.
	if (ob->temp4 > 0)
	{
		if (--ob->temp4 <= 0)
		{
			ob->state = &s_cainghost1;
			ob->temp4 = 3;	// number of times the ghost must hit the floor to land
			ob->ydir = 1;
			SD_PlaySound(GROWLSND);
			ob->shootable = false;
			return;
		}
		else
		{
			return;
		}
	}
	if (ob->x < player->x)
	{
		ob->xdir = 1;
	}
	else
	{
		ob->xdir = -1;
	}
	if (ob->y > ob->temp5)
	{
		AccelerateY(ob, -1, 24);
	}
	else
	{
		ob->ydir = 0;
		ob->yspeed = 0;
		ob->temp4 = US_RndT() >> 3;	// wait a random amount of frames (0..31)
	}
}

/*
==================
=
= CainGhostReact
=
==================
*/

void CainGhostReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		if (--ob->temp4 <= 0)
		{
			ob->state = &s_cainstand;
			ob->y += 8*PIXGLOBAL;
			ob->x += 8*PIXGLOBAL;
			ob->temp4 = 0;
			ob->ydir = 0;
			ob->shootable = true;
			return;
		}
		// bounce off the floor:
		ob->ydir = -1;
		if (ob->x < player->x)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
	}
	else if (ob->hitsouth)
	{
		// bounce off the ceiling:
		if (ob->x < player->x)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
		ob->ydir = 1;
	}
	// bounce off walls:
	if (ob->hitwest)
	{
		ob->xdir = -1;
	}
	else if (ob->hiteast)
	{
		ob->xdir = 1;
	}
	PLACESPRITE;
}

/*
==================
=
= CainDeadThink		(never used!)
=
==================
*/

void CainDeadThink(objtype *ob)
{
	if (++ob->temp7 > 100)
	{
		StartMusic(MUS_HIGHSCORELEVEL);
		gamestate.mapon = 12;
		BossDialog();
		gamestate.mapon = 11;
		StartMusic(MUS_VICTORY);
		// BUG: Starting a new music track might move memory buffers around!
		// You should call RF_ForceRefresh or RF_NewPosition to avoid issues
		// with animated tiles.
		ob->hitnorth = 1;
	}
}
#endif	// (EPISODE == 3)

#ifndef BETA
//////////////////////////////////////////////////////////////////////////////
//
// BEACON
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_beacon  = {BEACON1SPR, BEACON1SPR, step, false, push_down, 10, 0, 0, NULL, BeaconContact, DrawReact, &s_beacon};
FARSTATE s_beacon1 = {BEACON2SPR, BEACON2SPR, step, false, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_beacon2};
FARSTATE s_beacon2 = {BEACON3SPR, BEACON3SPR, step, false, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_beacon3};
FARSTATE s_beacon3 = {BEACON4SPR, BEACON4SPR, step, false, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_beacon4};
FARSTATE s_beacon4 = {BEACON5SPR, BEACON5SPR, step, false, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_beacon1};

/*
==================
=
= SpawnBeacon
=
==================
*/

void SpawnBeacon(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	newobj->obclass = decoobj;
	newobj->priority = 2;
	newobj->x = CONVERT_TILE_TO_GLOBAL(x);
	newobj->y = SPAWN_ADJUST_Y(y, 48);
	newobj->xdir = 1;
	newobj->ydir = 1;
	newobj->hitnorth = 1;
	NewState(newobj, &s_beacon);
}

/*
==================
=
= BeaconContact
=
==================
*/

void BeaconContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj)
	{
		respawnx = player->x;
		respawny = player->y;
		ChangeState(ob, &s_beacon1);
		SD_PlaySound(BEACONSND);
		if (!gamestate.hint_beacon)
		{
			HelpMessage("Continue from beacons when you get killed.\n");
			gamestate.hint_beacon = true;
		}
	}
	ob++;
}
#endif	// ifndef BETA

REFKEEN_NS_E
