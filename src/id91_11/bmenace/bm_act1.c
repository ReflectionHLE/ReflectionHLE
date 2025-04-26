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

Sint16 pdirx[] = { 0, 1, 0, -1,  1, 1, -1, -1};
Sint16 pdiry[] = {-1, 0, 1,  0, -1, 1,  1, -1};

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
==================
=
= ProjectileReact		(based on ProjectileReact() from KEEN DREAMS)
=
==================
*/

void ProjectileReact(objtype *ob)
{
	Uint16 wall, absx, absy, angle, newangle;
	Uint32 speed;
	
	PLACESPRITE;
	if (ob->hiteast || ob->hitwest)
	{
		ob->xspeed = -ob->xspeed/2;
	}
	if (ob->hitsouth)
	{
		ob->yspeed = -ob->yspeed/2;
	}
	wall = ob->hitnorth;
	if (wall)
	{
		if (ob->yspeed < 0)
		{
			ob->yspeed = 0;
		}
		absx = abs(ob->xspeed);
		absy = ob->yspeed;
		if (absx>absy)
		{
			if (absx>absy*2)	// 22 degrees
			{
				angle = 0;
				speed = absx*286;	// x*sqrt(5)/2
			}
			else				// 45 degrees
			{
				angle = 1;
				speed = absx*362;	// x*sqrt(2)
			}
		}
		else
		{
			if (absy>absx*2)	// 90 degrees
			{
				angle = 3;
				speed = absy*256;
			}
			else
			{
				angle = 2;		// 67 degrees
				speed = absy*286;	// y*sqrt(5)/2
			}
		}
		if (ob->xspeed > 0)
			angle = 7-angle;

		speed >>= 1;
		newangle = bounceangle[ob->hitnorth][angle];	//I think this should use (ob->hitnorth&SLOPEMASK) just to be safe.
		switch (newangle)
		{
		case 0:
			ob->xspeed = speed / 286;
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 1:
			ob->xspeed = speed / 362;
			ob->yspeed = -ob->xspeed;
			break;
		case 2:
			ob->yspeed = -(speed / 286);
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 3:

		case 4:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 5:
			ob->yspeed = -(speed / 286);
			ob->xspeed = ob->yspeed / 2;
			break;
		case 6:
			ob->xspeed = ob->yspeed = -(speed / 362);
			break;
		case 7:
			ob->xspeed = -(speed / 286);
			ob->yspeed = ob->xspeed / 2;
			break;

		case 8:
			ob->xspeed = -(speed / 286);
			ob->yspeed = -ob->xspeed / 2;
			break;
		case 9:
			ob->xspeed = -(speed / 362);
			ob->yspeed = -ob->xspeed;
			break;
		case 10:
			ob->yspeed = speed / 286;
			ob->xspeed = -ob->yspeed / 2;
			break;
		case 11:

		case 12:
			ob->xspeed = 0;
			ob->yspeed = -(speed / 256);
			break;
		case 13:
			ob->yspeed = speed / 286;
			ob->xspeed = ob->yspeed / 2;
			break;
		case 14:
			ob->xspeed = speed / 362;
			ob->yspeed = speed / 362;
			break;
		case 15:
			ob->xspeed = speed / 286;
			ob->yspeed = ob->xspeed / 2;
			break;
		}

		if (speed < 256*16)
		{
			RemoveObj(ob);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// REGULAR PLATFORM
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_platform = {PLATFORMSPR, PLATFORMSPR, think, false, push_none, 0, 0, 0, PlatformThink, NULL, DrawReact, NULL};

/*
==================
=
= SpawnPlatform
=
==================
*/

void SpawnPlatform(Uint16 x, Uint16 y, arrowdirtype dir)
{
	GetNewObj(false);
	new->obclass = platformobj;
	new->active = allways;
#if (EPISODE == 1)
	new->priority = 1;
#else
	new->priority = 0;
#endif
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	switch (dir)
	{
	case arrow_North:
		new->xdir = 0;
		new->ydir = -1;
		break;
	case arrow_East:
		new->xdir = 1;
		new->ydir = 0;
		break;
	case arrow_South:
		new->xdir = 0;
		new->ydir = 1;
		break;
	case arrow_West:
		new->xdir = -1;
		new->ydir = 0;
		break;
	}
	NewState(new, &s_platform);
}

#ifndef BETA
//////////////////////////////////////////////////////////////////////////////
//
// APOGEE PLATFORM
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_apogee = {APOGEESPR, APOGEESPR, think, false, push_none, 0, 0, 0, PlatformThink, NULL, DrawReact, NULL};

/*
==================
=
= SpawnApogee
=
==================
*/

void SpawnApogee(Uint16 x, Uint16 y, arrowdirtype dir)
{
	GetNewObj(false);
	new->obclass = platformobj;
	new->active = allways;
	new->priority = 0;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	switch (dir)
	{
	case arrow_North:
		new->xdir = 0;
		new->ydir = -1;
		break;
	case arrow_East:
		new->xdir = 1;
		new->ydir = 0;
		break;
	case arrow_South:
		new->xdir = 0;
		new->ydir = 1;
		break;
	case arrow_West:
		new->xdir = -1;
		new->ydir = 0;
		break;
	}
	NewState(new, &s_apogee);
}
#endif

/*
==================
=
= PlatformThink
=
==================
*/

void PlatformThink(objtype *ob)
{
	Uint16 newy, newtiley;
	
	if (xtry || ytry)
	{
		return;
	}
	xtry = ob->xdir*12*tics;
	ytry = ob->ydir*12*tics;
	if (ob->ydir == 1)
	{
		newy = ob->bottom + ytry;
		newtiley = CONVERT_GLOBAL_TO_TILE(newy);
		if (ob->tilebottom != newtiley && INFOSPOT(ob->tileleft,newtiley) == STOPICON)
		{
			ytry = 0;
			ob->needtoreact = true;
			return;
		}
		if (ob->tilebottom != newtiley && INFOSPOT(ob->tileleft,newtiley) == BUMPICON)
		{
			if (INFOSPOT(ob->tileleft,newtiley-2) == BUMPICON)
			{
				ytry = 0;
				ob->needtoreact = true;
				return;
			}
			else
			{
				ob->ydir = -1;
				ytry = ytry - (newy & 0xFF);
			}
		}
	}
	else if (ob->ydir == -1)
	{
		newy = ob->top + ytry;
		newtiley = CONVERT_GLOBAL_TO_TILE(newy);
		if (ob->tiletop != newtiley && INFOSPOT(ob->tileleft,newtiley) == STOPICON)
		{
			ytry = 0;
			ob->needtoreact = true;
			return;
		}
		if (ob->tiletop != newtiley && INFOSPOT(ob->tileleft,newtiley) == BUMPICON)
		{
			if (INFOSPOT(ob->tileleft,newtiley+2) == BUMPICON)
			{
				ytry = 0;
				ob->needtoreact = true;
				return;
			}
			else
			{
				ob->ydir = 1;
				ytry = ytry + (TILEGLOBAL - (newy & 0xFF));
			}
		}
	}
}

#if (EPISODE != 1)
//////////////////////////////////////////////////////////////////////////////
//
// SHUTTLE PLATFORM
//
// temp1: direction
// temp2: movement remaining until next direction tile check
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_shuttle = {SHUTTLESPR, SHUTTLESPR, think, false, push_none, 0, 0, 0, GoplatThink, NULL, DrawReact, NULL};

/*
==================
=
= SpawnShuttle
=
==================
*/

void SpawnShuttle(Uint16 x, Uint16 y, arrowdirtype dir)
{
	GetNewObj(false);
	new->obclass = shuttleobj;
	new->active = allways;
	new->priority = 0;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 0;
	new->ydir = 1;
	new->needtoclip = cl_noclip;
	NewState(new, &s_shuttle);
	INFOSPOT(x,y) = DIRARROWSTART+dir;
	new->temp1 = dir;
	new->temp2 = TILEGLOBAL;
}

#endif

#if (EPISODE == 2)
//////////////////////////////////////////////////////////////////////////////
//
// QUEEN ANT
//
// temp1: direction
// temp2: movement remaining until next direction tile check
//
// temp7: countdown for spawning next Nymph
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_queen1 = {QUEEN1SPR, QUEEN1SPR, stepthink, false, push_none, 1, 0, 0, GoplatThink, NULL, DrawReact, &s_queen2};
FARSTATE s_queen2 = {QUEEN2SPR, QUEEN2SPR, stepthink, false, push_none, 1, 0, 0, GoplatThink, NULL, QueenReact, &s_queen1};

/*
==================
=
= SpawnQueen
=
==================
*/

void SpawnQueen(Uint16 x, Uint16 y, arrowdirtype dir)
{
	GetNewObj(false);
	new->obclass = queenobj;
	new->active = allways;
	new->priority = 3;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 0;
	new->ydir = 1;
	new->needtoclip = cl_noclip;
	INFOSPOT(x,y) = DIRARROWSTART+dir;
	new->temp1 = dir;
	new->temp2 = TILEGLOBAL;
	new->temp7 = 20;
	new->shootable = true;
#ifdef BETA
	new->health = 50;
#else
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 100;
		break;
	default:
		new->health = 50;
		break;
	}
	bosshealth = oldhealthbarlength = new->health;
	bossdiv = bosshealth / 20;
#endif
	NewState(new, &s_queen1);
}

/*
==================
=
= QueenReact
=
==================
*/

void QueenReact(objtype *ob)
{
#ifndef BETA
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
#endif
	if (--ob->temp7 <= 0)
	{
		ob->temp7 = 20;
		if (OnScreen(ob))
			SpawnNymph(ob);
	}
	PLACESPRITE;
}
#endif	// (EPISODE == 2)

#if (EPISODE == 1)
//////////////////////////////////////////////////////////////////////////////
//
// HELICOPTER & PARABOT (a.k.a. "parachuting robot")
//
// temp1: direction
// temp2: movement remaining until next direction tile check
//
// temp6: animation countdown
// temp7: countdown for spawning next ParaBot (HELICOPTER)
//        turn cooldown for WalkReact (PARABOT)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_helicopter1 = {HELICOPTER2SPR, HELICOPTER2SPR, stepthink, false, push_none, 1, 0, 0, GoplatThink, NULL, HelicopterReact, &s_helicopter1};
FARSTATE s_helicopter2 = {HELICOPTER1SPR, HELICOPTER1SPR, stepthink, false, push_none, 10, 0, 0, GoplatThink, NULL, HelicopterReact, &s_helicopter1};
FARSTATE s_helicopter3 = {HELICOPTER1SPR, HELICOPTER1SPR, stepthink, false, push_none, 1, 0, 0, GoplatThink, NULL, HelicopterSpawnParabot, &s_helicopter1}; // never used!
FARSTATE s_parabotwalk1 = {PARABOTWALK1LSPR, PARABOTWALK1RSPR, step, false, push_down, 10, 128, 0, ParabotWalkThink, NULL, WalkReact, &s_parabotwalk2};
FARSTATE s_parabotwalk2 = {PARABOTWALK2LSPR, PARABOTWALK2RSPR, step, false, push_down, 10, 128, 0, ParabotWalkThink, NULL, WalkReact, &s_parabotwalk1};
FARSTATE s_parabotjump  = {PARABOTWALK2LSPR, PARABOTWALK2RSPR, think, false, push_none, 0, 0, 0, ProjectileThink, ParabotContact, ParabotJumpReact, &s_parabotjump};
FARSTATE s_parabotfall  = {PARABOTFALLLSPR, PARABOTFALLRSPR, stepthink, false, push_none, 10, 8, 32, 0, ParabotContact, ParabotFallReact, &s_parabotfall};

/*
==================
=
= SpawnHelicopter
=
==================
*/

void SpawnHelicopter(Uint16 x, Uint16 y, arrowdirtype dir)
{
	GetNewObj(false);
	new->obclass = helicopterobj;
	new->active = allways;
	new->priority = 3;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 0;
	new->ydir = 1;
	new->needtoclip = cl_noclip;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 50;
		break;
	default:
		new->health = 25;
		break;
	}
	new->temp6 = 15;
	new->temp7 = 40;
	NewState(new, &s_helicopter1);
	INFOSPOT(x,y) = dir+DIRARROWSTART;
	new->temp1 = dir;
	new->temp2 = TILEGLOBAL;
}

// implementation follows after GoplatThink()

#else		// (EPISODE != 1) now!
//////////////////////////////////////////////////////////////////////////////
//
// CATERPILLAR
//
// temp1: direction
// temp2: movement remaining until next direction tile check
//
//////////////////////////////////////////////////////////////////////////////

#if (EPISODE == 2)
FARSTATE s_caterpillarhead1 = {CATERPILLARHEAD1SPR, CATERPILLARHEAD1SPR, stepthink, false, push_none, 25, 0, 0, GoplatThink, NULL, DrawReact, &s_caterpillarhead2};
FARSTATE s_caterpillarhead2 = {CATERPILLARHEAD2SPR, CATERPILLARHEAD2SPR, stepthink, false, push_none, 25, 0, 0, GoplatThink, NULL, DrawReact, &s_caterpillarhead1};
#else
FARSTATE s_caterpillarhead1 = {CATERPILLARHEAD1SPR, CATERPILLARHEAD1SPR, stepthink, false, push_none, 10, 0, 0, GoplatThink, NULL, DrawReact, &s_caterpillarhead2};
FARSTATE s_caterpillarhead2 = {CATERPILLARHEAD2SPR, CATERPILLARHEAD2SPR, stepthink, false, push_none, 10, 0, 0, GoplatThink, NULL, DrawReact, &s_caterpillarhead1};
#endif
FARSTATE s_caterpillarbody  = {CATERPILLARBODYSPR, CATERPILLARBODYSPR, think, false, push_none, 0, 0, 0, GoplatThink, NULL, DrawReact, NULL};

/*
==================
=
= SpawnCaterpillar
=
==================
*/

void SpawnCaterpillar(Uint16 x, Uint16 y, arrowdirtype dir, Uint16 kind)
{
	GetNewObj(false);
	new->obclass = caterpillarobj;
	new->active = allways;
	new->priority = 3;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = CONVERT_TILE_TO_GLOBAL(y);
	new->xdir = 0;
	new->ydir = 1;
	new->needtoclip = cl_noclip;
	INFOSPOT(x,y) = DIRARROWSTART+dir;
	new->temp1 = dir;
	new->temp2 = TILEGLOBAL;
	new->shootable = true;
	if (kind == 0)
	{
#ifdef BETA
		new->health = 25;
#else
		switch (gamestate.difficulty)
		{
		case gd_Hard:
			new->health = 50;
			break;
		default:
			new->health = 25;
			break;
		}
#endif
		NewState(new, &s_caterpillarhead1);
	}
	else
	{
#ifdef BETA
		new->health = 5;
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
		NewState(new, &s_caterpillarbody);
	}
}
#endif	// (EPISODE == 1) ... else ...

/*
==================
=
= GoplatThink
=
==================
*/

void GoplatThink(objtype *ob)
{
	Uint16 amount, tx, ty;
	Sint16 delta;
	
	if (xtry || ytry)
	{
		return;
	}
#if (EPISODE == 1)
	amount = tics * 24;
#elif (EPISODE == 2)
	if (ob->obclass != shuttleobj)
	{
		amount = tics * 24;
	}
	else
	{
		amount = tics * 24;
	}
#elif (EPISODE == 3)
	if (ob->obclass == caterpillarobj)
	{
		amount = tics * 12;
	}
	else
	{
		amount = tics * 24;
	}
#endif
	if (ob->temp2 > amount)
	{
		ob->temp2 = ob->temp2 - amount;
		delta = pdirx[ob->temp1];
		if (delta == 1)
		{
			xtry = xtry + amount;
		}
		else if (delta == -1)
		{
			xtry = xtry + -amount;
		}
		delta = pdiry[ob->temp1];
		if (delta == 1)
		{
			ytry = ytry + amount;
		}
		else if (delta == -1)
		{
			ytry = ytry + -amount;
		}
		return;
	}
	delta = pdirx[ob->temp1];
	if (delta == 1)
	{
		xtry += ob->temp2;
	}
	else if (delta == -1)
	{
		xtry += -ob->temp2;
	}
	delta = pdiry[ob->temp1];
	if (delta == 1)
	{
		ytry += ob->temp2;
	}
	else if (delta == -1)
	{
		ytry += -ob->temp2;
	}
	tx = CONVERT_GLOBAL_TO_TILE(ob->x + xtry);
	ty = CONVERT_GLOBAL_TO_TILE(ob->y + ytry);
	ob->temp1 = INFOSPOT(tx,ty) - DIRARROWSTART;
	if (ob->temp1 < arrow_North || ob->temp1 > arrow_None)
	{
		id0_char_t errmsg[60] = "Goplat moved to a bad spot: ";
		id0_char_t buffer[5] = "";

		strcat(errmsg, BE_Cross_itoa_dec(ob->x, buffer));
		strcat(errmsg, ",");
		strcat(errmsg, BE_Cross_itoa_dec(ob->y, buffer));
		Quit(errmsg);
	}
	amount -= ob->temp2;
	ob->temp2 = TILEGLOBAL - amount;
	delta = pdirx[ob->temp1];
	if (delta == 1)
	{
		xtry = xtry + amount;
	}
	else if (delta == -1)
	{
		xtry = xtry - amount;
	}
	delta = pdiry[ob->temp1];
	if (delta == 1)
	{
		ytry = ytry + amount;
	}
	else if (delta == -1)
	{
		ytry = ytry - amount;
	}
}

#if (EPISODE == 1)
//////////////////////////////////////////////////////////////////////////////
//
// HELICOPTER & PARABOT (a.k.a. "parachuting robot")         (continued...)
//
// temp5: explosion countdown (ParaBot)
//
//////////////////////////////////////////////////////////////////////////////

/*
==================
=
= HelicopterReact
=
==================
*/

void HelicopterReact(objtype *ob)
{
	if (--ob->temp6 <= 0)
	{
		ob->state = &s_helicopter2;
		ob->temp6 = 15;
	}
	if (--ob->temp7 <= 0)
	{
		ob->temp7 = 40;
		if (OnScreen(ob))
		{
			HelicopterSpawnParabot(ob);
		}
	}
	PLACESPRITE;
}

/*
==================
=
= HelicopterSpawnParabot
=
==================
*/

void HelicopterSpawnParabot(objtype *ob)
{
	SD_PlaySound(RICOCHET1SND);
	GetNewObj(true);
	new->obclass = parabotobj;
	new->priority = 2;
	new->active = allways;
	new->x = ob->x + 24*PIXGLOBAL;
	new->y = ob->y + 8*PIXGLOBAL;
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
	new->temp5 = 75;
	NewState(new, &s_parabotfall);
}

/*
==================
=
= ParabotWalkThink
=
==================
*/

void ParabotWalkThink(objtype *ob)
{
	Sint16 dist;

	if (--ob->temp5 <= 0)
	{
		ob->shootable = false;
		ChangeState(ob, &s_grenadeexplosion1);
		return;
	}
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
		ob->state = &s_parabotjump;
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
		ob->state = &s_parabotjump;
		return;
	}
}

