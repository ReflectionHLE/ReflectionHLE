/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2025 NY00123
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

// WL_STATE.C

#include "wl_def.h"
//#pragma hdrstop

REFKEEN_NS_B

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


dirtype opposite[9] =
	{west,southwest,south,southeast,east,northeast,north,northwest,nodir};

dirtype diagonal[9][9] =
{
/* east */	{nodir,nodir,northeast,nodir,nodir,nodir,southeast,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* north */ {northeast,nodir,nodir,nodir,northwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* west */  {nodir,nodir,northwest,nodir,nodir,nodir,southwest,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* south */ {southeast,nodir,nodir,nodir,southwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir}
};



void	SpawnNewObj (id0_unsigned_t tilex, id0_unsigned_t tiley, statetype *state);
void	NewState (objtype *ob, statetype *state);

id0_boolean_t TryWalk (objtype *ob);
void	MoveObj (objtype *ob, id0_long_t move);

void	KillActor (objtype *ob);
void	DamageActor (objtype *ob, id0_unsigned_t damage);

id0_boolean_t CheckLine (objtype *ob);
void FirstSighting (objtype *ob);
id0_boolean_t	CheckSight (objtype *ob);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/



//===========================================================================


/*
===================
=
= SpawnNewObj
=
= Spaws a new actor at the given TILE coordinates, with the given state, and
= the given size in GLOBAL units.
=
= newobj			= a pointer to an initialized new actor
=
===================
*/

void SpawnNewObj (id0_unsigned_t tilex, id0_unsigned_t tiley, statetype *state)
{
	GetNewActor ();
	newobj->state = state;
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	newobj->ticcount = BE_Cross_Brandom(state->tictime) + 1;
#else
	if (state->tictime)
		newobj->ticcount = US_RndT () % state->tictime;
	else
		newobj->ticcount = 0;
#endif

	newobj->tilex = tilex;
	newobj->tiley = tiley;
	newobj->x = ((id0_long_t)tilex<<TILESHIFT)+TILEGLOBAL/2;
	newobj->y = ((id0_long_t)tiley<<TILESHIFT)+TILEGLOBAL/2;
	newobj->dir = nodir;

	actorat[tilex][tiley] = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(newobj);
	newobj->areanumber =
		*(mapsegs[0] + farmapylookup[newobj->tiley]+newobj->tilex) - AREATILE;
}



/*
===================
=
= NewState
=
= Changes ob to a new state, setting ticcount to the max for that state
=
===================
*/

void NewState (objtype *ob, statetype *state)
{
	ob->state = state;
	ob->ticcount = state->tictime;
}



/*
=============================================================================

				ENEMY TILE WORLD MOVEMENT CODE

=============================================================================
*/


/*
==================================
=
= TryWalk
=
= Attempts to move ob in its current (ob->dir) direction.
=
= If blocked by either a wall or an actor returns FALSE
=
= If move is either clear or blocked only by a door, returns TRUE and sets
=
= ob->tilex			= new destination
= ob->tiley
= ob->areanumber    = the floor tile number (0-(NUMAREAS-1)) of destination
= ob->distance  	= TILEGLOBAl, or -doornumber if a door is blocking the way
=
= If a door is in the way, an OpenDoor call is made to start it opening.
= The actor code should wait until
= 	doorobjlist[-ob->distance].action = dr_open, meaning the door has been
=	fully opened
=
==================================
*/

#define CHECKDIAG(x,y)								\
{                                                   \
	temp=(id0_unsigned_t)actorat[x][y];                   \
	if (temp)                                       \
	{                                               \
		if (temp<256)                               \
			return false;                           \
		if (COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(temp)->flags&FL_SHOOTABLE)  \
			return false;                           \
	}                                               \
}

// *** S3DNA RESTORATION ***
// Different masking for doornum
#ifdef GAMEVER_NOAH3D
#define CHECKSIDE(x,y)								\
{                                                   \
	temp=(id0_unsigned_t)actorat[x][y];                   \
	if (temp)                                       \
	{                                               \
		if (temp<128)                               \
			return false;                           \
		if (temp<256)                               \
			doornum = temp&31;                      \
		else if (COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(temp)->flags&FL_SHOOTABLE)\
			return false;                           \
	}                                               \
}
#else
#define CHECKSIDE(x,y)								\
{                                                   \
	temp=(id0_unsigned_t)actorat[x][y];                   \
	if (temp)                                       \
	{                                               \
		if (temp<128)                               \
			return false;                           \
		if (temp<256)                               \
			doornum = temp&63;                      \
		else if (COMPAT_OBJ_CONVERT_DOS_PTR_TO_OBJ_PTR(temp)->flags&FL_SHOOTABLE)\
			return false;                           \
	}                                               \
}
#endif

id0_boolean_t TryWalk (objtype *ob)
{
	id0_int_t			doornum;
	id0_unsigned_t	temp;

	doornum = -1;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (ob->obclass == inertobj)
	{
		switch (ob->dir)
		{
		case north:
			ob->tiley--;
			break;

		case northeast:
			ob->tilex++;
			ob->tiley--;
			break;

		case east:
			ob->tilex++;
			break;

		case southeast:
			ob->tilex++;
			ob->tiley++;
			break;

		case south:
			ob->tiley++;
			break;

		case southwest:
			ob->tilex--;
			ob->tiley++;
			break;

		case west:
			ob->tilex--;
			break;

		case northwest:
			ob->tilex--;
			ob->tiley--;
			break;
		}
	}
	else
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11
		switch (ob->dir)
		{
		case north:
			// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
			if (ob->obclass == dogobj)
#else
			if (ob->obclass == dogobj || ob->obclass == fakeobj)
#endif
			{
				CHECKDIAG(ob->tilex,ob->tiley-1);
			}
			else
			{
				CHECKSIDE(ob->tilex,ob->tiley-1);
			}
			ob->tiley--;
			break;

		case northeast:
			CHECKDIAG(ob->tilex+1,ob->tiley-1);
			CHECKDIAG(ob->tilex+1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley-1);
			ob->tilex++;
			ob->tiley--;
			break;

		case east:
			// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
			if (ob->obclass == dogobj)
#else
			if (ob->obclass == dogobj || ob->obclass == fakeobj)
#endif
			{
				CHECKDIAG(ob->tilex+1,ob->tiley);
			}
			else
			{
				CHECKSIDE(ob->tilex+1,ob->tiley);
			}
			ob->tilex++;
			break;

		case southeast:
			CHECKDIAG(ob->tilex+1,ob->tiley+1);
			CHECKDIAG(ob->tilex+1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley+1);
			ob->tilex++;
			ob->tiley++;
			break;

		case south:
			// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
			if (ob->obclass == dogobj)
#else
			if (ob->obclass == dogobj || ob->obclass == fakeobj)
#endif
			{
				CHECKDIAG(ob->tilex,ob->tiley+1);
			}
			else
			{
				CHECKSIDE(ob->tilex,ob->tiley+1);
			}
			ob->tiley++;
			break;

		case southwest:
			CHECKDIAG(ob->tilex-1,ob->tiley+1);
			CHECKDIAG(ob->tilex-1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley+1);
			ob->tilex--;
			ob->tiley++;
			break;

		case west:
			// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10) || (defined GAMEVER_NOAH3D)
			if (ob->obclass == dogobj)
#else
			if (ob->obclass == dogobj || ob->obclass == fakeobj)
#endif
			{
				CHECKDIAG(ob->tilex-1,ob->tiley);
			}
			else
			{
				CHECKSIDE(ob->tilex-1,ob->tiley);
			}
			ob->tilex--;
			break;

		case northwest:
			CHECKDIAG(ob->tilex-1,ob->tiley-1);
			CHECKDIAG(ob->tilex-1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley-1);
			ob->tilex--;
			ob->tiley--;
			break;

		case nodir:
			return false;

		default:
			Quit ("Walk: Bad dir");
		}

	if (doornum != -1)
	{
		OpenDoor (doornum);
		ob->distance = -doornum-1;
		return true;
	}


	ob->areanumber =
		*(mapsegs[0] + farmapylookup[ob->tiley]+ob->tilex) - AREATILE;

	ob->distance = TILEGLOBAL;
	return true;
}



/*
==================================
=
= SelectDodgeDir
=
= Attempts to choose and initiate a movement for ob that sends it towards
= the player while dodging
=
= If there is no possible move (ob is totally surrounded)
=
= ob->dir			=	nodir
=
= Otherwise
=
= ob->dir			= new direction to follow
= ob->distance		= TILEGLOBAL or -doornumber
= ob->tilex			= new destination
= ob->tiley
= ob->areanumber    = the floor tile number (0-(NUMAREAS-1)) of destination
=
==================================
*/

void SelectDodgeDir (objtype *ob)
{
	id0_int_t 		deltax,deltay,i;
	id0_unsigned_t	absdx,absdy;
	dirtype 	dirtry[5];
	dirtype 	turnaround,tdir;

	if (ob->flags & FL_FIRSTATTACK)
	{
	//
	// turning around is only ok the very first time after noticing the
	// player
	//
		turnaround = nodir;
		ob->flags &= ~FL_FIRSTATTACK;
	}
	else
		turnaround=opposite[ob->dir];

	deltax = player->tilex - ob->tilex;
	deltay = player->tiley - ob->tiley;

//
// arange 5 direction choices in order of preference
// the four cardinal directions plus the diagonal straight towards
// the player
//

	if (deltax>0)
	{
		dirtry[1]= east;
		dirtry[3]= west;
	}
	else
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		if (deltax<=0)
#endif
	{
		dirtry[1]= west;
		dirtry[3]= east;
	}

	if (deltay>0)
	{
		dirtry[2]= south;
		dirtry[4]= north;
	}
	else
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		if (deltay<=0)
#endif
	{
		dirtry[2]= north;
		dirtry[4]= south;
	}

//
// randomize a bit for dodging
//
	absdx = abs(deltax);
	absdy = abs(deltay);

	if (absdx > absdy)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	if (US_RndT() < 128)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	dirtry[0] = diagonal [ dirtry[1] ] [ dirtry[2] ];

//
// try the directions util one works
//
	for (i=0;i<5;i++)
	{
		if ( dirtry[i] == nodir || dirtry[i] == turnaround)
			continue;

		ob->dir = dirtry[i];
		if (TryWalk(ob))
			return;
	}

//
// turn around only as a last resort
//
	if (turnaround != nodir)
	{
		ob->dir = turnaround;

		if (TryWalk(ob))
			return;
	}

	ob->dir = nodir;
}


/*
============================
=
= SelectChaseDir
=
= As SelectDodgeDir, but doesn't try to dodge
=
============================
*/

void SelectChaseDir (objtype *ob)
{
	id0_int_t deltax,deltay,i;
	dirtype d[3];
	// (REFKEEN) Incrementing/Decrementing an enum is a bad idea (leading to undefined behaviors in C, including "Bad dir" bug reproduced with Catacomb Abyss),
	// and illegal in C++. Hence, tdir is redefined to be a (signed) int here. Casts are done (to be compatible with C++).
	int tdir;
	dirtype /*tdir, */olddir, turnaround;


	olddir=ob->dir;
	turnaround=opposite[olddir];

	deltax=player->tilex - ob->tilex;
	deltay=player->tiley - ob->tiley;

	d[1]=nodir;
	d[2]=nodir;

	if (deltax>0)
		d[1]= east;
	else if (deltax<0)
		d[1]= west;
	if (deltay>0)
		d[2]=south;
	else if (deltay<0)
		d[2]=north;

	if (abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=(dirtype)tdir;
	}

	if (d[1]==turnaround)
		d[1]=nodir;
	if (d[2]==turnaround)
		d[2]=nodir;


	if (d[1]!=nodir)
	{
		ob->dir=d[1];
		if (TryWalk(ob))
			return;     /*either moved forward or attacked*/
	}

	if (d[2]!=nodir)
	{
		ob->dir=d[2];
		if (TryWalk(ob))
			return;
	}

/* there is no direct path to the player, so pick another direction */

	if (olddir!=nodir)
	{
		ob->dir=olddir;
		if (TryWalk(ob))
			return;
	}

	if (US_RndT()>128) 	/*randomly determine direction of search*/
	{
		for (tdir=north;tdir<=west;tdir++)
		{
			if (tdir!=turnaround)
			{
				ob->dir=(dirtype)tdir;
				if ( TryWalk(ob) )
					return;
			}
		}
	}
	else
	{
		for (tdir=west;tdir>=north;tdir--)
		{
			if (tdir!=turnaround)
			{
			  ob->dir=(dirtype)tdir;
			  if ( TryWalk(ob) )
				return;
			}
		}
	}

	if (turnaround !=  nodir)
	{
		ob->dir=turnaround;
		if (ob->dir != nodir)
		{
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			TryWalk(ob);
#else
			if ( TryWalk(ob) )
				return;
#endif
		}
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	ob->dir = nodir;		// can't move
#endif
}


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
============================
=
= SelectRunDir
=
= Run Away from player
=
============================
*/

void SelectRunDir (objtype *ob)
{
	id0_int_t deltax,deltay,i;
	dirtype d[3];
	// (REFKEEN) tdir was changed into an int, as done in SelectChaseDir
	int tdir;
	dirtype /*tdir, */olddir, turnaround;


	deltax=player->tilex - ob->tilex;
	deltay=player->tiley - ob->tiley;

	if (deltax<0)
		d[1]= east;
	else
		d[1]= west;
	if (deltay<0)
		d[2]=south;
	else
		d[2]=north;

	if (abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=(dirtype)tdir;
	}

	ob->dir=d[1];
	if (TryWalk(ob))
		return;     /*either moved forward or attacked*/

	ob->dir=d[2];
	if (TryWalk(ob))
		return;

/* there is no direct path to the player, so pick another direction */

	if (US_RndT()>128) 	/*randomly determine direction of search*/
	{
		for (tdir=north;tdir<=west;tdir++)
		{
			ob->dir=(dirtype)tdir;
			if ( TryWalk(ob) )
				return;
		}
	}
	else
	{
		for (tdir=west;tdir>=north;tdir--)
		{
			ob->dir=(dirtype)tdir;
			if ( TryWalk(ob) )
			  return;
		}
	}

	ob->dir = nodir;		// can't move
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


/*
=================
=
= MoveObj
=
= Moves ob be move global units in ob->dir direction
= Actors are not allowed to move inside the player
= Does NOT check to see if the move is tile map valid
=
= ob->x			= adjusted for new position
= ob->y
=
=================
*/

void MoveObj (objtype *ob, id0_long_t move)
{
	id0_long_t	deltax,deltay;

	switch (ob->dir)
	{
	case north:
		ob->y -= move;
		break;
	case northeast:
		ob->x += move;
		ob->y -= move;
		break;
	case east:
		ob->x += move;
		break;
	case southeast:
		ob->x += move;
		ob->y += move;
		break;
	case south:
		ob->y += move;
		break;
	case southwest:
		ob->x -= move;
		ob->y += move;
		break;
	case west:
		ob->x -= move;
		break;
	case northwest:
		ob->x -= move;
		ob->y -= move;
		break;

	case nodir:
		return;

	default:
		Quit ("MoveObj: bad dir!");
	}

//
// check to make sure it's not on top of player
//
	if (areabyplayer[ob->areanumber])
	{
		deltax = ob->x - player->x;
		if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
			goto moveok;
		deltay = ob->y - player->y;
		if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
			goto moveok;

		// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		if (obj->obclass == ghostobj)
			// REFKEEN: Assume NULL. Most known impact is on player death
			TakeDamage (tics*2,NULL);
//			TakeDamage (tics*2);
#else
		if (ob->obclass == ghostobj || ob->obclass == spectreobj)
			TakeDamage (tics*2,ob);
#endif
#endif

	//
	// back up
	//
		switch (ob->dir)
		{
		case north:
			ob->y += move;
			break;
		case northeast:
			ob->x -= move;
			ob->y += move;
			break;
		case east:
			ob->x -= move;
			break;
		case southeast:
			ob->x -= move;
			ob->y -= move;
			break;
		case south:
			ob->y -= move;
			break;
		case southwest:
			ob->x += move;
			ob->y -= move;
			break;
		case west:
			ob->x += move;
			break;
		case northwest:
			ob->x += move;
			ob->y += move;
			break;

		case nodir:
			return;
		}
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		return;
#endif
	}
moveok:
	ob->distance -=move;
}

/*
=============================================================================

							STUFF

=============================================================================
*/

/*
===============
=
= DropItem
=
= Tries to drop a bonus item somewhere in the tiles surrounding the
= given tilex/tiley
=
===============
*/

void DropItem (stat_t itemtype, id0_int_t tilex, id0_int_t tiley)
{
	id0_int_t	x,y,xl,xh,yl,yh;

//
// find a free spot to put it in
//
	if (!actorat[tilex][tiley])
	{
		PlaceItemType (itemtype, tilex,tiley);
		return;
	}

	xl = tilex-1;
	xh = tilex+1;
	yl = tiley-1;
	yh = tiley+1;

	for (x=xl ; x<= xh ; x++)
		for (y=yl ; y<= yh ; y++)
			if (!actorat[x][y])
			{
				PlaceItemType (itemtype, x,y);
				return;
			}
}



/*
===============
=
= KillActor
=
===============
*/

void KillActor (objtype *ob)
{
	id0_int_t	tilex,tiley;

	tilex = ob->tilex = ob->x >> TILESHIFT;		// drop item on center
	tiley = ob->tiley = ob->y >> TILESHIFT;

	switch (ob->obclass)
	{
	case guardobj:
		GivePoints (100);
		NewState (ob,&s_grddie1);
		PlaceItemType (bo_clip2,tilex,tiley);
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case officerobj:
		GivePoints (400);
		NewState (ob,&s_ofcdie1);
		PlaceItemType (bo_clip2,tilex,tiley);
		break;

	case mutantobj:
		GivePoints (700);
		NewState (ob,&s_mutdie1);
		PlaceItemType (bo_clip2,tilex,tiley);
		break;
#endif

	case ssobj:
		GivePoints (500);
		NewState (ob,&s_ssdie1);
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (!gamestate.machinegun)
#else
		if (gamestate.bestweapon < wp_machinegun)
#endif
			PlaceItemType (bo_machinegun,tilex,tiley);
		else
			PlaceItemType (bo_clip2,tilex,tiley);
		break;

	case dogobj:
		GivePoints (200);
		NewState (ob,&s_dogdie1);
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef SPEAR
	case bossobj:
		GivePoints (5000);
		NewState (ob,&s_bossdie1);
		PlaceItemType (bo_key1,tilex,tiley);
		break;

	// *** SHAREWARE V1.0 APOGEE RESTORATION  ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	case gretelobj:
		GivePoints (5000);
		NewState (ob,&s_greteldie1);
		PlaceItemType (bo_key1,tilex,tiley);
		break;

	case giftobj:
		GivePoints (5000);
		// *** SHAREWARE V1.1 APOGEE + S3DNA RESTORATION *** - Bits of different code (The APOGEE V1.1 EXE was originally released before registered versions, and the case of S3DNA just occurs to have a common function call)
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
		gamestate.killx = player->x;
		gamestate.killy = player->y;
#endif
		NewState (ob,&s_giftdie1);
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11) || (defined GAMEVER_NOAH3D)
		PlaceItemType (bo_key1,tilex,tiley);
#endif
		break;

	case fatobj:
		GivePoints (5000);
		gamestate.killx = player->x;
		gamestate.killy = player->y;
		NewState (ob,&s_fatdie1);
		// *** SHAREWARE V1.1 APOGEE + S3DNA RESTORATION *** - Bits of different code (The APOGEE V1.1 EXE was originally released before registered versions, and the case of S3DNA just occurs to have a common function call)
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11) || (defined GAMEVER_NOAH3D)
		PlaceItemType (bo_key1,tilex,tiley);
#endif
		break;
#endif // GAMEVER_WOLFREV > GV_WR_WL1AP10

	case schabbobj:
		GivePoints (5000);
		// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION *** - Bits of different code
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		gamestate.killx = player->x;
		gamestate.killy = player->y;
#endif
		NewState (ob,&s_schabbdie1);
#ifdef GAMEVER_NOAH3D
		PlaceItemType (bo_key1,tilex,tiley);
#endif
		A_DeathScream(ob);
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaceItemType (bo_key1,tilex,tiley);
#endif
		break;
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	case fakeobj:
		GivePoints (2000);
		NewState (ob,&s_fakedie1);
		break;
#endif

	case mechahitlerobj:
		GivePoints (5000);
		NewState (ob,&s_mechadie1);
		break;
	case realhitlerobj:
		GivePoints (5000);
		// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Bits of different code
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		gamestate.killx = player->x;
		gamestate.killy = player->y;
#endif
		NewState (ob,&s_hitlerdie1);
		A_DeathScream(ob);
		break;
#else
	case spectreobj:
		GivePoints (200);
		NewState (ob,&s_spectredie1);
		break;

	case angelobj:
		GivePoints (5000);
		NewState (ob,&s_angeldie1);
		break;

	case transobj:
		GivePoints (5000);
		NewState (ob,&s_transdie0);
		PlaceItemType (bo_key1,tilex,tiley);
		break;

	case uberobj:
		GivePoints (5000);
		NewState (ob,&s_uberdie0);
		PlaceItemType (bo_key1,tilex,tiley);
		break;

	case willobj:
		GivePoints (5000);
		NewState (ob,&s_willdie1);
		PlaceItemType (bo_key1,tilex,tiley);
		break;

	case deathobj:
		GivePoints (5000);
		NewState (ob,&s_deathdie1);
		PlaceItemType (bo_key1,tilex,tiley);
		break;
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	gamestate.killcount++;
#endif
	ob->flags &= ~FL_SHOOTABLE;
	// *** PRE-V1.4 APOGEE RESTORATION *** - Relocate this
	// based on version, but disable all that follows in v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	ob->flags |= FL_NONMARK;
#endif
	actorat[ob->tilex][ob->tiley] = 0;
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	ob->flags |= FL_NONMARK;
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
}



/*
===================
=
= DamageActor
=
= Called when the player succesfully hits an enemy.
=
= Does damage points to enemy ob, either putting it into a stun frame or
= killing it.
=
===================
*/

void DamageActor (objtype *ob, id0_unsigned_t damage)
{
	madenoise = true;

//
// do double damage if shooting a non attack mode actor
//
	if ( !(ob->flags & FL_ATTACKMODE) )
		damage <<= 1;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	switch (gamestate.difficulty)
	{
	case gd_baby:
		damage <<= 1;
		break;
	case gd_easy:
		damage += (damage >> 1);
		break;
	case gd_medium:
		damage += (damage >> 2);
		break;
	}
#endif

	ob->hitpoints -= damage;

	if (ob->hitpoints<=0)
		KillActor (ob);
	else
	{
		if (! (ob->flags & FL_ATTACKMODE) )
			FirstSighting (ob);		// put into combat mode

		// *** PRE-V1.4 APOGEE RESTORATION *** - Disabling pain1 states

		switch (ob->obclass)		// dogs only have one hit point
		{
		case guardobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			NewState (ob,&s_grdpain);
#else
			if (ob->hitpoints&1)
				NewState (ob,&s_grdpain);
			else
				NewState (ob,&s_grdpain1);
#endif
			break;

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		case officerobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			NewState (ob,&s_ofcpain);
#else
			if (ob->hitpoints&1)
				NewState (ob,&s_ofcpain);
			else
				NewState (ob,&s_ofcpain1);
#endif
			break;

		case mutantobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			NewState (ob,&s_mutpain);
#else
			if (ob->hitpoints&1)
				NewState (ob,&s_mutpain);
			else
				NewState (ob,&s_mutpain1);
#endif
			break;
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

		case ssobj:
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			NewState (ob,&s_sspain);
#else
			if (ob->hitpoints&1)
				NewState (ob,&s_sspain);
			else
				NewState (ob,&s_sspain1);
#endif

			break;

		}
	}
}

/*
=============================================================================

							CHECKSIGHT

=============================================================================
*/


/*
=====================
=
= CheckLine
=
= Returns true if a straight line between the player and ob is unobstructed
=
=====================
*/

id0_boolean_t CheckLine (objtype *ob)
{
	id0_int_t	x1,y1,xt1,yt1,x2,y2,xt2,yt2;
	id0_int_t	x,y;
	// *** SHAREWARE V1.0 APOGEE RESTORATION *** - v1.0 specific variables
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_int_t	xd1,xd2,yd1,yd2; // This should be the order
#endif
	id0_int_t	xdist,ydist,xstep,ystep;
	id0_int_t	temp;
	id0_int_t	partial,delta;
	// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Don't define this in v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	id0_long_t	ltemp;
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_unsigned_t	xfrac,yfrac,deltafrac;
#else
	id0_int_t	xfrac,yfrac,deltafrac;
#endif
	id0_unsigned_t	value,intercept;

	x1 = ob->x >> UNSIGNEDSHIFT;		// 1/256 tile precision
	y1 = ob->y >> UNSIGNEDSHIFT;
	xt1 = x1 >> 8;
	yt1 = y1 >> 8;

	x2 = plux;
	y2 = pluy;
	xt2 = player->tilex;
	yt2 = player->tiley;


	xdist = abs(xt2-xt1);

	// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Relocate line based on version
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	ydist = abs(yt2-yt1);
#endif

	if (xdist > 0)
	{
		if (xt2 > xt1)
		{
			partial = 256-(x1&0xff);
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			xd1 = xt1;
			xd2 = xt2;
			yd1 = y1;
			yd2 = y2;
			deltafrac = x2-x1;
#else
			xstep = 1;
#endif
		}
		else
		{
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			partial = 256-(x2&0xff);
			xd1 = xt2;
			xd2 = xt1;
			yd1 = y2;
			yd2 = y1;
			deltafrac = x1-x2;
#else
			partial = x1&0xff;
			xstep = -1;
#endif
		}

		// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		delta = yd2-yd1;
		ystep = ((id0_long_t)delta<<8)/deltafrac;
		yfrac = yd1 + (((id0_long_t)ystep*partial) >>8);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (ystep < 0)
		{
			xd1--;
			xd2--;
		}
#endif
#else // GAMEVER_WOLFREV > GV_WR_WL1AP10
		deltafrac = abs(x2-x1);
		delta = y2-y1;
		ltemp = ((id0_long_t)delta<<8)/deltafrac;
		if (ltemp > 0x7fffl)
			ystep = 0x7fff;
		else if (ltemp < -0x7fffl)
			ystep = -0x7fff;
		else
			ystep = ltemp;
		yfrac = y1 + (((id0_long_t)ystep*partial) >>8);

		x = xt1+xstep;
		xt2 += xstep;
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		for (x = xd1+1; x <= xd2; x++)
#else
		do
#endif
		{
			y = yfrac>>8;
			yfrac += ystep;

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			if (!(value = (id0_unsigned_t)tilemap[x][y]))
				continue;
#else
			value = (id0_unsigned_t)tilemap[x][y];
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			value &= 0xFFDF;
#endif
			x += xstep;

			if (!value)
				continue;
#endif

			if (value<128 || value>256)
				return false;

			//
			// see if the door is open enough
			//
			value &= ~0x80;
			intercept = yfrac-ystep/2;

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if (doorposition[value]<16384)
				return false;

#endif
			if (intercept>doorposition[value])
				return false;

		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		}
#else
		} while (x != xt2);
#endif
	}

	// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Relocate line based on version
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	ydist = abs(yt2-yt1);
#endif

	if (ydist > 0)
	{
		if (yt2 > yt1)
		{
			partial = 256-(y1&0xff);
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			xd1 = x1;
			xd2 = x2;
			yd1 = yt1;
			yd2 = yt2;
			deltafrac = y2-y1;
#else
			ystep = 1;
#endif
		}
		else
		{
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			partial = 256-(y2&0xff);
			xd1 = x2;
			xd2 = x1;
			yd1 = yt2;
			yd2 = yt1;
			deltafrac = y1-y2;
#else
			partial = y1&0xff;
			ystep = -1;
#endif
		}

		// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		delta = xd2-xd1;
		xstep = ((id0_long_t)delta<<8)/deltafrac;
		xfrac = xd1 + (((id0_long_t)xstep*partial) >>8);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (xstep < 0)
		{
			yd1--;
			yd2--;
		}
#endif
#else // GAMEVER_WOLFREV > GV_WR_WL1AP10
		deltafrac = abs(y2-y1);
		delta = x2-x1;
		ltemp = ((id0_long_t)delta<<8)/deltafrac;
		if (ltemp > 0x7fffl)
			xstep = 0x7fff;
		else if (ltemp < -0x7fffl)
			xstep = -0x7fff;
		else
			xstep = ltemp;
		xfrac = x1 + (((id0_long_t)xstep*partial) >>8);

		y = yt1 + ystep;
		yt2 += ystep;
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		for (y = yd1+1; y <= yd2; y++)
#else
		do
#endif
		{
			x = xfrac>>8;
			xfrac += xstep;

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			if (!(value = (id0_unsigned_t)tilemap[x][y]))
				continue;
#else
			value = (id0_unsigned_t)tilemap[x][y];
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			value &= 0xFFDF;
#endif
			y += ystep;

			if (!value)
				continue;
#endif

			if (value<128 || value>256)
				return false;

			//
			// see if the door is open enough
			//
			value &= ~0x80;
			intercept = xfrac-xstep/2;

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if (doorposition[value]<16384)
				return false;

#endif
			if (intercept>doorposition[value])
				return false;
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		}
#else
		} while (y != yt2);
#endif
	}

	return true;
}



/*
================
=
= CheckSight
=
= Checks a straight line between player and current object
=
= If the sight is ok, check alertness and angle to see if they notice
=
= returns true if the player has been spoted
=
================
*/

#define MINSIGHT	0x18000l

id0_boolean_t CheckSight (objtype *ob)
{
	id0_long_t		deltax,deltay;

//
// don't bother tracing a line if the area isn't connected to the player's
//
	if (!areabyplayer[ob->areanumber])
		return false;

//
// if the player is real close, sight is automatic
//
	deltax = player->x - ob->x;
	deltay = player->y - ob->y;

	if (deltax > -MINSIGHT && deltax < MINSIGHT
	&& deltay > -MINSIGHT && deltay < MINSIGHT)
		return true;

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
//
// see if they are looking in the right direction
//
	switch (ob->dir)
	{
	case north:
		if (deltay > 0)
			return false;
		break;

	case east:
		if (deltax < 0)
			return false;
		break;

	case south:
		if (deltay < 0)
			return false;
		break;

	case west:
		if (deltax > 0)
			return false;
		break;
	}
#endif

//
// trace a line to check for blocking tiles (corners)
//
	return CheckLine (ob);

}



/*
===============
=
= FirstSighting
=
= Puts an actor into attack mode and possibly reverses the direction
= if the player is behind it
=
===============
*/

void FirstSighting (objtype *ob)
{
//
// react to the player
//
	switch (ob->obclass)
	{
	case guardobj:
		PlaySoundLocActor(HALTSND,ob);
		NewState (ob,&s_grdchase1);
		ob->speed *= 3;			// go faster when chasing player
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case officerobj:
		PlaySoundLocActor(SPIONSND,ob);
		NewState (ob,&s_ofcchase1);
		ob->speed *= 5;			// go faster when chasing player
		break;

	case mutantobj:
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		PlaySoundLocActor(D_OXSND,ob);
#endif
		NewState (ob,&s_mutchase1);
		ob->speed *= 3;			// go faster when chasing player
		break;
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	// *** PRE-V1.4 APOGEE + ALPHA RESTORATION *** - Relocate based on version
	// (Weirdly enough the alpha is like v1.4 here)
#if (GAMEVER_WOLFREV <= GV_WR_WL920312) || (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	case ssobj:
		// *** ALPHA RESTORATION ***
	#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		PlaySoundLocActor(HALTSND,ob);
	#else
		PlaySoundLocActor(SCHUTZADSND,ob);
	#endif
		NewState (ob,&s_sschase1);
		ob->speed *= 4;			// go faster when chasing player
		break;

	case dogobj:
		PlaySoundLocActor(DOGBARKSND,ob);
		NewState (ob,&s_dogchase1);
		ob->speed *= 2;			// go faster when chasing player
		break;
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef SPEAR
	case bossobj:
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
		PlaySoundLocActor(GUTENTAGSND,ob);
#else
		SD_PlaySound(GUTENTAGSND);
#endif
		NewState (ob,&s_bosschase1);
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		ob->speed *= 3;			// go faster when chasing player
#else
		ob->speed = SPDPATROL*3;	// go faster when chasing player
#endif
		break;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	case gretelobj:
		SD_PlaySound(KEINSND);
		NewState (ob,&s_gretelchase1);
		ob->speed *= 3;			// go faster when chasing player
		break;

	case giftobj:
		SD_PlaySound(EINESND);
		NewState (ob,&s_giftchase1);
		ob->speed *= 3;			// go faster when chasing player
		break;

	case fatobj:
		SD_PlaySound(ERLAUBENSND);
		NewState (ob,&s_fatchase1);
		ob->speed *= 3;			// go faster when chasing player
		break;
#endif

	case schabbobj:
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
		PlaySoundLocActor(SCHABBSHASND,ob);
#else
		SD_PlaySound(SCHABBSHASND);
#endif
		NewState (ob,&s_schabbchase1);
		ob->speed *= 3;			// go faster when chasing player
		break;

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	case fakeobj:
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(TOT_HUNDSND,ob);
#else
		SD_PlaySound(TOT_HUNDSND);
#endif
		NewState (ob,&s_fakechase1);
		ob->speed *= 3;			// go faster when chasing player
		break;
#endif

	case mechahitlerobj:
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
		PlaySoundLocActor(DIESND,ob);
#else
		SD_PlaySound(DIESND);
#endif
		NewState (ob,&s_mechachase1);
		ob->speed *= 3;			// go faster when chasing player
		break;

	case realhitlerobj:
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PlaySoundLocActor(DIESND,ob);
#else
		SD_PlaySound(DIESND);
#endif
		NewState (ob,&s_hitlerchase1);
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		ob->speed *= 3;			// go faster when chasing player
#else
		ob->speed *= 5;			// go faster when chasing player
#endif
		break;

	// *** PRE-V1.4 APOGEE RESTORATION *** - Relocate based on version
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	case ssobj:
		PlaySoundLocActor(SCHUTZADSND,ob);
		NewState (ob,&s_sschase1);
		ob->speed *= 4;			// go faster when chasing player
		break;

	case dogobj:
		PlaySoundLocActor(DOGBARKSND,ob);
		NewState (ob,&s_dogchase1);
		ob->speed *= 2;			// go faster when chasing player
		break;
#endif

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	case ghostobj:
		NewState (ob,&s_blinkychase1);
		ob->speed *= 2;			// go faster when chasing player
		break;
#endif
#else

	case spectreobj:
		SD_PlaySound(GHOSTSIGHTSND);
		NewState (ob,&s_spectrechase1);
		ob->speed = 800;			// go faster when chasing player
		break;

	case angelobj:
		SD_PlaySound(ANGELSIGHTSND);
		NewState (ob,&s_angelchase1);
		ob->speed = 1536;			// go faster when chasing player
		break;

	case transobj:
		SD_PlaySound(TRANSSIGHTSND);
		NewState (ob,&s_transchase1);
		ob->speed = 1536;			// go faster when chasing player
		break;

	case uberobj:
		NewState (ob,&s_uberchase1);
		ob->speed = 3000;			// go faster when chasing player
		break;

	case willobj:
		SD_PlaySound(WILHELMSIGHTSND);
		NewState (ob,&s_willchase1);
		ob->speed = 2048;			// go faster when chasing player
		break;

	case deathobj:
		SD_PlaySound(KNIGHTSIGHTSND);
		NewState (ob,&s_deathchase1);
		ob->speed = 2048;			// go faster when chasing player
		break;

#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	}

	if (ob->distance < 0)
		ob->distance = 0;	// ignore the door opening command

	ob->flags |= FL_ATTACKMODE|FL_FIRSTATTACK;
}



/*
===============
=
= SightPlayer
=
= Called by actors that ARE NOT chasing the player.  If the player
= is detected (by sight, noise, or proximity), the actor is put into
= it's combat frame and true is returned.
=
= Incorporates a random reaction delay
=
===============
*/

id0_boolean_t SightPlayer (objtype *ob)
{
	if (ob->flags & FL_ATTACKMODE)
		Quit ("An actor in ATTACKMODE called SightPlayer!");

	if (ob->temp2)
	{
	//
	// count down reaction time
	//
		ob->temp2 -= tics;
		if (ob->temp2 > 0)
			return false;
		ob->temp2 = 0;					// time to react
	}
	else
	{
		if (!areabyplayer[ob->areanumber])
			return false;

		if (ob->flags & FL_AMBUSH)
		{
			if (!CheckSight (ob))
				return false;
			ob->flags &= ~FL_AMBUSH;
		}
		else
		{
			if (!madenoise && !CheckSight (ob))
				return false;
		}


		switch (ob->obclass)
		{
		case guardobj:
			ob->temp2 = 1+US_RndT()/4;
			break;
		case officerobj:
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			ob->temp2 = 1+US_RndT()/5;
#else
			ob->temp2 = 2;
#endif
			break;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		case mutantobj:
			ob->temp2 = 1+US_RndT()/6;
			break;
#endif
		case ssobj:
			ob->temp2 = 1+US_RndT()/6;
			break;
		case dogobj:
			ob->temp2 = 1+US_RndT()/8;
			break;

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		case bossobj:
		case schabbobj:
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		case fakeobj:
#endif
		case mechahitlerobj:
		case realhitlerobj:
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		case gretelobj:
		case giftobj:
		case fatobj:
#endif
		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
		case spectreobj:
		case angelobj:
		case transobj:
		case uberobj:
		case willobj:
		case deathobj:
#endif
			ob->temp2 = 1;
			break;
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		default:
			sprintf (str,"SightPlayer: Invalid object class %u!\n",ob->obclass);
			Quit (str);
#endif
		}
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		switch (gamestate.difficulty)
		{
		case gd_baby:
			ob->temp2 += (ob->temp2+3)>>1;
			break;
		case gd_easy:
			ob->temp2 += (ob->temp2+6)>>2;
			break;
		case gd_medium:
			ob->temp2 += (ob->temp2+9)>>3;
			break;
		}
#endif
		return false;
	}

	FirstSighting (ob);

	return true;
}

REFKEEN_NS_E
