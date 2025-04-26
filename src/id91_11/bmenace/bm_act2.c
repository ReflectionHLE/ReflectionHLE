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

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

static Uint16 flowertime[] = {700, 700, 350, 175};	//never used! (Keen Dreams leftovers)

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

boolean robopal_mustfire;

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
==================
=
= CeilWalkReact
=
= React function for walking on ceilings
=
==================
*/

void CeilWalkReact(objtype *ob)
{
	if (ob->xdir == 1 && ob->hitwest)
	{
		ob->x -= ob->xmove;
		ob->xdir = -1;
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
	}
	else if (ob->xdir == -1 && ob->hiteast)
	{
		ob->x -= ob->xmove;
		ob->xdir = 1;
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
	}
	else if (!ob->hitsouth)
	{
		ob->x -= ob->xmove*2;
		ob->y -= ob->ymove;
		ob->xdir = -ob->xdir;
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// BONUS ITEMS
//
// temp1: bonus type (info plane icon number of the bonus)
// temp2: base shape number
// temp3: last animated shape number +1
//
//////////////////////////////////////////////////////////////////////////////

#if 1
//the original code:
FARSTATE s_bonus1    = {0, 0, step,  false, push_none, 20, 0, 0, BonusThink, NULL, DrawReact, &s_bonus2};
FARSTATE s_bonus2    = {0, 0, step,  false, push_none, 20, 0, 0, BonusThink, NULL, DrawReact, &s_bonus1};
FARSTATE s_bonusrise = {0, 0, slide, false, push_none, 40, 0, 8, FlyBonusThink, NULL, DrawReact, NULL};
#else
//the modified version with s_bonus2 as a dedicated falling bonus state:
FARSTATE s_bonus1    = {0, 0, step,  false, push_none, 20, 0, 0, BonusThink, NULL, DrawReact, &s_bonus1};
FARSTATE s_bonus2    = {0, 0, stepthink, false, push_none, 20, 0, 0, FlyBonusThink, NULL, DrawReact, &s_bonus2};
FARSTATE s_bonusrise = {0, 0, slide, false, push_none, 40, 0, 8, NULL, NULL, DrawReact, NULL};
/*
You don't need to have two states for the regular bonus items, the think code
takes care of the animation anyway. This allows you to re-use the second bonus
state as a flying/falling bonus state. This is basically what you will find in
Keen 5 and 6, except with 3 states instead of 5. Keep in mind that there is a
design flaw in the implementation of FlyBonusThink when used like this: 
The think code is executed for every frame, not just every 20 timer tics, which
increases the animation speed. The same flaw is also present in Keen 6.
*/
#endif

static Uint16 bonusshape[] =
{
	ITEMGREENGRENADE1SPR,        // 0
	ITEMMACHINEGUN1SPR,          // 1
	ITEMKEYCARD1SPR,             // 2
	ITEMKEY1SPR,                 // 3
	ITEMBOTTLE1SPR,              // 4
	ITEMVIAL1SPR,                // 5
	ITEMGIZMO1SPR,               // 6
	ITEMBOOK1SPR,                // 7
	ITEMTAPEDECK1SPR,            // 8
	ITEMCOMPUTER1SPR,            // 9
	ITEMEXTRALIFE1SPR,           //10
	ITEMBLUESHARD1SPR,           //11
	ITEMGREENSHARD1SPR,          //12
	ITEMREDSHARD1SPR,            //13
	ITEMCYANSHARD1SPR,           //14
	ITEMSPECIALKEY1SPR,          //15
	BONUS1UPSPR,                 //16
	BONUS100SPR,                 //17
	BONUS200SPR,                 //18
	BONUS500SPR,                 //19
	BONUS800SPR,                 //20
	BONUS1000SPR,                //21
	BONUS1500SPR,                //22
	BONUS2000SPR,                //23
	BONUS5000SPR,                //24
	BONUS50000SPR,               //25
#ifdef BETA
	ITEMBEER1SPR,                //26
	ITEMMEAL1SPR,                //27
#else
	ITEMGEM1SPR,                 //26
	ITEMFIRSTAID1SPR,            //27
#endif
	ITEMNUKE1SPR,                //28
	ITEMANTIRADIATIONPILL1SPR,   //29
	ITEMINVINCIBILITYPOTION1SPR, //30
	ITEMDIAMOND1SPR,             //31
	ITEMORB1SPR,                 //32
	ITEMEXITKEY1SPR,             //33
#ifdef BETA
	0,
	0,
#else
	ITEMREDGRENADE1SPR,          //34
	ITEMSUPERGUN1SPR,            //35
	ITEMLANDMINES1SPR,           //36
	ITEMPLASMABOLTS1SPR,         //37
	ITEMWARPGEM1SPR              //38
#endif
};

/*
==================
=
= SpawnBonus
=
==================
*/

void SpawnBonus(Uint16 x, Uint16 y, Uint16 num)
{
	Uint16 bonusindex;

	bonusindex = 0;
	GetNewObj(false);
	new->needtoclip = cl_noclip;
	new->obclass = bonusobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->ydir = -1;
	new->temp1 = num;
	switch (num)
	{
#ifdef BETA
	case 80:
		bonusindex = 33;
		break;
#endif
	case 9:
		bonusindex = 0;
		break;
	case 16:
		bonusindex = 1;
		break;
	case 17:
		bonusindex = 2;
		break;
	case 18:
		bonusindex = 3;
		break;
	case 19:
		bonusindex = 4;
		break;
	case 20:
		bonusindex = 5;
		break;
	case 21:
		bonusindex = 6;
		break;
	case 22:
		bonusindex = 7;
		break;
	case 23:
		bonusindex = 8;
		break;
	case 24:
		bonusindex = 9;
		break;
	case 25:
		bonusindex = 10;
		break;
	case 26:
		bonusindex = 11;
		break;
	case 27:
		bonusindex = 12;
		break;
	case 28:
		bonusindex = 13;
		break;
	case 29:
		bonusindex = 14;
		break;
	case 30:
		bonusindex = 15;
		break;
#ifndef BETA
	case 36:
		bonusindex = 34;
		break;
	case 37:
		bonusindex = 35;
		break;
	case 38:
		bonusindex = 36;
		break;
	case 39:
		bonusindex = 37;
		break;
#endif
	case 52:
		bonusindex = 26;
		break;
	case 53:
		bonusindex = 27;
		break;
#ifndef BETA
	case 54:
		bonusindex = 38;
		break;
#endif
	case 75:
		bonusindex = 28;
		break;
	case 76:
		bonusindex = 29;
		break;
	case 77:
		bonusindex = 30;
		break;
	case 78:
		bonusindex = 31;
		break;
	case 79:
		bonusindex = 32;
		break;
#ifndef BETA
	case 80:
		bonusindex = 33;
		break;
#endif
	}
	new->temp2 = new->shapenum = bonusshape[bonusindex];
	new->temp3 = new->temp2 + 2;
	NewState(new, &s_bonus1);
}

/*
==================
=
= BonusThink
=
==================
*/

void BonusThink(objtype *ob)
{
	if (++ob->shapenum == ob->temp3)
	{
		ob->shapenum = ob->temp2;
	}
}

/*
==================
=
= FlyBonusThink
=
==================
*/

// Note: FlyBonusThink is only executed when s_bonusrise is about to transition
// to the next state, which is NULL and will remove the object. That makes this
// think function rather pointless. This function is identical to the T_FlyBonus
// function from Keen 5 and 6 (see K5_ACT1.C and K6_ACT1.C in my reconstructed
// Keen 4-6 code), but it isn't used correctly in the original BioMenace code.
// Keen 5 and 6 had extra states for the falling version of the bonus items
// (used for the key gems dropped by Blooglets in Keen 6) and those states don't
// exist in BioMenace. Maybe the keys that some of the bosses spawn when they
// get killed were supposed to fall to the ground at some point, but that was
// never implemented correctly. Adding it would really help, since E2L5 (Queen
// Ant) can become unwinnable if killing the Queen spawns the key where the
// player cannot reach it. If you use the modified states as shown above, you
// can make the key that the Queen spawns fall to the ground by enabling the
// bugfix I added after the SpawnBonus call that spawns the Queen's key in the
// HurtObject function in BM_SNAK2.C.
void FlyBonusThink(objtype *ob)
{
	if (ob->hitnorth)
	{
		ob->state = &s_bonus1;
	}
	if (++ob->shapenum == ob->temp3)
	{
		ob->shapenum = ob->temp2;
	}
	DoGravity(ob);
}

#if (EPISODE != 1)
//////////////////////////////////////////////////////////////////////////////
//
// BUG (SMALL RED MUTANT INSECT)
//
// temp1: jumptime (time without gravity)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_bug1 = {BUG1LSPR, BUG1RSPR, stepthink, false, push_none, 20, 0, 0, BugThink, NULL, BugReact, &s_bug2};
FARSTATE s_bug2 = {BUG2LSPR, BUG2RSPR, stepthink, false, push_none, 20, 0, 0, BugThink, NULL, BugReact, &s_bug1};

/*
==================
=
= SpawnBug
=
==================
*/

void SpawnBug(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = bugobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 24);
	new->xdir = 1;
	new->shootable = true;
#ifdef BETA
	new->health = 2;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 4;
		break;
	default:
		new->health = 2;
		break;
	}
#endif
	NewState(new, &s_bug1);
}