/*
==================
=
= ParabotContact
=
==================
*/

void ParabotContact(objtype *ob, objtype *hit)
{
	if (hit->obclass == playerobj)
	{
		ob->shootable = false;
		ChangeState(ob, &s_grenadeexplosion1);
	}
}

/*
==================
=
= ParabotFallReact
=
==================
*/

void ParabotFallReact(objtype *ob)
{
	if (!ob->hitnorth)
	{
		if (US_RndT() < 100)
		{
			ob->xdir = -ob->xdir;
		}
	}
	else
	{
		ChangeState(ob, &s_parabotwalk1);
		ob->y += TILEGLOBAL;
	}
	PLACESPRITE;
}

/*
==================
=
= ParabotJumpReact
=
==================
*/

void ParabotJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_grenadeexplosion1);
		ob->shootable = false;
		ob->nothink = 12;
	}
	PLACESPRITE;
}
#endif	// (EPISODE == 1)

//////////////////////////////////////////////////////////////////////////////
//
// COBRA
// (basically a carbon copy of FRENCHY from KEEN DREAMS)
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_cobrawalk1   = {COBRAWALK1LSPR, COBRAWALK1RSPR, step, false, push_down, 10, 128, 0, CobraWalkThink, NULL, WalkReact, &s_cobrawalk2};
FARSTATE s_cobrawalk2   = {COBRAWALK2LSPR, COBRAWALK2RSPR, step, false, push_down, 10, 128, 0, CobraWalkThink, NULL, WalkReact, &s_cobrawalk1};
FARSTATE s_cobrarun1    = {COBRAWALK1LSPR, COBRAWALK1RSPR, step, true, push_down, 5, 128, 0, CobraRunThink, NULL, WalkReact, &s_cobrarun2};
FARSTATE s_cobrarun2    = {COBRAWALK2LSPR, COBRAWALK2RSPR, step, true, push_down, 5, 128, 0, CobraRunThink, NULL, WalkReact, &s_cobrarun1};
FARSTATE s_cobraattack1 = {COBRAATTACKLSPR, COBRAATTACKRSPR, step, false, push_none, 10, 0, 0, CobraAttackThink, NULL, DrawReact, &s_cobraattack2};
FARSTATE s_cobraattack2 = {COBRAATTACKLSPR, COBRAATTACKRSPR, step, false, push_none, 10, -128, 0, NULL, NULL, DrawReact, &s_cobrawalk1};
FARSTATE s_cobraspit1   = {COBRASPIT1LSPR, COBRASPIT1RSPR, stepthink, false, push_none, 4, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_cobraspit2};
FARSTATE s_cobraspit2   = {COBRASPIT2LSPR, COBRASPIT2RSPR, stepthink, false, push_none, 4, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_cobraspit1};

