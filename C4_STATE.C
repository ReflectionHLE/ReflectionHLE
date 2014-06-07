/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
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

// C3_STATE.C

#include "DEF.H"
#pragma hdrstop

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



/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


dirtype opposite[9] =
	{south,west,north,east,southwest,northwest,northeast,southeast,nodir};



//===========================================================================


/*
===================
=
= Internal_SpawnNewObj
=
===================
*/
void Internal_SpawnNewObj (unsigned x, unsigned y, statetype *state, unsigned size,boolean UseDummy)
{
	extern objtype dummyobj;

	GetNewObj(UseDummy);
	new->size = size;
	new->state = state;
	new->ticcount = random (state->tictime)+1;

	new->tilex = x;
	new->tiley = y;
	new->x = ((long)x<<TILESHIFT)+TILEGLOBAL/2;
	new->y = ((long)y<<TILESHIFT)+TILEGLOBAL/2;
	CalcBounds(new);
	new->dir = nodir;
	new->active = noalways;

	if (new != &dummyobj)
		actorat[new->tilex][new->tiley] = new;
}

void Internal_SpawnNewObjFrac (long x, long y, statetype *state, unsigned size,boolean UseDummy)
{
	GetNewObj(UseDummy);
	new->size = size;
	new->state = state;
	new->ticcount = random (state->tictime)+1;
	new->active = noalways;

	new->x = x;
	new->y = y;
	new->tilex = x>>TILESHIFT;
	new->tiley = y>>TILESHIFT;
	CalcBounds(new);
	new->distance = 100;
	new->dir = nodir;
}




/*
===================
=
= CheckHandAttack
=
= If the object can move next to the player, it will return true
=
===================
*/

boolean CheckHandAttack (objtype *ob)
{
	long deltax,deltay,size;

	size = (long)ob->size + player->size + ob->speed*tics + SIZE_TEST;
	deltax = ob->x - player->x;
	deltay = ob->y - player->y;

	if (deltax > size || deltax < -size || deltay > size || deltay < -size)
		return false;

	return true;
}


/*
===================
=
= T_DoDamage
=
= Attacks the player if still nearby, then immediately changes to next state
=
===================
*/

void T_DoDamage (objtype *ob)
{
	int	points;


	if (CheckHandAttack(ob) && (!(ob->flags & of_damagedone)))
	{
		points = 0;

		switch (ob->obclass)
		{
		case orcobj:
			points = 4;
			break;
		case zombieobj:
		case trollobj:
			points = 8;
			break;
		case reddemonobj:
		case demonobj:
			points = 15;
			break;
		case spookobj:
			points = 2;
			break;
		case skeletonobj:
			points = 6;
			break;

		case wetobj:
			points = 7;
			break;

		}
		TakeDamage (EasyDoDamage(points));

		ob->flags |= of_damagedone;
	}
}


//==========================================================================

/*
==================================
=
= Walk
=
==================================
*/

boolean Walk (objtype *ob)
{
	switch (ob->dir)
	{
	case north:
		if (actorat[ob->tilex][ob->tiley-1])
			return false;
		ob->tiley--;
		ob->distance = TILEGLOBAL;
		return true;

	case northeast:
		if (actorat[ob->tilex+1][ob->tiley-1])
			return false;
		ob->tilex++;
		ob->tiley--;
		ob->distance = TILEGLOBAL;
		return true;

	case east:
		if (actorat[ob->tilex+1][ob->tiley])
			return false;
		ob->tilex++;
		ob->distance = TILEGLOBAL;
		return true;

	case southeast:
		if (actorat[ob->tilex+1][ob->tiley+1])
			return false;
		ob->tilex++;
		ob->tiley++;
		ob->distance = TILEGLOBAL;
		return true;

	case south:
		if (actorat[ob->tilex][ob->tiley+1])
			return false;
		ob->tiley++;
		ob->distance = TILEGLOBAL;
		return true;

	case southwest:
		if (actorat[ob->tilex-1][ob->tiley+1])
			return false;
		ob->tilex--;
		ob->tiley++;
		ob->distance = TILEGLOBAL;
		return true;

	case west:
		if (actorat[ob->tilex-1][ob->tiley])
			return false;
		ob->tilex--;
		ob->distance = TILEGLOBAL;
		return true;

	case northwest:
		if (actorat[ob->tilex-1][ob->tiley-1])
			return false;
		ob->tilex--;
		ob->tiley--;
		ob->distance = TILEGLOBAL;
		return true;

	case nodir:
		return false;
	}

	Quit ("Walk: Bad dir");
	return false;
}



/*
==================================
=
= ChaseThink
= have the current monster go after the player,
= either diagonally or straight on
=
==================================
*/