/*
==================
=
= BugThink		(a carbon copy of TomatBounceThink from KEEN DREAMS)
=
==================
*/

void BugThink(objtype *ob)
{
	AccelerateX(ob, (ob->x > player->x)? -1 : 1, 16);
	if (ob->xspeed > 0)
	{
		ob->xdir = 1;
	}
	else
	{
		ob->xdir = -1;
	}
	// BUG: BioMenace is not Keen Dreams. The amount the object should move must
	// be stored in the global variable ytry, not in the object's ymove field.
	// This basically makes the Bug not move up/down at all when temp1 is not 0.
	if (ob->temp1)
	{
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
}

/*
==================
=
= BugReact
=
==================
*/

void BugReact(objtype *ob)
{
	if (ob->hiteast || ob->hitwest)
	{
		ob->xdir = -ob->xdir;
		ob->xspeed = -ob->xspeed;
	}
	if (ob->hitsouth)
	{
		ob->yspeed = -ob->yspeed;
	}
	if (ob->hitnorth)
	{
		ob->yspeed = -15 - (US_RndT() >> 4);
		ob->temp1 = 10;
		ChangeState(ob, &s_bug1);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// BEE (& NYMPH)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_bee1 = {BEE1LSPR, BEE1RSPR, stepthink, false, push_none, 1, 0, 0, BeeThink, NULL, BeeReact, &s_bee2};
FARSTATE s_bee2 = {BEE2LSPR, BEE2RSPR, stepthink, false, push_none, 1, 0, 0, BeeThink, NULL, BeeReact, &s_bee1};

/*
==================
=
= SpawnBee
=
==================
*/

void SpawnBee(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = beeobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 24);
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
#ifdef BETA
	new->health = 1;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 3;
		break;
	default:
		new->health = 1;
		break;
	}
#endif
	NewState(new, &s_bee1);
}

#if (EPISODE == 2)
FARSTATE s_nymph1 = {NYMPH1SPR, NYMPH1SPR, stepthink, false, push_none, 1, 0, 0, BeeThink, NULL, BeeReact, &s_nymph2};
FARSTATE s_nymph2 = {NYMPH2SPR, NYMPH2SPR, stepthink, false, push_none, 1, 0, 0, BeeThink, NULL, BeeReact, &s_nymph1};

/*
==================
=
= SpawnNymph
=
==================
*/

void SpawnNymph(objtype *ob)
{
	GetNewObj(false);
	new->obclass = beeobj;
	new->x = ob->x + 16*PIXGLOBAL;
	new->y = ob->y + 24*PIXGLOBAL;
	new->active = allways;
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
#ifdef BETA
	new->health = 1;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 3;
		break;
	default:
		new->health = 1;
		break;
	}
#endif
	NewState(new, &s_nymph1);
}
#endif	// (EPISODE == 2)

/*
==================
=
= BeeThink
=
==================
*/

void BeeThink(objtype *ob)
{
	Sint16 xdist;

	AccelerateY(ob, (ob->y > player->y)? -1 : 1, US_RndT()/8 + 10);
	if (ob->yspeed > 0)
	{
		ob->ydir = 1;
	}
	else
	{
		ob->ydir = -1;
	}
	AccelerateX(ob, ob->xdir, (US_RndT()/8) << 1);
	if (ob->xspeed < 0)
	{
		xdist = ob->left - player->right;
		if (xdist < -8*PIXGLOBAL)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
	}
	else
	{
		xdist = player->left - ob->right;
		if (xdist < -8*PIXGLOBAL)
		{
			ob->xdir = -1;
		}
		else
		{
			ob->xdir = 1;
		}
	}
}

/*
==================
=
= BeeReact
=
==================
*/

void BeeReact(objtype *ob)
{
	if (ob->top > player->top + 24)
	{
		ob->ydir = -1;
	}
	else if (ob->top < player->top + 24)
	{
		ob->ydir = 1;
	}
	if (ob->hiteast || ob->hitwest)
	{
		ob->xdir = -ob->xdir;
		ob->xspeed = (US_RndT() * -ob->xspeed)/8;
	}
	if (ob->hitnorth)
	{
		ob->yspeed = -40 - US_RndT()/2;
	}
	if (ob->hitsouth)
	{
		ob->yspeed = US_RndT()/2 + 40;
	}
	PLACESPRITE;
}
#endif	// (EPISODE != 1)