/*
==================
=
= SpawnCobra
=
==================
*/

void SpawnCobra(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = cobraobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 24);
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
	NewState(new, &s_cobrawalk1);
}

/*
==================
=
= CobraRunThink
=
==================
*/

void CobraRunThink(objtype *ob)
{
	ob->state = &s_cobrawalk1;
}

/*
==================
=
= CobraAttackThink
=
==================
*/

void CobraAttackThink(objtype *ob)
{
	GetNewObj(true);
	new->obclass = enemyshotobj;
	if (ob->xdir == 1)
	{
		new->x = ob->x + 24*PIXGLOBAL;
		new->y = ob->y + 8*PIXGLOBAL;
	}
	else
	{
		new->x = ob->x;
		new->y = ob->y + 8*PIXGLOBAL;
	}
	new->xdir = ob->xdir;
	new->ydir = 1;
	new->xspeed = (ob->xdir * 40) - (US_RndT() >> 4);
	new->yspeed = -20;
	new->active = removable;
	NewState(new, &s_cobraspit1);
	ob->nothink = 2;
}

/*
==================
=
= CobraWalkThink
=
==================
*/

void CobraWalkThink(objtype *ob)
{
	Sint16 dist;
	if (abs(ob->y-player->y) > 3*TILEGLOBAL)
	{
		if (US_RndT() < 8)
		{
			ob->xdir = -ob->xdir;		// turn randomly
		}
		return;
	}
	dist = player->x-ob->x;
	if (dist < -8*TILEGLOBAL)
	{
	// walk closer
		ob->xdir = -1;
	}
	if (dist < -4*TILEGLOBAL)	//BUG? this is NOT 'else if', which makes the 'if' check above rather useless
	{
	// throw
		SD_PlaySound(COBRASHOOTSND);
		ob->xdir = -1;
		ob->state = &s_cobraattack1;
	}
	else if (dist < 0)
	{
	// run away
		ob->xdir = 1;
		ob->state = &s_cobrarun1;
		ob->nothink = 8;
	}
	else if (dist < 4*TILEGLOBAL)
	{
	// run away
		ob->xdir = -1;
		ob->state = &s_cobrarun1;
		ob->nothink = 8;
	}
	else if (dist < 8*TILEGLOBAL)
	{
	// throw and walk closer
		SD_PlaySound(COBRASHOOTSND);
		ob->xdir = 1;
		ob->state = &s_cobraattack1;
	}
	else
	{
	// walk closer
		ob->xdir = 1;
	}
}