void ChaseThink (objtype *obj, boolean diagonal)
{
	int deltax,deltay,i;
	dirtype d[3];
	dirtype tdir, olddir, turnaround;


	olddir=obj->dir;
	turnaround=opposite[olddir];

	deltax=player->tilex - obj->tilex;
	deltay=player->tiley - obj->tiley;

	d[1]=nodir;
	d[2]=nodir;

	if (deltax>0)
		d[1]= east;
	if (deltax<0)
		d[1]= west;
	if (deltay>0)
		d[2]=south;
	if (deltay<0)
		d[2]=north;

	if (abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	if (d[1]==turnaround)
		d[1]=nodir;
	if (d[2]==turnaround)
		d[2]=nodir;


	if (diagonal)
	{                           /*ramdiagonals try the best dir first*/
		if (d[1]!=nodir)
		{
			obj->dir=d[1];
			if (Walk(obj))
				return;     /*either moved forward or attacked*/
		}

		if (d[2]!=nodir)
		{
			obj->dir=d[2];
			if (Walk(obj))
				return;
		}
	}
	else
	{                  /*ramstraights try the second best dir first*/

		if (d[2]!=nodir)
		{
			obj->dir=d[2];
			if (Walk(obj))
				return;
		}

		if (d[1]!=nodir)
		{
			obj->dir=d[1];
			if (Walk(obj))
				return;
		}
	}

/* there is no direct path to the player, so pick another direction */

	obj->dir=olddir;
	if (Walk(obj))
		return;

	if (US_RndT()>128) 	/*randomly determine direction of search*/
	{
		for (tdir=north;tdir<=west;tdir++)
		{
			if (tdir!=turnaround)
			{
				obj->dir=tdir;
				if (Walk(obj))
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
			  obj->dir=tdir;
			  if (Walk(obj))
				return;
			}
		}
	}

	obj->dir=turnaround;
	Walk(obj);		/*last chance, don't worry about returned value*/
}


/*
=================
=
= MoveObj
=
=================
*/

void MoveObj (objtype *ob, long move)
{
	ob->distance -=move;

	switch (ob->dir)
	{
	case north:
		ob->y -= move;
		return;
	case northeast:
		ob->x += move;
		ob->y -= move;
		return;
	case east:
		ob->x += move;
		return;
	case southeast:
		ob->x += move;
		ob->y += move;
		return;
	case south:
		ob->y += move;
		return;
	case southwest:
		ob->x -= move;
		ob->y += move;
		return;
	case west:
		ob->x -= move;
		return;
	case northwest:
		ob->x -= move;
		ob->y -= move;
		return;

	case nodir:
		return;
	}
}


/*
=================
=
= Chase
=
= returns true if hand attack range
=
=================
*/

boolean Chase (objtype *ob, boolean diagonal)
{
	long move;
	long deltax,deltay,size;

	ob->flags &= ~of_damagedone;

	move = ob->speed*tics;
	size = (long)ob->size + player->size + move + SIZE_TEST;

	while (move)
	{
		deltax = ob->x - player->x;
		deltay = ob->y - player->y;

		if (deltax <= size && deltax >= -size
		&& deltay <= size && deltay >= -size)
		{
			CalcBounds (ob);
			return true;
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}
		actorat[ob->tilex][ob->tiley] = 0;	// pick up marker from goal
		if (ob->dir == nodir)
			ob->dir = north;

		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
		move -= ob->distance;

		ChaseThink (ob,diagonal);
		if (!ob->distance)
			break;			// no possible move
		actorat[ob->tilex][ob->tiley] = ob;	// set down a new goal marker
	}
	CalcBounds (ob);
	return false;
}

//===========================================================================


/*
===================
=
= ShootActor
=
===================
*/

void ShootActor (objtype *ob, unsigned damage)
{
	ob->hitpoints -= damage;
	if (ob->hitpoints<=0)
	{
		switch (ob->obclass)
		{
		case reddemonobj:
			ob->state = &s_red_demondie1;
			break;
		case orcobj:
			ob->state = &s_orcdie1;
			break;
		case trollobj:
			ob->state = &s_trolldie1;
			break;
		case demonobj:
			ob->state = &s_demondie1;
			break;
		case mageobj:
			ob->state = &s_magedie1;
			break;
		case batobj:
			ob->state = &s_batdie1;
			break;
		case grelmobj:
			ob->state = &s_greldie1;
			break;

		case zombieobj:
			ob->state = &s_zombie_death1;
		break;

		case skeletonobj:
			ob->state = &s_skel_die1;
		break;

		case spookobj:
			ob->state = &s_spookdie;
		break;

		case wetobj:
			ob->state = &s_wet_die1;
		break;

		case eyeobj:
			ob->state = &s_eye_die1;
		break;

		case eshotobj:
		case mshotobj:
			ob->state = &s_bonus_die;
		break;

		case bonusobj:
		case freezeobj:
			switch (ob->temp1)
			{
				case B_POTION:
				case B_CHEST:
				case B_NUKE:
				case B_BOLT:
					ob->state = &s_pshot_exp1;
					ob->obclass = expobj;
					ob->ticcount = ob->state->tictime;
					SpawnBigExplosion(ob->x,ob->y,12,(16l<<16L));
					bordertime = FLASHTICS<<2;
					bcolor = 14;
					VW_ColorBorder(14 | 56);
					DisplaySMsg("Item destroyed", NULL);
					status_flag  = S_NONE;
					status_delay = 80;
				break;
			}
		break;

		}

		if (ob->obclass != solidobj)
		{
			ob->obclass = inertobj;
			ob->flags &= ~of_shootable;
			actorat[ob->tilex][ob->tiley] = NULL;
		}
	}
	else
	{
		switch (ob->obclass)
		{
		case reddemonobj:
			ob->state = &s_red_demonouch;
			break;
		case orcobj:
			ob->state = &s_orcouch;
			break;
		case trollobj:
			ob->state = &s_trollouch;
			break;
		case demonobj:
			ob->state = &s_demonouch;
			break;
		case mageobj:
			ob->state = &s_mageouch;
			break;

		case grelmobj:
			ob->state = &s_grelouch;
			break;

		case zombieobj:
			ob->state = &s_zombie_ouch;
		break;

		case spookobj:
			ob->state = &s_spookouch;
			break;

		case skeletonobj:
			ob->state = &s_skel_ouch;
			break;

		case wetobj:
			ob->state = &s_wet_ouch;
			break;

		case eyeobj:
			ob->state = &s_eye_ouch;
		break;

		}
	}
	ob->ticcount = ob->state->tictime;
}