//////////////////////////////////////////////////////////////////////////////
//
// ROBOPAL
//
// temp7: chase counter (teleport to player after a certain amount of time)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_robopalwait1     = {ROBOPAL1SPR, ROBOPAL1SPR, step, false, push_none, 15, 0, 0, NULL, NULL, DrawReact, &s_robopalwait2};
FARSTATE s_robopalwait2     = {ROBOPAL2SPR, ROBOPAL2SPR, step, false, push_none, 15, 0, 0, NULL, NULL, DrawReact, &s_robopalwait1};
FARSTATE s_robopalfollow1   = {ROBOPAL1SPR, ROBOPAL1SPR, stepthink, false, push_none, 5, 0, 0, RobopalThink, NULL, RobopalReact, &s_robopalfollow2};
FARSTATE s_robopalfollow2   = {ROBOPAL2SPR, ROBOPAL2SPR, stepthink, false, push_none, 5, 0, 0, RobopalThink, NULL, RobopalReact, &s_robopalfollow1};
FARSTATE s_robopalteleport1 = {ROBOPAL3SPR, ROBOPAL3SPR, step, false, push_none, 25, 0, 0, NULL, NULL, DrawReact, &s_robopalteleport2};
FARSTATE s_robopalteleport2 = {ROBOPAL4SPR, ROBOPAL4SPR, step, false, push_none, 25, 0, 0, NULL, NULL, DrawReact, &s_robopalteleport3};
FARSTATE s_robopalteleport3 = {-1, -1, step, false, push_none, 1, 0, 0, RobopalTeleportThink, NULL, DrawReact, &s_robopalteleport4};
FARSTATE s_robopalteleport4 = {ROBOPAL4SPR, ROBOPAL4SPR, step, false, push_none, 25, 0, 0, NULL, NULL, DrawReact, &s_robopalteleport5};
FARSTATE s_robopalteleport5 = {ROBOPAL3SPR, ROBOPAL3SPR, step, false, push_none, 25, 0, 0, NULL, NULL, DrawReact, &s_robopalfollow1};

/*
==================
=
= SpawnRobopal
=
==================
*/

void SpawnRobopal(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->active = allways;
	new->obclass = robopalobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 24);
	new->xdir = 1;
	new->ydir = 1;
	new->temp7 = 0;
	new->shootable = false;
#ifdef BETA
	new->priority = 3;
#endif
	robopal_mustfire = false;
	NewState(new, &s_robopalwait1);
}

/*
==================
=
= RobopalThink
=
==================
*/

void RobopalThink(objtype *ob)
{
	if (!gamestate.got_robopal)
	{
		ChangeState(ob, &s_robopalteleport1);
		return;
	}
	if (ob->needtoclip == cl_noclip)
	{
		ob->needtoclip = cl_midclip;
	}
	if (ob->temp7 > 300)
	{
		ob->state = &s_robopalteleport1;
		return;
	}
	AccelerateY(ob, ob->y>player->y+8*PIXGLOBAL? -1 : 1, 10);
	if (ob->yspeed > 0)
	{
		ob->ydir = 1;
	}
	else
	{
		ob->ydir = -1;
	}
	if (player->xdir == 1)
	{
		if (!ob->hitnorth && !ob->hitsouth)
		{
			if (ob->x > player->x-18*PIXGLOBAL)
			{
				ob->xdir = -1;
			}
			else if (ob->x > player->x-40*PIXGLOBAL && ob->x < player->x-35*PIXGLOBAL)
			{
				ob->xspeed = 0;
				ob->temp7 = 0;	// reset chase counter
				ob->xdir = player->xdir;
				return;
			}
		}
		ob->temp7++;
		AccelerateX(ob, ob->x>player->x-40*PIXGLOBAL? -1 : 1, 16);
	}
	else if (player->xdir == -1)
	{
		if (!ob->hitnorth && !ob->hitsouth)
		{
			if (ob->x < player->x+40)
			{
				ob->xdir = 1;
			}
			else if (ob->x > player->x+40*PIXGLOBAL && ob->x < player->x+45*PIXGLOBAL)
			{
				ob->xspeed = 0;
				ob->temp7 = 0;	// reset chase counter
				ob->xdir = player->xdir;
				return;
			}
		}
		ob->temp7++;
		AccelerateX(ob, ob->x>player->x+40*PIXGLOBAL? -1 : 1, 16);
	}
	if (ob->xspeed > 0)
	{
		ob->xdir = 1;
	}
	else
	{
		ob->xdir = -1;
	}
}

/*
==================
=
= RobopalReact
=
==================
*/

void RobopalReact(objtype *ob)
{
	if (ob->top > player->top+8*PIXGLOBAL)
	{
		ob->ydir = -1;
	}
	else if (ob->top < player->top+8*PIXGLOBAL)
	{
		ob->ydir = 1;
	}
	if (ob->hiteast || ob->hitwest)
	{
		ob->xdir = -ob->xdir;
		ob->xspeed = -ob->xspeed;
	}
	if (ob->hitnorth)
	{
		if (ob->x < player->x)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
	}
	if (ob->hitsouth)
	{
		if (ob->x < player->x)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
	}
	if (robopal_mustfire)
	{
		if (player->xdir == 1)
		{
			SpawnShot(ob->midx, ob->top+4*PIXGLOBAL, player->xdir);
		}
		else
		{
			SpawnShot(ob->midx-8*PIXGLOBAL, ob->top+4*PIXGLOBAL, player->xdir);
		}
		robopal_mustfire = false;
	}
	PLACESPRITE;
}

/*
==================
=
= RobopalTeleportThink
=
==================
*/

void RobopalTeleportThink(objtype *ob)
{
	if (!gamestate.got_robopal)
	{
		RemoveObj(ob);
		return;
	}
	ob->needtoclip = cl_noclip;
	ob->x = player->x;
	ob->y = player->y;
	ob->temp7 = 0;	// reset chase counter
}

//////////////////////////////////////////////////////////////////////////////
//
// WORM & PLASMA WORM & GRENADE FIRE 
//
// temp7: time to live (grenade fire)
//        turn cooldown for WalkReact (worm & plasma worm)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_wormwalk1    = {PINKWORM1LSPR, PINKWORM1RSPR, step, false, push_down, 10, 128, 0, WormWalkThink, NULL, WalkReact, &s_wormwalk2};
FARSTATE s_wormwalk2    = {PINKWORM2LSPR, PINKWORM2RSPR, step, false, push_down, 10, 128, 0, WormWalkThink, NULL, WalkReact, &s_wormwalk1};
FARSTATE s_wormjump     = {PINKWORM3LSPR, PINKWORM3RSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, WormJumpReact, &s_wormwalk1};
#ifndef BETA
FARSTATE s_grenadefire1 = {BIGFIRE1SPR, BIGFIRE1SPR, step, false, push_down, 5, 64, 0, GrenadeFireThink, GrenadeExplosionContact, GrenadeFireReact, &s_grenadefire2};
FARSTATE s_grenadefire2 = {BIGFIRE2SPR, BIGFIRE2SPR, step, false, push_down, 5, 64, 0, GrenadeFireThink, GrenadeExplosionContact, GrenadeFireReact, &s_grenadefire1};
#endif
#if (EPISODE == 1)
FARSTATE s_plasmawormwalk1 = {PLASMAWORM1LSPR, PLASMAWORM1RSPR, step, false, push_down, 10, 128, 0, PlasmaWormWalkThink, NULL, WalkReact, &s_plasmawormwalk2};
FARSTATE s_plasmawormwalk2 = {PLASMAWORM2LSPR, PLASMAWORM2RSPR, step, false, push_down, 10, 128, 0, PlasmaWormWalkThink, NULL, WalkReact, &s_plasmawormwalk1};
FARSTATE s_plasmawormjump  = {PLASMAWORM1LSPR, PLASMAWORM1RSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, PlasmaWormJumpReact, &s_plasmawormwalk1};
#endif