#if (EPISODE == 1)
//////////////////////////////////////////////////////////////////////////////
//
// SEWERMAN
// (basically a simplified version of FRENCHY from KEEN DREAMS)
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_sewermanwalk1  = {SEWERMANWALK1LSPR, SEWERMANWALK1RSPR, step, false, push_down, 20, 128, 0, SewermanWalkThink, NULL, WalkReact, &s_sewermanwalk2};
FARSTATE s_sewermanwalk2  = {SEWERMANWALK2LSPR, SEWERMANWALK2RSPR, step, false, push_down, 20, 128, 0, SewermanWalkThink, NULL, WalkReact, &s_sewermanwalk1};
FARSTATE s_sewermanattack = {SEWERMANATTACKLSPR, SEWERMANATTACKRSPR, step, false, push_down, 10, 0, 0, SewermanAttackThink, NULL, WalkReact, &s_sewermanwalk1};
FARSTATE s_sewermanspit1  = {SEWERMANSPIT1SPR, SEWERMANSPIT1SPR, stepthink, false, push_none, 4, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_sewermanspit2};
FARSTATE s_sewermanspit2  = {SEWERMANSPIT2SPR, SEWERMANSPIT2SPR, stepthink, false, push_none, 4, 0, 0, ProjectileThink, NULL, ProjectileReact, &s_sewermanspit1};