// Note: The fact that the grenadefire states and functions are declared here
// and not in the same spot as the grenades themselves might suggest that this
// was originally supposed to be the projectile of one of these enemies.

/*
==================
=
= SpawnWorm
=
==================
*/

void SpawnWorm(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = wormobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y) + -0x71;
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_wormwalk1);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 0x20;
	new->shootable = true;
#ifdef BETA
	new->health = 1;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 3;
		break;
	default:
		new->health = 1;
		break;
	}
#endif
}

#if (EPISODE != 2)
/*
==================
=
= SpawnPlasmaWorm
=
==================
*/

//#pragma argsused	// because it's empty in episode 3
void SpawnPlasmaWorm(Uint16 x, Uint16 y)
{
	// Episode 3 has an empty function here, chances are it's SpawnPlasmaWorm.
#if (EPISODE == 1)
	GetNewObj(false);
	new->obclass = wormobj;
	new->priority = 2;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_plasmawormwalk1);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 0x20;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 10;
		break;
	default:
		new->health = 5;
		break;
	}
#endif
}
#endif	//  (EPISODE != 2)

/*
==================
=
= WormWalkThink
=
==================
*/

void WormWalkThink(objtype *ob)
{
	Sint16 dist;

	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	
	if (ob->xdir == -1)
	{
		dist = ob->left - player->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = 1;
			return;
		}
		ob->yspeed = -24 - (US_RndT() / 0x10);
		ob->xspeed = -32;
		SD_PlaySound(BUGSCREAMSND);
		ob->state = &s_wormjump;
		return;
	}
	else
	{
		dist = player->left - ob->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = -1;
			return;
		}
		ob->yspeed = -24 - (US_RndT() / 0x10);
		ob->xspeed = 32;
		SD_PlaySound(BUGSCREAMSND);
		ob->state = &s_wormjump;
		return;
	}
}

#if (EPISODE == 1)
/*
==================
=
= PlasmaWormWalkThink
=
==================
*/

void PlasmaWormWalkThink(objtype *ob)
{
	Sint16 dist;

	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	
	if (ob->xdir == -1)
	{
		dist = ob->left - player->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = 1;
			return;
		}
		ob->yspeed = -24 - (US_RndT() / 8);
		ob->xspeed = -32 - (US_RndT() / 8);
		SD_PlaySound(BUGSCREAMSND);
		ob->state = &s_plasmawormjump;
		return;
	}
	else
	{
		dist = player->left - ob->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = -1;
			return;
		}
		ob->yspeed = -24 - (US_RndT() / 8);
		ob->xspeed = 32 + (US_RndT() / 8);
		SD_PlaySound(BUGSCREAMSND);
		ob->state = &s_plasmawormjump;
		return;
	}
}
#endif	//  (EPISODE == 1)

#ifndef BETA
/*
==================
=
= GrenadeFireThink
=
==================
*/

void GrenadeFireThink(objtype *ob)
{
	if (ob->temp7-- <= 0)
	{
		ChangeState(ob, &s_grenadeexplosion1);
	}
	if (US_RndT() > 230)
	{
		FragBloom(ob->midx+2*PIXGLOBAL, ob->y-8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_East);
		SD_PlaySound(RICOCHET1SND);
	}
	if (US_RndT() > 230)
	{
		FragBloom(ob->midx-2*PIXGLOBAL, ob->y-8*PIXGLOBAL, FRAG_SPAREPLAYER+dir_West);
		SD_PlaySound(RICOCHET1SND);
	}
}
#endif

/*
==================
=
= WormJumpReact
=
==================
*/

void WormJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_wormwalk1);
		ob->nothink = 12;
	}
	PLACESPRITE;
}

#if (EPISODE == 1)
/*
==================
=
= PlasmaWormJumpReact
=
==================
*/

void PlasmaWormJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_plasmawormwalk1);
		ob->nothink = 12;
	}
	PLACESPRITE;
}
#endif

#ifndef BETA
/*
==================
=
= GrenadeFireReact
=
==================
*/

void GrenadeFireReact(objtype *ob)
{
	if (ob->xdir == 1 && ob->hitwest)
	{
		ChangeState(ob, &s_grenadeexplosion1);
	}
	else if (ob->xdir == -1 && ob->hiteast)
	{
		ChangeState(ob, &s_grenadeexplosion1);
	}
	else if (!ob->hitnorth)
	{
		ChangeState(ob, &s_grenadeexplosion1);
	}
	PLACESPRITE;
}
#endif

#if (EPISODE != 1)
//////////////////////////////////////////////////////////////////////////////
//
// BLUE DEVIL
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_bluedevilwalk1 = {BLUEDEVILWALK1LSPR, BLUEDEVILWALK1RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, BlueDevilWalkReact, &s_bluedevilwalk2};
FARSTATE s_bluedevilwalk2 = {BLUEDEVILWALK2LSPR, BLUEDEVILWALK2RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, BlueDevilWalkReact, &s_bluedevilwalk3};
FARSTATE s_bluedevilwalk3 = {BLUEDEVILWALK3LSPR, BLUEDEVILWALK3RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, BlueDevilWalkReact, &s_bluedevilwalk4};
FARSTATE s_bluedevilwalk4 = {BLUEDEVILWALK4LSPR, BLUEDEVILWALK4RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, BlueDevilWalkReact, &s_bluedevilwalk1};
FARSTATE s_bluedeviljump  = {BLUEDEVILJUMPLSPR,  BLUEDEVILJUMPRSPR,  think, false, push_none, 0, 0, 0, ProjectileThink, 0, BlueDevilJumpReact, NULL};

/*
==================
=
= SpawnBlueDevil
=
==================
*/

void SpawnBlueDevil(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = bluedevilobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 32);
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
#ifdef BETA
	new->health = 3;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 6;
		break;
	default:
		new->health = 3;
		break;
	}
#endif
	NewState(new, &s_bluedevilwalk1);
	new->hitnorth = 1;
}

/*
==================
=
= BlueDevilWalkReact
=
==================
*/

void BlueDevilWalkReact(objtype *ob)
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
		ob->x -= ob->xmove;
		ob->y -= ob->ymove;
		ob->yspeed = -((US_RndT()/32) * 20);
		if (ob->yspeed < -60)
		{
			ob->yspeed = -60;
		}
		if (ob->yspeed > -30)
		{
			ob->yspeed = -30;
		}
		ob->xspeed = ob->xdir << 5;
		ChangeState(ob, &s_bluedeviljump);
	}
	PLACESPRITE;
}

/*
==================
=
= BlueDevilJumpReact
=
==================
*/

void BlueDevilJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_bluedevilwalk1);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// KAMIKAZE
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_kamikazestand1 = {KAMIKAZE1LSPR, KAMIKAZE1LSPR, step, true, push_down, 10, 0, 0, NULL, NULL, WalkReact, &s_kamikazestand2};
FARSTATE s_kamikazestand2 = {KAMIKAZE2LSPR, KAMIKAZE2LSPR, step, true, push_down, 10, 0, 0, NULL, NULL, WalkReact, &s_kamikazeturn1};
FARSTATE s_kamikazeturn1  = {KAMIKAZE1RSPR, KAMIKAZE1LSPR, step, true, push_down, 10, 0, 0, NULL, NULL, WalkReact, &s_kamikazeturn2};
FARSTATE s_kamikazeturn2  = {KAMIKAZE2RSPR, KAMIKAZE2LSPR, step, true, push_down, 10, 0, 0, KamikazeTurnThink, NULL, WalkReact, &s_kamikazewalk1};
FARSTATE s_kamikazewalk1  = {KAMIKAZE3LSPR, KAMIKAZE3RSPR, step, true, push_down, 4, 128, 0, KamikazeWalkThink, NULL, WalkReact, &s_kamikazewalk2};
FARSTATE s_kamikazewalk2  = {KAMIKAZE4LSPR, KAMIKAZE4RSPR, step, true, push_down, 4, 128, 0, KamikazeWalkThink, NULL, WalkReact, &s_kamikazewalk1};
FARSTATE s_kamikazedie1   = {KAMIKAZEDIE1SPR, KAMIKAZEDIE1SPR, step, true, push_none, 14, 0, 0, NULL, NULL, DrawReact, &s_kamikazedie2};
FARSTATE s_kamikazedie2   = {KAMIKAZEDIE2SPR, KAMIKAZEDIE2SPR, step, true, push_none, 6, 0, 0, KamikazeDieThink, NULL, DrawReact, NULL};

/*
==================
=
= SpawnKamikaze
=
==================
*/

void SpawnKamikaze(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = kamikazeobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y) + (15-TILEGLOBAL);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_kamikazestand1);
	new->hitnorth = 1;
	new->ticcount = US_RndT()/32;
	new->shootable = true;
#ifdef BETA
	new->health = 3;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 6;
		break;
	default:
		new->health = 3;
		break;
	}
#endif
}

/*
==================
=
= KamikazeTurnThink
=
==================
*/

void KamikazeTurnThink(objtype *ob)
{
	if (player->x < ob->x)
	{
		ob->xdir = -1;
	}
	else
	{
		ob->xdir = 1;
	}
}

/*
==================
=
= KamikazeDieThink
=
==================
*/

void KamikazeDieThink(objtype *ob)
{
	SD_PlaySound(GRENADEXPLODESND);
	ob->nothink = 1;
	ChunkBloom(ob, ob->x, ob->y, 0);
	ChunkBloom(ob, ob->x, ob->y, 4);
	ChunkBloom(ob, ob->x, ob->y, 2);
	ChunkBloom(ob, ob->x, ob->y, 6);
	FragBloom(ob->x + 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, dir_East);
	FragBloom(ob->x - 2*PIXGLOBAL, ob->y - 8*PIXGLOBAL, dir_West);
	ob->obclass = explosionobj;
	ob->shootable = false;
	ChangeState(ob, &s_grenadeexplosion1);
}

/*
==================
=
= KamikazeWalkThink
=
==================
*/

void KamikazeWalkThink(objtype *ob)
{
	if (ob->bottom != player->bottom && US_RndT() < 16)
	{
		ob->state = &s_kamikazestand1;
		return;
	}
	if (player->right < ob->left || player->left > ob->right
		|| player->top > ob->bottom || player->bottom < ob->top)
	{
		return;
	}
	ob->state = &s_kamikazedie1;
}
#endif	// (EPISODE != 1)

#if (EPISODE == 2)
//////////////////////////////////////////////////////////////////////////////
//
// TRASH BOSS
//
// temp1: number of explosions to spawn during death animation
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_trashbossdie   = {TRASHBOSS3LSPR, TRASHBOSS3RSPR, stepthink, false, push_down, 6, 0, 0, ExplodeThink, NULL, DrawReact, &s_trashbossdie};
FARSTATE s_trashbossstand = {TRASHBOSS1LSPR, TRASHBOSS1RSPR, step, true, push_down, 10, 0, 0, TrashBossStandThink, NULL, TrashBossReact, &s_trashbossstand};
FARSTATE s_trashbosswalk1 = {TRASHBOSS1LSPR, TRASHBOSS1RSPR, slide, true, push_down, 10, 16, 0, NULL, NULL, DrawReact, &s_trashbosswalk2};
FARSTATE s_trashbosswalk2 = {TRASHBOSS2LSPR, TRASHBOSS2RSPR, slide, true, push_down, 10, 16, 0, TrashBossWalkThink, NULL, DrawReact, &s_trashbosswalk1};
FARSTATE s_trashbossthrow = {TRASHBOSS3LSPR, TRASHBOSS3RSPR, step, false, push_none, 10, 0, 0, TrashBossAttackThink, NULL, DrawReact, &s_trashbossstand};
FARSTATE s_trashcan1      = {TRASHCAN1SPR, TRASHCAN1SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashcan2};
FARSTATE s_trashcan2      = {TRASHCAN2SPR, TRASHCAN2SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashcan3};
FARSTATE s_trashcan3      = {TRASHCAN3SPR, TRASHCAN3SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashcan4};
FARSTATE s_trashcan4      = {TRASHCAN4SPR, TRASHCAN4SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashcan1};
FARSTATE s_trashapple1    = {TRASHAPPLE1SPR, TRASHAPPLE1SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashapple2};
FARSTATE s_trashapple2    = {TRASHAPPLE2SPR, TRASHAPPLE2SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashapple3};
FARSTATE s_trashapple3    = {TRASHAPPLE3SPR, TRASHAPPLE3SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashapple4};
FARSTATE s_trashapple4    = {TRASHAPPLE4SPR, TRASHAPPLE4SPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashapple1};
FARSTATE s_trashbird1     = {TRASHBIRD1LSPR, TRASHBIRD1RSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashbird2};
FARSTATE s_trashbird2     = {TRASHBIRD2LSPR, TRASHBIRD2RSPR, stepthink, false, push_none, 10, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_trashbird1};

/*
==================
=
= SpawnTrashBoss
=
==================
*/

void SpawnTrashBoss(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = trashbossobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 88);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_trashbossstand);
	new->hitnorth = 1;
	new->ticcount = US_RndT()/32;
	new->shootable = true;
#ifdef BETA
	new->health = 100;
#else
	new->health = bosshealth = oldhealthbarlength = 100;
	bossdiv = bosshealth / 20;
#endif
}

/*
==================
=
= TrashBossStandThink
=
==================
*/

void TrashBossStandThink(objtype *ob)
{
#ifndef BETA
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
#endif
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT()/8;	// number of explosions to spawn (0..31)
		ob->shootable = false;
		ChangeState(ob, &s_trashbossdie);
	}
	else if (US_RndT() < 50)
	{
		ob->state = &s_trashbosswalk1;
	}
}

/*
==================
=
= TrashBossWalkThink
=
==================
*/

void TrashBossWalkThink(objtype *ob)
{
	Sint16 xdist;

#ifndef BETA
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
#endif
	ob->needtoreact = true;
	if (ob->health <= 0)
	{
		ob->temp1 = US_RndT()/8;	// number of explosions to spawn (0..31)
		ob->shootable = false;
		ChangeState(ob, &s_trashbossdie);
	}
	else
	{
		if (US_RndT() < 40)
		{
			ob->state = &s_trashbossstand;
		}
		// another re-use of the FRENCHY code follows:
		if (abs(ob->y - player->y) > 3*TILEGLOBAL)		// turn randomly
		{
			if (US_RndT() < 8)
			{
				ob->xdir = -ob->xdir;
			}
		}
		else
		{
			xdist = player->x - ob->x;
			if (xdist < -8*TILEGLOBAL)
			{
			// walk closer
				ob->xdir = -1;
			}
			if (xdist < -4*TILEGLOBAL)	//BUG? this is NOT 'else if', which makes the 'if' check above rather useless
			{
			// throw
				ob->xdir = -1;
				ob->state = &s_trashbossthrow;
			}
			else if (xdist < 0)
			{
			// run away
				ob->xdir = -1;
				ob->nothink = 8;
			}
			else if (xdist < 4*TILEGLOBAL)
			{
			// run away
				ob->xdir = 1;
				ob->nothink = 8;
			}
			else if (xdist < 8*TILEGLOBAL)
			{
			// throw and walk closer
				ob->xdir = 1;
				ob->state = &s_trashbossthrow;
			}
			else
			{
			// walk closer
				ob->xdir = 1;
			}
		}
	}
}

/*
==================
=
= TrashBossReact
=
==================
*/

void TrashBossReact(objtype *ob)
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
	PLACESPRITE;
}

/*
==================
=
= TrashBossAttackThink
=
==================
*/

void TrashBossAttackThink(objtype *ob)
{
	Sint16 randnum = 0;

	ob->needtoreact = true;
	GetNewObj(true);
	new->obclass = enemyshotobj;
	if (ob->xdir == 1)
	{
		new->x = ob->x + 40*PIXGLOBAL;
		new->y = ob->y + 48*PIXGLOBAL;
	}
	else
	{
		new->x = ob->x + 8*PIXGLOBAL;
		new->y = ob->y + 48*PIXGLOBAL;
	}
	new->xdir = ob->xdir;
	new->ydir = 1;
	new->xspeed = ob->xdir * 60 - (US_RndT() >> 4);
	new->yspeed = -30;
	new->active = removable;
	randnum = US_RndT();
	if (randnum < 85)
	{
		NewState(new, &s_trashcan1);
	}
	else if (randnum >= 85 && randnum < 170)
	{
		NewState(new, &s_trashapple1);
	}
	else if (randnum >= 170)
	{
		NewState(new, &s_trashbird1);
	}
	ob->nothink = 2;
}
#else		// (EPISODE != 2) now!
//////////////////////////////////////////////////////////////////////////////
//
// JUMP BOMB
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_jumpbomb     = {JUMPBOMB1LSPR, JUMPBOMB1RSPR, step, false, push_down, 10, 0, 0, JumpbombThink, NULL, DrawReact, &s_jumpbomb};
FARSTATE s_jumpbombfly1 = {JUMPBOMB2LSPR, JUMPBOMB2RSPR, stepthink, false, push_none, 4, 0, 0, ProjectileThink, NULL, DrawReact, &s_jumpbombfly2};
FARSTATE s_jumpbombfly2 = {JUMPBOMB3LSPR, JUMPBOMB3RSPR, stepthink, false, push_none, 4, 0, 0, ProjectileThink, NULL, JumpbombReact, &s_jumpbombfly3};
FARSTATE s_jumpbombfly3 = {JUMPBOMB4LSPR, JUMPBOMB4RSPR, stepthink, false, push_none, 0, 0, 0, ProjectileThink, NULL, JumpbombReact, &s_jumpbombfly3};

/*
==================
=
= SpawnJumpbomb
=
==================
*/

void SpawnJumpbomb(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = jumpbombobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y) + -0x71;
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_jumpbomb);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 0x20;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 10;
		break;
	default:
		new->health = 5;
		break;
	}
}

/*
==================
=
= JumpbombThink
=
==================
*/

void JumpbombThink(objtype *ob)
{
	Sint16 dist;

	ob->needtoreact = true;
	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	if (ob->xdir == -1)
	{
		dist = ob->left - player->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = 1;
			return;
		}
		ob->yspeed = -32 - (US_RndT() / 8);
		ob->xspeed = -32;
		ob->state = &s_jumpbombfly1;
		return;
	}
	else
	{
		dist = player->left - ob->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = -1;
			return;
		}
		ob->yspeed = -32 - (US_RndT() / 8);
		ob->xspeed = 32;
		ob->state = &s_jumpbombfly1;
		return;
	}
}

/*
==================
=
= JumpbombReact
=
==================
*/

void JumpbombReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_grenadeexplosion1);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// RED DEVIL
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_devilwalk1 = {DEVILWALK1LSPR, DEVILWALK1RSPR, step, false, push_down, 7, 128, 0, DevilWalkThink, NULL, DevilWalkReact, &s_devilwalk2};
FARSTATE s_devilwalk2 = {DEVILWALK2LSPR, DEVILWALK2RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, DevilWalkReact, &s_devilwalk3};
FARSTATE s_devilwalk3 = {DEVILWALK3LSPR, DEVILWALK3RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, DevilWalkReact, &s_devilwalk4};
FARSTATE s_devilwalk4 = {DEVILWALK4LSPR, DEVILWALK4RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, DevilWalkReact, &s_devilwalk1};
FARSTATE s_devilburn1 = {BIGFIRE1SPR, BIGFIRE1SPR, stepthink, false, push_down, 7, 128, 0, NULL, NULL, DevilWalkReact, &s_devilburn2};
FARSTATE s_devilburn2 = {BIGFIRE2SPR, BIGFIRE2SPR, stepthink, false, push_down, 7, 128, 0, DevilBurnThink, NULL, DevilWalkReact, &s_devilburn1};
FARSTATE s_devilstand = {DEVILSTANDSPR, DEVILSTANDSPR, stepthink, false, push_down, 50, 0, 0, NULL, NULL, DrawReact, &s_devilwalk1};
FARSTATE s_deviljump  = {DEVILWALK4LSPR, DEVILWALK4RSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, DevilJumpReact, NULL};

/*
==================
=
= SpawnDevil
=
==================
*/

void SpawnDevil(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = devilobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 32);
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 20;
		break;
	default:
		new->health = 10;
		break;
	}
	NewState(new, &s_devilwalk1);
	new->hitnorth = 1;
}

/*
==================
=
= DevilWalkThink
=
==================
*/

void DevilWalkThink(objtype *ob)
{
	if (player->right < ob->left || player->left > ob->right
		|| player->top > ob->bottom || player->bottom < ob->top)
	{
		return;
	}
	
	ob->state = &s_devilburn1;
}

/*
==================
=
= DevilBurnThink
=
==================
*/

void DevilBurnThink(objtype *ob)
{
	if (US_RndT() > 240)
	{
		ChangeState(ob, &s_devilstand);
	}
}

/*
==================
=
= DevilWalkReact
=
==================
*/