/*
==================
=
= SpawnSewerman
=
==================
*/

void SpawnSewerman(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = sewermanobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 48);
	new->xdir = 1;
	new->ydir = 1;
	new->shootable = true;
	switch (gamestate.difficulty)
	{
	case gd_Hard:
		new->health = 30;
		break;
	default:
		new->health = 15;
		break;
	}
	NewState(new, &s_sewermanwalk1);
}

/*
==================
=
= SewermanAttackThink
=
==================
*/

void SewermanAttackThink(objtype *ob)
{
	GetNewObj(true);
	new->obclass = enemyshotobj;
	if (ob->xdir == 1)
	{
		new->x = ob->x + 24*PIXGLOBAL;
		new->y = ob->y + 8*PIXGLOBAL;
	}
	else
	{
		new->x = ob->x;
		new->y = ob->y + 8*PIXGLOBAL;
	}
	new->xdir = ob->xdir;
	new->ydir = 1;
	new->xspeed = (ob->xdir * 30)-(US_RndT() >> 4);
	new->yspeed = -20;
	new->active = removable;
	NewState(new, &s_sewermanspit1);
	ob->nothink = 2;
}

/*
==================
=
= SewermanWalkThink
=
==================
*/

void SewermanWalkThink(objtype *ob)
{
	Sint16 dist;

	if (abs(ob->y - player->y) > 3*TILEGLOBAL)
	{
		if (US_RndT() < 8)
		{
			ob->xdir = -ob->xdir;		// turn randomly
		}
		return;
	}
	dist = player->x - ob->x;
	if (dist < -8*TILEGLOBAL)
	{
	// walk closer
		ob->xdir = -1;
	}
	if (dist < -4*TILEGLOBAL)	//BUG? this is NOT 'else if', which makes the 'if' check above rather useless
	{
	// throw
		SD_PlaySound(SEWERMANATTACKSND);
		ob->xdir = -1;
		ob->state = &s_sewermanattack;
	}
	else if (dist < 0)
	{
	// run away
		ob->xdir = 1;
		ob->state = &s_sewermanwalk1;
		ob->nothink = 8;
	}
	else if (dist < 4*TILEGLOBAL)
	{
	// run away
		ob->xdir = -1;
		ob->state = &s_sewermanwalk1;
		ob->nothink = 8;
	}
	else if (dist < 8*TILEGLOBAL)
	{
	// throw and walk closer
		SD_PlaySound(SEWERMANATTACKSND);
		ob->xdir = 1;
		ob->state = &s_sewermanattack;
	}
	else
	{
	// walk closer
		ob->xdir = 1;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// CIVILIAN HOSTAGES (EPISODE 1 ONLY)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_hostage1stand1 = {HOSTAGE11SPR, HOSTAGE11SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage1stand2};
FARSTATE s_hostage1stand2 = {HOSTAGE12SPR, HOSTAGE12SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage1stand1};
FARSTATE s_hostage2stand1 = {HOSTAGE21SPR, HOSTAGE21SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage2stand2};
FARSTATE s_hostage2stand2 = {HOSTAGE22SPR, HOSTAGE22SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage2stand1};
FARSTATE s_hostage3stand1 = {HOSTAGE31SPR, HOSTAGE31SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage3stand2};
FARSTATE s_hostage3stand2 = {HOSTAGE32SPR, HOSTAGE32SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage3stand1};

/*
==================
=
= SpawnHostage
=
==================
*/

void SpawnHostage(Uint16 x, Uint16 y, Uint16 type)
{
	GetNewObj(false);
	new->obclass = hostageobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	switch (type)
	{
	case 0:
		NewState(new, &s_hostage1stand1);
		break;
	case 1:
		NewState(new, &s_hostage2stand1);
		break;
	case 2:
		NewState(new, &s_hostage3stand1);
		break;
	}
	new->hitnorth = 1;
	new->shootable = false;
}
#endif	// (EPISODE == 1)

#if (EPISODE == 2)
//////////////////////////////////////////////////////////////////////////////
//
// JIM, GEORGE & KEEN (EPISODE 2 ONLY)
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_jimstand     = {JIM1LSPR, JIM1RSPR, step, false, push_down, 1, 0, 0, JimThink, NULL, DrawReact, &s_jimstand};
FARSTATE s_jimattack1   = {JIM2LSPR, JIM2RSPR, step, false, push_down, 8, 0, 0, JimThink, NULL, DrawReact, &s_jimattack2};
FARSTATE s_jimattack2   = {JIM2LSPR, JIM2RSPR, step, false, push_down, 1, 0, 0, JimAttackThink, NULL, DrawReact, &s_jimattack1};
FARSTATE s_georgestand1 = {GEORGE1LSPR, GEORGE1RSPR, step, false, push_down, 20, 0, 0, JimThink, NULL, DrawReact, &s_georgestand2};
FARSTATE s_georgestand2 = {GEORGE2LSPR, GEORGE2RSPR, step, false, push_down, 20, 0, 0, JimThink, NULL, DrawReact, &s_georgestand1};
FARSTATE s_keenstand1   = {KEEN1SPR, KEEN1SPR, step, false, push_down, 25, 0, 0, NULL, NULL, DrawReact, &s_keenstand2};
FARSTATE s_keenstand2   = {KEEN2SPR, KEEN2SPR, step, false, push_down, 25, 0, 0, NULL, NULL, DrawReact, &s_keenstand1};

/*
==================
=
= SpawnJim
=
==================
*/

void SpawnJim(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = jimobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	new->xdir = -1;
	new->hitnorth = 1;
	new->shootable = true;
	new->active = removable;
	NewState(new, &s_jimstand);
}

/*
==================
=
= SpawnGeorge
=
==================
*/

void SpawnGeorge(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = decoobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 48);
	new->xdir = -1;
	NewState(new, &s_georgestand1);
	new->hitnorth = 1;
}

/*
==================
=
= SpawnKeen
=
==================
*/

void SpawnKeen(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = hostageobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 32);
	new->xdir = -1;
	NewState(new, &s_keenstand1);
	new->hitnorth = 1;
}