void DevilWalkReact(objtype *ob)
{
	if (US_RndT() > 200)
	{
		ChangeState(ob, &s_devilburn1);
	}
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
		ob->x -= ob->xmove;
		ob->y -= ob->ymove;
		ob->yspeed = -((US_RndT()/0x20)*20);
		if (ob->yspeed < -60)
		{
			ob->yspeed = -60;
		}
		if (ob->yspeed > -30)
		{
			ob->yspeed = -30;
		}
		ob->xspeed = ob->xdir << 5;
		ChangeState(ob, &s_deviljump);
	}
	PLACESPRITE;
}

/*
==================
=
= DevilJumpReact
=
==================
*/

void DevilJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_devilwalk1);
	}
	PLACESPRITE;
}

//////////////////////////////////////////////////////////////////////////////
//
// HAIRY
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_hairywalk1 = {HAIRYWALK1LSPR, HAIRYWALK1RSPR, step, false, push_down, 7, 128, 0, HairyWalkThink, NULL, HairyReact, &s_hairywalk2};
FARSTATE s_hairywalk2 = {HAIRYWALK2LSPR, HAIRYWALK2RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, HairyReact, &s_hairywalk3};
FARSTATE s_hairywalk3 = {HAIRYWALK3LSPR, HAIRYWALK3RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, HairyReact, &s_hairywalk4};
FARSTATE s_hairywalk4 = {HAIRYWALK4LSPR, HAIRYWALK4RSPR, step, false, push_down, 7, 128, 0, NULL, NULL, HairyReact, &s_hairywalk1};
FARSTATE s_hairystand = {HAIRYSTANDLSPR, HAIRYSTANDRSPR, stepthink, false, push_down, 25, 0, 0, HairyStandThink, NULL, HairyReact, &s_hairystand};
FARSTATE s_hairylook  = {HAIRYSTANDLSPR, HAIRYSTANDRSPR, stepthink, false, push_down, 75, 0, 0, HairyLookThink, NULL, DrawReact, &s_hairylook};
FARSTATE s_hairyjump  = {HAIRYSTANDLSPR, HAIRYSTANDRSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, HairyJumpReact, NULL};

/*
==================
=
= SpawnHairy
=
==================
*/

void SpawnHairy(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = hairyobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 32);
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
	NewState(new, &s_hairywalk1);
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 6;
		break;
	default:
		new->health = 3;
		break;
	}
	new->hitnorth = 1;
}

/*
==================
=
= HairyWalkThink
=
==================
*/

void HairyWalkThink(objtype *ob)
{
	if (player->right < ob->left || player->left > ob->right
		|| player->top > ob->bottom || player->bottom < ob->top)
	{
		return;
	}
	
	ob->state = &s_hairystand;
}

/*
==================
=
= HairyStandThink
=
==================
*/

void HairyStandThink(objtype *ob)
{
	if (US_RndT() > 200)
	{
		ChangeState(ob, &s_hairywalk1);
	}
}

/*
==================
=
= HairyLookThink
=
==================
*/

void HairyLookThink(objtype *ob)
{
	if (US_RndT() > 160)
	{
		ob->xdir = 1;
	}
	else
	{
		ob->xdir = -1;
	}
	if (US_RndT() > 175)
	{
		ob->state = &s_hairywalk1;
	}
}

/*
==================
=
= HairyReact
=
==================
*/

void HairyReact(objtype *ob)
{
	if (US_RndT() > 200)
	{
		ChangeState(ob, &s_hairystand);
	}
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
		ob->x -= ob->xmove;
		ob->y -= ob->ymove;
		ob->yspeed = -((US_RndT() / 0x20)*20);
		if (ob->yspeed < -60)
		{
			ob->yspeed = -60;
		}
		if (ob->yspeed > -30)
		{
			ob->yspeed = -30;
		}
		ob->xspeed = ob->xdir << 5;
		ChangeState(ob, &s_hairyjump);
	}
	PLACESPRITE;
}

/*
==================
=
= HairyJumpReact
=
==================
*/

void HairyJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_hairywalk1);
	}
	PLACESPRITE;
}
#endif	// (EPISODE == 2) ... else ...

//////////////////////////////////////////////////////////////////////////////
//
// PUNCHY
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_punchystand   = {PUNCHYSTANDLSPR, PUNCHYSTANDRSPR, step, true, push_down, 75, 0, 0, NULL, NULL, DrawReact, &s_punchywalk1};
FARSTATE s_punchywalk1   = {PUNCHYWALK1LSPR, PUNCHYWALK1RSPR, stepthink, false, push_down, 10, 64, 0, PunchyThink, NULL, WalkReact, &s_punchywalk2};
FARSTATE s_punchywalk2   = {PUNCHYWALK2LSPR, PUNCHYWALK2RSPR, stepthink, false, push_down, 10, 64, 0, PunchyThink, NULL, WalkReact, &s_punchywalk3};
FARSTATE s_punchywalk3   = {PUNCHYWALK3LSPR, PUNCHYWALK3RSPR, stepthink, false, push_down, 10, 64, 0, PunchyThink, NULL, WalkReact, &s_punchywalk4};
FARSTATE s_punchywalk4   = {PUNCHYWALK4LSPR, PUNCHYWALK4RSPR, stepthink, false, push_down, 10, 64, 0, PunchyThink, NULL, WalkReact, &s_punchywalk1};
FARSTATE s_punchyattack1 = {PUNCHYATTACK1LSPR, PUNCHYATTACK1RSPR, step, true, push_down, 15, 0, 0, NULL, NULL, DrawReact, &s_punchyattack2};
FARSTATE s_punchyattack2 = {PUNCHYATTACK2LSPR, PUNCHYATTACK2RSPR, step, true, push_down, 10, 0, 0, NULL, NULL, DrawReact, &s_punchywalk1};

/*
==================
=
= SpawnPunchy
=
==================
*/

void SpawnPunchy(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = punchyobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_punchywalk1);
	new->hitnorth = 1;
	new->shootable = true;
#ifdef BETA
	new->health = 10;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 10;
		break;
	default:
		new->health = 5;
		break;
	}
#endif
}

/*
==================
=
= PunchyThink
=
==================
*/

void PunchyThink(objtype *ob)
{
	Sint16 dist;

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
		if (dist < -TILEGLOBAL)
		{
			ob->xdir = 1;
			return;
		}
	}
	else
	{
		dist = player->left - ob->right;
		if (dist > 4*TILEGLOBAL)
		{
			return;
		}
		if (dist < -TILEGLOBAL)
		{
			ob->xdir = -1;
			return;
		}
	}
	if (player->right < ob->left || player->left > ob->right
		|| player->top > ob->bottom || player->bottom < ob->top)
	{
		return;
	}
	ob->state = &s_punchyattack1;
}