/*
==================
=
= JimThink
=
==================
*/

void JimThink(objtype *ob)
{
	ob->needtoreact = true;
	if (player->right < ob->left)
		ob->xdir = -1;
	if (player->left > ob->right)
		ob->xdir = 1;
}

/*
==================
=
= JimAttackThink
=
==================
*/

void JimAttackThink(objtype *ob)
{
	Uint16 shotx;

	ob->needtoreact = true;
	if (player->right < ob->left)
		ob->xdir = -1;
	if (player->left > ob->right)
		ob->xdir = 1;
	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	if (ob->xdir == 1)
	{
		shotx = ob->x + 24*PIXGLOBAL;
	}
	else
	{
		shotx = ob->x - 8*PIXGLOBAL;
	}
	if (SpawnEnemyShot(shotx, ob->y+15*PIXGLOBAL, &s_enemyshot) == -1)
	{
		return;
	}
	new->xspeed = ob->xdir * 60;
	new->yspeed = 0;
	SD_PlaySound(SNAKESHOOTSND);
}
#endif	// (EPISODE == 2)

#if (EPISODE != 1)
//////////////////////////////////////////////////////////////////////////////
//
// SCIENTIST HOSTAGES (EPISODES 2 & 3)
//
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_hostage1stand  = {HOSTAGE12LSPR, HOSTAGE12RSPR, step, false, push_down, 10, 128, 0, Hostage1Think, NULL, WalkReact, &s_hostage1stand};
FARSTATE s_hostage1walk   = {HOSTAGE11LSPR, HOSTAGE11RSPR, step, false, push_down, 10, 128, 0, NULL, NULL, WalkReact, &s_hostage1stand};
FARSTATE s_hostage2stand1 = {HOSTAGE21SPR, HOSTAGE21SPR, step, false, push_down, 40, 0, 0, Hostage2Think, NULL, DrawReact, &s_hostage2stand1};
FARSTATE s_hostage2stand2 = {HOSTAGE22SPR, HOSTAGE22SPR, step, false, push_down, 40, 0, 0, NULL, NULL, DrawReact, &s_hostage2stand1};
FARSTATE s_hostage3stand1 = {HOSTAGE31LSPR, HOSTAGE31RSPR, step, false, push_down, 30, 128, 0, Hostage3Think, NULL, WalkReact, &s_hostage3stand2};
FARSTATE s_hostage3stand2 = {HOSTAGE32LSPR, HOSTAGE32RSPR, step, false, push_down, 5, 128, 20, Hostage3Think, NULL, DrawReact, &s_hostage3stand1};

/*
==================
=
= SpawnHostage1
=
==================
*/

void SpawnHostage1(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = hostageobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_hostage1stand);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 32;
	new->shootable = false;
}

/*
==================
=
= Hostage1Think
=
==================
*/

void Hostage1Think(objtype *ob)
{
	if (US_RndT() > 70)
	{
		ChangeState(ob, &s_hostage1walk);
		if (US_RndT() > 32)
		{
			ob->xdir = 1;
		}
		else
		{
			ob->xdir = -1;
		}
	}
}

/*
==================
=
= SpawnHostage2
=
==================
*/

void SpawnHostage2(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = hostageobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	NewState(new, &s_hostage2stand1);
	new->hitnorth = 1;
	new->shootable = false;
}

/*
==================
=
= Hostage2Think
=
==================
*/

void Hostage2Think(objtype *ob)
{
	if (US_RndT() > 32)
	{
		ChangeState(ob, &s_hostage2stand2);
	}
}

/*
==================
=
= SpawnHostage3
=
==================
*/

void SpawnHostage3(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = hostageobj;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_hostage3stand1);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 32;
	new->shootable = false;
}

/*
==================
=
= Hostage3Think
=
==================
*/

void Hostage3Think(objtype *ob)
{
	if (US_RndT() > 32)
	{
		ob->xdir = 1;
	}
	else
	{
		ob->xdir = -1;
	}
}
#endif	// (EPISODE != 1)

//////////////////////////////////////////////////////////////////////////////
//
// BOSS HEAD / ENFORCER
//
// Dr. Mangle's head in the BETA and in Ep. 1 (and Ep. 3), Enforcer in Ep. 2
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_bosshead1     = {BOSSHEAD1SPR, BOSSHEAD1SPR, step, false, push_none, 10, 0, 0, BossheadThink, NULL, DrawReact, &s_bosshead1};
FARSTATE s_bosshead2     = {BOSSHEAD2SPR, BOSSHEAD2SPR, step, false, push_none, 10, 0, 0, BossheadThink, NULL, DrawReact, &s_bosshead1};
FARSTATE s_bosshead3     = {BOSSHEAD3SPR, BOSSHEAD3SPR, step, false, push_none, 200, 0, 0, BossheadThink, NULL, DrawReact, &s_bosshead1};
FARSTATE s_bossheadfade1 = {BOSSHEAD1FADESPR, BOSSHEAD1FADESPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, &s_bosshead1};
FARSTATE s_bossheadfade2 = {BOSSHEAD2FADESPR, BOSSHEAD2FADESPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, &s_bosshead2};
FARSTATE s_bossheadfade3 = {BOSSHEAD3FADESPR, BOSSHEAD3FADESPR, step, false, push_none, 10, 0, 0, NULL, NULL, DrawReact, &s_bosshead3};
#ifdef BETA
FARSTATE s_bossheadmetal = {BOSSHEADMETALSPR, BOSSHEADMETALSPR, step, false, push_none, 5, 0, 0, BossheadMetalThink, NULL, BossheadReact, &s_bossheadmetal};
#else
FARSTATE s_bossheadmetal = {BOSSHEADMETALSPR, BOSSHEADMETALSPR, step, false, push_none, 3, 0, 0, BossheadMetalThink, NULL, BossheadReact, &s_bossheadmetal};
#endif

/*
==================
=
= SpawnBosshead
=
==================
*/

void SpawnBosshead(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = bossobj;
	new->active = allways;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 32);
	NewState(new, &s_bosshead1);
	new->priority = 3;
	new->shootable = false;
#ifdef BETA
	if (mapon == 11)
	{
		new->shootable = true;
		new->health = 90;
	}
	new->needtoclip = cl_noclip;
#else
	new->needtoclip = cl_noclip;
	bossActive = false;
#endif
}

/*
==================
=
= BossheadThink
=
==================
*/

void BossheadThink(objtype *ob)
{
	Sint16 randval;
	
	if (US_RndT() > 220)
	{
		if (ob->state == &s_bosshead1)
		{
			ChangeState(ob, &s_bossheadfade1);
			return;
		}
		if (ob->state == &s_bosshead2)
		{
			ChangeState(ob, &s_bossheadfade2);
			return;
		}
		if (ob->state == &s_bosshead3)
		{
			ChangeState(ob, &s_bossheadfade3);
			return;
		}
	}
	else if (US_RndT() > 110)
	{
		ChangeState(ob, &s_bosshead2);
		return;
	}
	if (US_RndT() > 250 && ob->state != &s_bosshead3)
	{
		ChangeState(ob, &s_bosshead3);
	}
	if (bossActive)
	{
		randval = US_RndT();
		if (randval > 120)
		{
			ThrowPower(ob->midx, ob->top, dir_West);	//BUG? this uses up the player's grenades!
		}
		else
		{
			TankAttack2Think(ob);	// spawn a small rocket
			//Note: This causes a crash if the level didn't have any tanks in it!
		}
		ob->state = &s_bossheadmetal;
	}
}

/*
==================
=
= BossheadMetalThink
=
==================
*/

void BossheadMetalThink(objtype *ob)
{
	Sint16 randval, dist;

#if (EPISODE == 2) && !(defined BETA)
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
#endif
	randval = US_RndT();
	AccelerateY(ob, ob->y>player->y? -1 : 1, US_RndT()/8 + 4);
	if (ob->yspeed > 0)
	{
		ob->ydir = 1;
	}
	else
	{
		ob->ydir = -1;
	}
	AccelerateX(ob, ob->xdir, US_RndT()/8 + 4);
	if (ob->xspeed < 0)
	{
		dist = ob->left - player->right;
		if (dist < -8*PIXGLOBAL)
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
		dist = player->left - ob->right;
		if (dist < -8*PIXGLOBAL)
		{
			ob->xdir = -1;
		}
		else
		{
			ob->xdir = 1;
		}
	}
	if (randval > 150 && randval < 200)
	{
		if (ob->top > player->bottom || ob->bottom < player->top)
		{
			return;
		}
		if (SpawnEnemyShot(ob->midx, ob->y+15*PIXGLOBAL, &s_enemyshot) == -1)
		{
			return;
		}
		new->xspeed = ob->xdir * 60;
		new->yspeed = 0;
		SD_PlaySound(SNAKESHOOTSND);
	}
	else if (randval > 250)
	{
		ThrowPower(ob->midx, ob->top, dir_West);	//BUG? this uses up the player's grenades!
		ThrowPower(ob->midx, ob->top, dir_East);	//BUG? this uses up the player's grenades!
	}
}

/*
==================
=
= BossheadReact
=
==================
*/

void BossheadReact(objtype *ob)
{
	if (ob->top > player->top + 24)
	{
		ob->ydir = -1;
	}
	else if (ob->top < player->top + 24)
	{
		ob->ydir = 1;
	}
	PLACESPRITE;
}