//////////////////////////////////////////////////////////////////////////////
//
// GORP (a GARG at the size of a YORP)
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_gorpceil1 = {GORPCWALK1LSPR, GORPCWALK1RSPR, step, false, push_up, 10, 128, 0, GorpCeilThink, NULL, CeilWalkReact, &s_gorpceil2};
FARSTATE s_gorpceil2 = {GORPCWALK2LSPR, GORPCWALK2RSPR, step, false, push_up, 10, 128, 0, GorpCeilThink, NULL, CeilWalkReact, &s_gorpceil3};
FARSTATE s_gorpceil3 = {GORPCWALK3LSPR, GORPCWALK3RSPR, step, false, push_up, 10, 128, 0, GorpCeilThink, NULL, CeilWalkReact, &s_gorpceil4};
FARSTATE s_gorpceil4 = {GORPCWALK4LSPR, GORPCWALK4RSPR, step, false, push_up, 10, 128, 0, GorpCeilThink, NULL, CeilWalkReact, &s_gorpceil1};
FARSTATE s_gorpwalk1 = {GORPWALK1LSPR, GORPWALK1RSPR, step, false, push_down, 6, 128, 0, GorpWalkThink, NULL, WalkReact, &s_gorpwalk2};
FARSTATE s_gorpwalk2 = {GORPWALK2LSPR, GORPWALK2RSPR, step, false, push_down, 6, 128, 0, GorpWalkThink, NULL, WalkReact, &s_gorpwalk3};
FARSTATE s_gorpwalk3 = {GORPWALK3LSPR, GORPWALK3RSPR, step, false, push_down, 6, 128, 0, GorpWalkThink, NULL, WalkReact, &s_gorpwalk4};
FARSTATE s_gorpwalk4 = {GORPWALK4LSPR, GORPWALK4RSPR, step, false, push_down, 6, 128, 0, GorpWalkThink, NULL, WalkReact, &s_gorpwalk1};
FARSTATE s_gorpfall  = {GORPFALLLSPR, GORPFALLRSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, GorpFallReact, &s_gorpfall};

/*
==================
=
= SpawnGorp
=
==================
*/

void SpawnGorp(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = gorpobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_gorpceil1);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 0x20;
	new->shootable = true;
#ifdef BETA
	new->health = 1;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 3;
		break;
	default:
		new->health = 1;
		break;
	}
#endif
}

/*
==================
=
= GorpCeilThink
=
==================
*/

void GorpCeilThink(objtype *ob)
{
	if (ob->xdir == -1)
	{
		if (player->right < ob->left || player->left > ob->right || player->bottom < ob->top)
		{
			return;
		}
			
		if (!ob->hitnorth)
		{
			ob->yspeed = 24 - (US_RndT() / 0x10);
			SD_PlaySound(GORPATTACKSND);
			ob->state = &s_gorpfall;
			return;
		}
	}
	else
	{
		if (player->right < ob->left || player->left > ob->right || player->bottom < ob->top)
		{
			return;
		}
			
		if (!ob->hitnorth)
		{
			ob->yspeed = 24 - (US_RndT() / 0x10);
			SD_PlaySound(GORPATTACKSND);
			ob->state = &s_gorpfall;
			return;
		}
	}
}

/*
==================
=
= GorpWalkThink
=
==================
*/

void GorpWalkThink(objtype *ob)
{
	if (ob->xdir == -1)
	{
		if (!ob->hitnorth)
		{
			ob->yspeed = 24 - (US_RndT() / 0x10);
			SD_PlaySound(GORPATTACKSND);
			ob->state = &s_gorpfall;
			return;
		}
	}
	else
	{
		if (!ob->hitnorth)
		{
			ob->yspeed = 24 - (US_RndT() / 0x10);
			SD_PlaySound(GORPATTACKSND);
			ob->state = &s_gorpfall;
			return;
		}
	}
}

/*
==================
=
= GorpFallReact
=
==================
*/

void GorpFallReact(objtype *ob)
{
	if (ob->hitnorth)
	{
		SD_PlaySound(GORPSPLATSND);
		ChangeState(ob, &s_gorpwalk1);
		ob->nothink = 12;
	}
	PLACESPRITE;
}

#if (EPISODE != 2)
//////////////////////////////////////////////////////////////////////////////
//
// SLIMER
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_slimerstand  = {SLIMERWALK1LSPR, SLIMERWALK1RSPR, step, true, push_down, 25, 0, 0, NULL, NULL, DrawReact, &s_slimerwalk1};
FARSTATE s_slimerwalk1  = {SLIMERWALK1LSPR, SLIMERWALK1RSPR, stepthink, false, push_down, 10, 64, 0, SlimerThink, NULL, WalkReact, &s_slimerwalk2};
FARSTATE s_slimerwalk2  = {SLIMERWALK2LSPR, SLIMERWALK2RSPR, stepthink, false, push_down, 10, 64, 0, SlimerThink, NULL, WalkReact, &s_slimerwalk1};
FARSTATE s_slimerattack = {SLIMERATTACKSPR, SLIMERATTACKSPR, step, true, push_down, 15, 0, 0, 0, 0, DrawReact, &s_slimerstand};

/*
==================
=
= SpawnSlimer
=
==================
*/

void SpawnSlimer(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = slimerobj;
	new->priority = 2;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_slimerwalk1);
	new->hitnorth = 1;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 3;
		break;
	default:
		new->health = 1;
		break;
	}
}

/*
==================
=
= SlimerThink
=
==================
*/

void SlimerThink(objtype *ob)
{
	if (US_RndT() > 250)
	{
		ob->state = &s_slimerstand;
	}
	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	if (player->right < ob->left || player->left > ob->right
		|| player->top > ob->bottom || player->bottom < ob->top)
	{
		return;
	}
	ob->state = &s_slimerattack;
}
#endif	// (EPISODE != 2)

#if (EPISODE == 3)
//////////////////////////////////////////////////////////////////////////////
//
// CYBORG
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_cyborgwalk1 = {CYBORG1LSPR, CYBORG1RSPR, step, false, push_down, 7, 128, 0, CyborgWalkThink, NULL, WalkReact, &s_cyborgwalk2};
FARSTATE s_cyborgwalk2 = {CYBORG2LSPR, CYBORG2RSPR, step, false, push_down, 7, 128, 0, CyborgWalkThink, NULL, WalkReact, &s_cyborgwalk3};
FARSTATE s_cyborgwalk3 = {CYBORG3LSPR, CYBORG3RSPR, step, false, push_down, 7, 128, 0, CyborgWalkThink, NULL, WalkReact, &s_cyborgwalk4};
FARSTATE s_cyborgwalk4 = {CYBORG4LSPR, CYBORG4RSPR, step, false, push_down, 7, 128, 0, CyborgWalkThink, NULL, WalkReact, &s_cyborgwalk1};

/*
==================
=
= SpawnCyborg
=
==================
*/

void SpawnCyborg(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = cyborgobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 15;
		break;
	default:
		new->health = 10;
		break;
	}
	NewState(new, &s_cyborgwalk1);
	new->hitnorth = 1;
}

/*
==================
=
= CyborgWalkThink
=
==================
*/

void CyborgWalkThink(objtype *ob)
{
	Uint16 shotx;

	if (US_RndT() > 230)
	{
		if (ob->xdir == 1)
		{
			shotx = ob->x + 2*TILEGLOBAL;
		}
		else
		{
			shotx = ob->x;
		}
		if (SpawnEnemyShot(shotx, ob->y + TILEGLOBAL, &s_cyborgshot) == -1)
		{
			return;
		}
		new->xspeed = ob->xdir * 60;
		new->yspeed = 0;
		SD_PlaySound(SNAKESHOOTSND);
	}
}
#endif	// (EPISODE == 3)