#if (EPISODE == 1)
//////////////////////////////////////////////////////////////////////////////
//
// DR. MANGLE
//
// temp1: shot burst counter (always 0)
// temp7: turn cooldown for WalkReact
//
//////////////////////////////////////////////////////////////////////////////

FARSTATE s_manglestand1 = {MANGLESTAND1SPR, MANGLESTAND1SPR, step, false, push_down, 20, 0, 0, MangleThink, NULL, DrawReact, &s_manglestand2};
FARSTATE s_manglestand2 = {MANGLESTAND2SPR, MANGLESTAND2SPR, step, false, push_down, 20, 0, 0, MangleThink, NULL, DrawReact, &s_manglestand1};
FARSTATE s_manglewalk1  = {MANGLEWALK1LSPR, MANGLEWALK1RSPR, step, false, push_down, 10, 128, 0, MangleThink, NULL, WalkReact, &s_manglewalk2};
FARSTATE s_manglewalk2  = {MANGLEWALK2LSPR, MANGLEWALK2RSPR, step, false, push_down, 10, 128, 0, MangleThink, NULL, WalkReact, &s_manglewalk1};
FARSTATE s_manglejump   = {MANGLEWALK2LSPR, MANGLEWALK2RSPR, think, false, push_none, 0, 0, 0, ProjectileThink, NULL, MangleJumpReact, &s_manglejump};
FARSTATE s_mangledead1  = {MANGLEDEADSPR, MANGLEDEADSPR, step, false, push_down, 200, 0, 0, NULL, NULL, DrawReact, &s_mangledead2};
FARSTATE s_mangledead2  = {MANGLEDEADSPR, MANGLEDEADSPR, step, false, push_down, 2, 0, 0, MangleDeadThink, NULL, DrawReact, &s_mangledead2};
FARSTATE s_mangledead3  = {MANGLEDEADSPR, MANGLEDEADSPR, think, false, push_down, 0, 0, 0, NULL, NULL, DrawReact, &s_mangledead3};

/*
==================
=
= SpawnMangle
=
==================
*/

void SpawnMangle(Uint16 x, Uint16 y)
{
	GetNewObj(false);
	new->obclass = bossobj;
	new->active = allways;
	new->priority = 2;
	new->x = CONVERT_TILE_TO_GLOBAL(x);
	new->y = SPAWN_ADJUST_Y(y, 40);
	new->xdir = 1;
	new->ydir = 1;
	NewState(new, &s_manglestand1);
	new->hitnorth = 1;
	new->ticcount = US_RndT() / 0x20;
	new->shootable = false;
	new->health = bosshealth = 200;
	oldhealthbarlength = -1;
	bossdiv = bosshealth/20 - 1;	//BUG? see comments in UpdateScorebox()
}

/*
==================
=
= MangleThink
=
==================
*/

void MangleThink(objtype *ob)
{
	Sint16 shotx, dist;
	
	if (ob->health != bosshealth)
	{
		oldhealthbarlength = bosshealth;
		bosshealth = ob->health;
	}
	if (ob->obclass == decoobj)
	{
		ob->state = &s_mangledead1;
		ob->y += 2*TILEGLOBAL;
		ob->shootable = false;
		return;
	}
	if (ob->top > player->bottom || ob->bottom < player->top)
	{
		return;
	}
	
	if (US_RndT() > 230 && ob->shootable == true)
	{
		// Most of this branch appears to have been copied from GuardAttackThink
		// (see BM_ACT3.C) and the x/y offsets were adjusted accordingly.
		if (ob->xdir == 1)
		{
			shotx = ob->x + 7*TILEGLOBAL;
		}
		else
		{
			shotx = ob->x;
		}
		if (SpawnEnemyShot(shotx, ob->y+2*TILEGLOBAL, &s_bossshot1) == -1)
		{
			return;
		}
		new->xspeed = ob->xdir * 60;
		if (US_RndT() < 70)
		{
			new->yspeed = 0;
		}
		else if (ob->temp1 & 1)	// never true (temp1 starts at 0 and never changes)
		{
			new->yspeed = 4;
		}
		else
		{
			new->yspeed = -4;
		}
		SD_PlaySound(LASERSND);
	}
	if (ob->xdir == -1)
	{
		dist = ob->left - player->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*TILEGLOBAL)
		{
			ob->xdir = 1;
			return;
		}
		ob->yspeed = -24 - (US_RndT() / 0x10);
		ob->xspeed = -32;
		SD_PlaySound(GROWLSND);
		ob->shootable = true;
		ob->state = &s_manglejump;
		return;
	}
	else
	{
		dist = player->left - ob->right;
		if (dist > 2*TILEGLOBAL)
		{
			return;
		}
		if (dist < -8*TILEGLOBAL)
		{
			ob->xdir = -1;
			return;
		}
		ob->yspeed = -24 - (US_RndT() / 0x10);
		ob->xspeed = 32;
		SD_PlaySound(GROWLSND);
		ob->shootable = true;
		ob->state = &s_manglejump;
		return;
	}
}

/*
==================
=
= MangleDeadThink
=
==================
*/

void MangleDeadThink(objtype *ob)
{
	// Killing Dr. Mangle was originally supposed to show a bunch of text
	// messages (see BossDialog() in BM_SPEC.C), and gamestate.mapon would
	// need to be set to 12 for that dialog. I guess that's why the line that
	// sets gamestate.mapon back to 11 is still present here.
	gamestate.mapon = 11;
	ob->hitnorth = 1;
	ob->state = &s_mangledead3;
	ob->white = 0;
	playstate = ex_completed;
}

/*
==================
=
= MangleJumpReact
=
==================
*/

void MangleJumpReact(objtype *ob)
{
	if (ob->hitsouth)
	{
		ob->yspeed = 0;
	}
	if (ob->hitnorth)
	{
		ChangeState(ob, &s_manglewalk1);
	}
	PLACESPRITE;
}
#endif	// (EPISODE == 1)
