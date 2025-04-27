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

void BadState(objtype *ob);

FARSTATE s_nullstate = {0, 0, think, false, push_none, 0, 0, 0, NULL, NULL, NULL, NULL};
#if 0 // REFKEEN: This is unused
//#pragma warn -sus	//BadState is not a valid contact function. Nobody cares.
FARSTATE s_badstate  = {0, 0, think, false, push_none, 0, 0, 0, BadState, BadState, BadState, NULL};
//#pragma warn +sus
#endif

Sint16 wallclip[8][16] = {			// the height of a given point in a tile
{ 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256},
{   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
{   0,0x08,0x10,0x18,0x20,0x28,0x30,0x38,0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78},
{0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8},
{   0,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xa0,0xb0,0xc0,0xd0,0xe0,0xf0},
{0x78,0x70,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x10,0x08,   0},
{0xf8,0xf0,0xe8,0xe0,0xd8,0xd0,0xc8,0xc0,0xb8,0xb0,0xa8,0xa0,0x98,0x90,0x88,0x80},
{0xf0,0xe0,0xd0,0xc0,0xb0,0xa0,0x90,0x80,0x70,0x60,0x50,0x40,0x30,0x20,0x10,   0}
};

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

Sint16 xtry;
Sint16 ytry;
static boolean playerkludgeclipcancel;	//is always false; only used by ClipToWalls()
static Uint16 oldtileleft;
static Uint16 oldtiletop;
static Uint16 oldtileright;
static Uint16 oldtilebottom;
static Uint16 oldtilemidx;
static Uint16 oldleft;
static Uint16 oldtop;
static Uint16 oldright;
static Uint16 oldbottom;
static Uint16 oldmidx;
static Sint16 leftmoved;
static Sint16 topmoved;
static Sint16 rightmoved;
static Sint16 bottommoved;
static Sint16 midxmoved;

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

//==========================================================================

/*
====================
=
= MoveObjVert
=
====================
*/

static void MoveObjVert(objtype *ob, Sint16 ymove)
{
	ob->y += ymove;
	ob->top += ymove;
	ob->bottom += ymove;
	ob->tiletop = CONVERT_GLOBAL_TO_TILE(ob->top);
	ob->tilebottom = CONVERT_GLOBAL_TO_TILE(ob->bottom);
}

/*
====================
=
= MoveObjHoriz
=
====================
*/

static void MoveObjHoriz(objtype *ob, Sint16 xmove)
{
	//BUG? ob->midx is not adjusted
	ob->x += xmove;
	ob->left += xmove;
	ob->right += xmove;
	ob->tileleft = CONVERT_GLOBAL_TO_TILE(ob->left);
	ob->tileright = CONVERT_GLOBAL_TO_TILE(ob->right);
}

//==========================================================================

/*
====================
=
= PlayerBottomKludge
=
====================
*/

static void PlayerBottomKludge(objtype *ob)
{
	Uint16 id0_far *map;
	Uint16 wall, clip, xpix;
	Sint16 xmove, ymove;

	map = (Uint16 id0_far *)(mapsegs[1]) + mapbwidthtable[ob->tilebottom-1]/2;
	if (ob->xdir == 1)
	{
		xpix = 0;
		map += ob->tileright;
		xmove = ob->right - ob->midx;
		if (tinf[*(map-mapwidth)+WESTWALL] || tinf[*map+WESTWALL])
		{
			return;
		}
	}
	else
	{
		xpix = 15;
		map += ob->tileleft;
		xmove = ob->left - ob->midx;
		if (tinf[*(map-mapwidth)+EASTWALL] || tinf[*map+EASTWALL])
		{
			return;
		}
	}
	if (!tinf[*map+NORTHWALL])
	{
		map += mapwidth;
		if ((wall = tinf[*map+NORTHWALL]) == 1)
		{
			clip = wallclip[wall&SLOPEMASK][xpix];
			ymove = CONVERT_TILE_TO_GLOBAL(ob->tilebottom) + clip - 1 -ob->bottom;
			if (ymove <= 0 && ymove >= -bottommoved)
			{
				ob->hitnorth = wall;
				MoveObjVert(ob, ymove);
				MoveObjHoriz(ob, xmove);
				return;
			}
		}
	}
}

/*
====================
=
= PlayerTopKludge
=
====================
*/

static void PlayerTopKludge(objtype *ob)
{
	Uint16 id0_far *map;
	Uint16 xpix, wall, clip;
	Sint16 move;

	map = (Uint16 id0_far *)(mapsegs[1]) + mapbwidthtable[ob->tiletop+1]/2;
	if (ob->xdir == 1)
	{
		xpix = 0;
		map += ob->tileright;
		if (tinf[map[mapwidth]+WESTWALL] || tinf[map[2*mapwidth]+WESTWALL])
		{
			return;
		}
	}
	else
	{
		xpix = 15;
		map += ob->tileleft;
		if (tinf[map[mapwidth]+EASTWALL] || tinf[map[2*mapwidth]+EASTWALL])
		{
			return;
		}
	}
	if (!tinf[map[0]+SOUTHWALL])
	{
		map -= mapwidth;
		if ((wall = tinf[map[0]+SOUTHWALL]) != 0)
		{
			clip = wallclip[wall&SLOPEMASK][xpix];
			move = CONVERT_TILE_TO_GLOBAL(ob->tiletop+1) - clip - ob->top;
			if (move >= 0 && move <= -topmoved)
			{
				ob->hitsouth = wall;
				MoveObjVert(ob, move);
				return;
			}
		}
	}
}

/*
===========================
=
= ClipToEnds
=
===========================
*/

static void ClipToEnds(objtype *ob)
{
	Uint16 id0_far *map;
	Uint16 wall, y, clip;
	Sint16 totalmove, maxmove, move;
	Uint16 midxpix;
	
	midxpix = CONVERT_GLOBAL_TO_PIXEL(ob->midx & 0xF0);
	maxmove = -abs(midxmoved)-bottommoved-16;
	map = (Uint16 id0_far *)(mapsegs[1]) + (mapbwidthtable-1)[oldtilebottom]/2 + ob->tilemidx;
	for (y=oldtilebottom-1; y <= ob->tilebottom; y++,map+=mapwidth)
	{
		if ((wall = tinf[*map + NORTHWALL]) != 0)
		{
			clip = wallclip[wall&SLOPEMASK][midxpix];
			move = (CONVERT_TILE_TO_GLOBAL(y) + clip)-1-ob->bottom;
			if (move < 0 && move >= maxmove)
			{
				ob->hitnorth = wall;
				MoveObjVert(ob, move);
				return;
			}
		}
	}
	maxmove = abs(midxmoved)-topmoved+16;
	map = (Uint16 id0_far *)(mapsegs[1]) + (mapbwidthtable+1)[oldtiletop]/2 + ob->tilemidx;
	for (y=oldtiletop+1; y >= ob->tiletop; y--,map-=mapwidth)	// BUG: unsigned comparison - loop never ends if ob->tiletop is 0
	{
		if ((wall = tinf[*map + SOUTHWALL]) != 0)
		{
			clip = wallclip[wall&SLOPEMASK][midxpix];
			move = CONVERT_TILE_TO_GLOBAL(y+1) - clip - ob->top;
			if (move > 0 && move <= maxmove)
			{
				totalmove = ytry+move;
				if (totalmove < TILEGLOBAL && totalmove > -TILEGLOBAL)
				{
					ob->hitsouth = wall;
					MoveObjVert(ob, move);
					//BUG? no return here
				}
			}
		}
	}
}

/*
===========================
=
= ClipToSides
=
===========================
*/

static void ClipToSides(objtype *ob)
{
	Sint16 move, y, top, bottom;
	Uint16 id0_far *map;
	
	top = ob->tiletop;
	if (ob->hitsouth > 1)
	{
		top++;
	}
	bottom = ob->tilebottom;
	if (ob->hitnorth > 1)
	{
		bottom--;
	}
	for (y=top; y<=bottom; y++)
	{
		map = (Uint16 id0_far *)(mapsegs[1]) + mapbwidthtable[y]/2 + ob->tileleft;
		if ((ob->hiteast = tinf[*map+EASTWALL]) != 0)
		{
			move = CONVERT_TILE_TO_GLOBAL(ob->tileleft+1) - ob->left;
			MoveObjHoriz(ob, move);
			return;
		}
	}
	for (y=top; y<=bottom; y++)
	{
		map = (Uint16 id0_far *)(mapsegs[1]) + mapbwidthtable[y]/2 + ob->tileright;
		if ((ob->hitwest = tinf[*map+WESTWALL]) != 0)
		{
			move = (CONVERT_TILE_TO_GLOBAL(ob->tileright)-1)-ob->right;
			MoveObjHoriz(ob, move);
			return;
		}
	}
}

/*
===========================
=
= CheckPosition
=
===========================
*/

boolean CheckPosition(objtype *ob)
{
	Uint16 y;
	Uint16 id0_far *map;
	Uint16 tile, x, rowdiff;
	
	map = (Uint16 id0_far *)(mapsegs[1]) + mapbwidthtable[ob->tiletop]/2 + ob->tileleft;
	rowdiff = mapwidth-(ob->tileright-ob->tileleft+1);
	for (y=ob->tiletop; y<=ob->tilebottom; y++,map+=rowdiff)
	{
		for (x=ob->tileleft; x<=ob->tileright; x++)
		{
			tile = *(map++);
			if (tinf[tile+NORTHWALL] || tinf[tile+EASTWALL] || tinf[tile+SOUTHWALL] || tinf[tile+WESTWALL])
			{
				return false;
			}
		}
	}
	return true;
}

/*
===========================
=
= StatePositionOk
=
===========================
*/

boolean StatePositionOk(objtype *ob, FARSTATE *state)
{
	spritetabletype id0_far *shape;

	if (ob->xdir > 0)
	{
		ob->shapenum = state->rightshapenum;
	}
	else
	{
		ob->shapenum = state->leftshapenum;
	}
	shape = &spritetable[ob->shapenum-STARTSPRITES];
	ob->left = ob->x + shape->xl;
	ob->right = ob->x + shape->xh;
	ob->top = ob->y + shape->yl;
	ob->bottom = ob->y + shape->yh;
	ob->midx = ob->left + (ob->right-ob->left)/2;
	ob->tileleft = CONVERT_GLOBAL_TO_TILE(ob->left);
	ob->tileright = CONVERT_GLOBAL_TO_TILE(ob->right);
	ob->tiletop = CONVERT_GLOBAL_TO_TILE(ob->top);
	ob->tilebottom = CONVERT_GLOBAL_TO_TILE(ob->bottom);
	ob->tilemidx = CONVERT_GLOBAL_TO_TILE(ob->midx);
	return CheckPosition(ob);
}

/*
===========================
=
= CalcBounds	(never used in BioMenace)
=
===========================
*/

static void CalcBounds(objtype *ob)
{
	spritetabletype id0_far *shape;

	shape = &spritetable[ob->shapenum-STARTSPRITES];
	ob->left = ob->x + shape->xl;
	ob->right = ob->x + shape->xh;
	ob->top = ob->y + shape->yl;
	ob->bottom = ob->y + shape->yh;
	ob->midx = ob->left + (ob->right-ob->left)/2;
}

//==========================================================================

/*
================
=
= ClipToWalls
=
= Moves the current object xtry/ytry units, clipping to walls
=
================
*/

static void ClipToWalls(objtype *ob)
{
	Uint16 oldx, oldy;
	spritetabletype id0_far *shape;
	boolean pushed;

	oldx = ob->x;
	oldy = ob->y;
	pushed = false;
//
// make sure it stays in contact with a 45 degree slope
//
	if (ob->state->pushtofloor == push_down)
	{
		if (ob->hitnorth == PLATFORMEDGE)
		{
			ytry = 145;
			//
			// Note: This code is not really required for BioMenace. If you go back
			// to the KEEN DREAMS source, you will see that it has no eqivalent to
			// the HandleRiding() routine. Instead, the ClipToSprite() code was
			// responsible for moving Keen sideways when he was standing on a
			// moving Canteloupe Cart in that game. That would make it necessary to
			// push Keen down into the other object if that object was moving up or
			// down, otherwise the objects would no longer touch each other and the
			// ClipToSprite() code would not be executed.
			//
			// Setting ytry to a relatively high value in here was probably just an
			// intermediate step in development, before the HandleRiding() routine
			// was implemented. But it might also have been intended to allow other
			// objects besides the player character to stand on top of another game
			// object.
			//
			// What IS important, however, is that pushed remains set to false in
			// this branch.
			// 
		}
		else
		{
			if (xtry > 0)
			{
				ytry = xtry+16;
			}
			else
			{
				ytry = -xtry+16;
			}
			pushed = true;
		}
	}
	else if (ob->state->pushtofloor == push_up)
	{
		if (xtry > 0)
		{
			ytry = -xtry-16;
		}
		else
		{
			ytry = xtry-16;
		}
		pushed = true;
	}
//
// move the shape
//
	if (xtry > 239)
	{
		xtry = 239;
	}
	else if (xtry < -239)
	{
		xtry = -239;
	}
	if (ytry > 255)			// +16 for push to floor
	{
		ytry = 255;
	}
	else if (ytry < -239)
	{
		ytry = -239;
	}
	ob->x += xtry;
	ob->y += ytry;
	ob->needtoreact = true;
	if (!ob->shapenum)				// can't get a hit rect with no shape!
	{
		return;
	}
	shape = &spritetable[ob->shapenum-STARTSPRITES];
	oldtileright = ob->tileright;
	oldtiletop = ob->tiletop;
	oldtileleft = ob->tileleft;
	oldtilebottom = ob->tilebottom;
	oldtilemidx = ob->tilemidx;
	oldright = ob->right;
	oldtop = ob->top;
	oldleft = ob->left;
	oldbottom = ob->bottom;
	oldmidx = ob->midx;
	ob->left = ob->x + shape->xl;
	ob->right = ob->x + shape->xh;
	ob->top = ob->y + shape->yl;
	ob->bottom = ob->y + shape->yh;
	ob->midx = ob->left + (ob->right-ob->left)/2;
	ob->tileleft = CONVERT_GLOBAL_TO_TILE(ob->left);
	ob->tileright = CONVERT_GLOBAL_TO_TILE(ob->right);
	ob->tiletop = CONVERT_GLOBAL_TO_TILE(ob->top);
	ob->tilebottom = CONVERT_GLOBAL_TO_TILE(ob->bottom);
	ob->tilemidx = CONVERT_GLOBAL_TO_TILE(ob->midx);
	ob->hitnorth=ob->hiteast=ob->hitsouth=ob->hitwest=0;
	if (ob->needtoclip)
	{
		leftmoved = ob->left - oldleft;
		rightmoved = ob->right - oldright;
		topmoved = ob->top - oldtop;
		bottommoved = ob->bottom - oldbottom;
		midxmoved = ob->midx - oldmidx;
	//
	// clip it
	//
		ClipToEnds(ob);
		if (ob == player && !playerkludgeclipcancel)	//Note: playerkludgeclipcancel is always false in BioMenace!
		{
			if (!ob->hitnorth && bottommoved > 0)
			{
				PlayerBottomKludge(ob);
			}
			if (!ob->hitsouth && topmoved < 0)
			{
				PlayerTopKludge(ob);
			}
		}
		ClipToSides(ob);
	}
//
// check if pushing to floor didn't work
//
	if (pushed && !ob->hitnorth)
	{
		// The object was pushed down to make it stick to the ground on 45 degree
		// slopes but now it's not touching the ground anymore. That means we
		// should undo the pushing down part (keep oldy and DON'T add ytry)!
		
		// BUG: The object might have been pushed up instead of down! This means
		// ceiling-based objects (the Gorps) can only walk on flat ceilings.
		ob->y = oldy;
		ob->x = oldx + xtry;
		ob->left = ob->x + shape->xl;
		ob->right = ob->x + shape->xh;
		ob->top = ob->y + shape->yl;
		ob->bottom = ob->y + shape->yh;
		ob->midx = ob->left + (ob->right-ob->left)/2;
		ob->tileleft = CONVERT_GLOBAL_TO_TILE(ob->left);
		ob->tileright = CONVERT_GLOBAL_TO_TILE(ob->right);
		ob->tiletop = CONVERT_GLOBAL_TO_TILE(ob->top);
		ob->tilebottom = CONVERT_GLOBAL_TO_TILE(ob->bottom);
		ob->tilemidx = CONVERT_GLOBAL_TO_TILE(ob->midx);
	}
	ob->xmove = ob->xmove + (ob->x - oldx);
	ob->ymove = ob->ymove + (ob->y - oldy);
}

/*
================
=
= FullClipToWalls
=
= Moves the current object xtry/ytry units, clipping to walls
=
= Full clipping means there is a hard-coded hit rectangle around the object and
= NO part of that hit rectangle is allowed to touch a tile that has ANY blocking
= walls. Slope tiles and one-way-blocking tiles will be considered blocking.
= The regular mid clipping allows the corners of the hit rectangle to be
= partially inside a wall, since only the midx point can actually touch the
= floor or the ceiling. It also means that objects will usually "slide off" the
= edge of a floor if only the leftmost or the rightmost part of the hit rect
= hits the floor. Full clipping objects will not slide off, but they might land
= above the floor if there are slope tiles in the way. Note that if a full
= clipping object already is at a position where it touches a tile with blocking
= walls, because it was spawned there or a blocking tile appeared because of a
= tile animation or when a bridge was activated, the object will most likely be
= stuck at that position (like when Eggbirds start to fly on/near slopes in
= Keen 4).
=
================
*/

static void FullClipToWalls(objtype *ob)
{
	Uint16 oldx, oldy, w, h;
	spritetabletype id0_far *shape;

	oldx = ob->x;
	oldy = ob->y;
//
// move the shape
//
	if (xtry > 239)
	{
		xtry = 239;
	}
	else if (xtry < -239)
	{
		xtry = -239;
	}
	if (ytry > 239)
	{
		ytry = 239;
	}
	else if (ytry < -239)
	{
		ytry = -239;
	}
	ob->x += xtry;
	ob->y += ytry;
	ob->needtoreact = true;
	shape = &spritetable[ob->shapenum-STARTSPRITES];
	/*
	Note: Only the crusher (fallingbrickobj), Snake's land mines (grenadeobj),
	the barrel and Skull Man's hands actually ever use full clipping. The other
	cases are unreachable in BioMenace.
	*/
	switch (ob->obclass)
	{
	case playerobj:
		w = 56*PIXGLOBAL;
		h = 24*PIXGLOBAL;
		break;
	case fallingbrickobj:
		w = 72*PIXGLOBAL;
		h = 56*PIXGLOBAL;
		break;
	case pushableobj:	// should also handle blockedobj, but both are never used here anyway
		w = 32*PIXGLOBAL;
		h = 40*PIXGLOBAL;
		break;
#ifdef BETA
	default:
		Quit("FullClipToWalls: Bad obclass");
#else
	case grenadeobj:
		w = 16*PIXGLOBAL;
		h = 16*PIXGLOBAL;
		break;
#if (EPISODE == 1)
	case skullhandobj:
		w = 40*PIXGLOBAL;
		h = 24*PIXGLOBAL;
		break;
#elif (EPISODE == 3)
	case barrelobj:
		w = 32*PIXGLOBAL;
		h = 32*PIXGLOBAL;
		break;
#endif
	default:
		w = 16*PIXGLOBAL;
		h = 16*PIXGLOBAL;
		break;
#endif
	}
	ob->right = ob->x + w;
	ob->left = ob->x;
	ob->top = ob->y;
	ob->bottom = ob->y + h;
	ob->tileleft = CONVERT_GLOBAL_TO_TILE(ob->left);
	ob->tileright = CONVERT_GLOBAL_TO_TILE(ob->right);
	ob->tiletop = CONVERT_GLOBAL_TO_TILE(ob->top);
	ob->tilebottom = CONVERT_GLOBAL_TO_TILE(ob->bottom);
	ob->hitnorth=ob->hiteast=ob->hitsouth=ob->hitwest=0;
//
// clip it
//
	if (!CheckPosition(ob))
	{
		MoveObjHoriz(ob, -xtry);	//undo x movement
		if (CheckPosition(ob))
		{
			if (xtry > 0)
			{
				ob->hitwest = 1;
			}
			else
			{
				ob->hiteast = 1;
			}
		}
		else
		{
			if (ytry > 0)
			{
				ob->hitnorth = 1;
			}
			else
			{
				ob->hitsouth = 1;
			}
			MoveObjHoriz(ob, xtry);	//redo x movement
			MoveObjVert(ob, -ytry);	//undo y movement
			if (!CheckPosition(ob))
			{
				MoveObjHoriz(ob, -xtry);	//undo x movement
				if (xtry > 0)
				{
					ob->hitwest = 1;
				}
				else
				{
					ob->hiteast = 1;
				}
			}
		}
	}
	ob->xmove = ob->xmove + (ob->x - oldx);
	ob->ymove = ob->ymove + (ob->y - oldy);
	ob->left = ob->x + shape->xl;
	ob->right = ob->x + shape->xh;
	ob->top = ob->y + shape->yl;
	ob->bottom = ob->y + shape->yh;
	ob->midx = ob->left + (ob->right-ob->left)/2;
}

/*
================
=
= PushObj
=
= Moves the current object xtry/ytry units, clipping to walls
= Used by ClipToSprite... routines and HandleRiding, doesn't limit movement and
= doesn't push to floor
=
================
*/

void PushObj(objtype *ob)
{
	Uint16 oldx, oldy;
	spritetabletype id0_far *shape;
	
	oldx = ob->x;
	oldy = ob->y;
	ob->x += xtry;
	ob->y += ytry;
	ob->needtoreact = true;
	if (!ob->shapenum)
	{
		return;
	}
	shape = &spritetable[ob->shapenum-STARTSPRITES];
	oldtileright = ob->tileright;
	oldtiletop = ob->tiletop;
	oldtileleft = ob->tileleft;
	oldtilebottom = ob->tilebottom;
	oldtilemidx = ob->tilemidx;
	oldright = ob->right;
	oldtop = ob->top;
	oldleft = ob->left;
	oldbottom = ob->bottom;
	oldmidx = ob->midx;
	ob->left = ob->x + shape->xl;
	ob->right = ob->x + shape->xh;
	ob->top = ob->y + shape->yl;
	ob->bottom = ob->y + shape->yh;
	ob->midx = ob->left + (ob->right-ob->left)/2;
	ob->tileleft = CONVERT_GLOBAL_TO_TILE(ob->left);
	ob->tileright = CONVERT_GLOBAL_TO_TILE(ob->right);
	ob->tiletop = CONVERT_GLOBAL_TO_TILE(ob->top);
	ob->tilebottom = CONVERT_GLOBAL_TO_TILE(ob->bottom);
	ob->tilemidx = CONVERT_GLOBAL_TO_TILE(ob->midx);
	if (ob->needtoclip)
	{
		leftmoved = ob->left - oldleft;
		rightmoved = ob->right - oldright;
		topmoved = ob->top - oldtop;
		bottommoved = ob->bottom - oldbottom;
		midxmoved = ob->midx - oldmidx;
		ClipToEnds(ob);
		ClipToSides(ob);
	}
	ob->xmove = ob->xmove + (ob->x - oldx);
	ob->ymove = ob->ymove + (ob->y - oldy);
}

//==========================================================================


/*
==================
=
= ClipToSpriteSide
=
= Clips push to solid
=
==================
*/

void ClipToSpriteSide(objtype *push, objtype *solid)
{
	Sint16 xmove, leftinto, rightinto;

	xmove = solid->xmove - push->xmove;
	leftinto = solid->right - push->left;
	rightinto = push->right - solid->left;
	if (leftinto > 0 && leftinto <= xmove)
	{
		xtry = leftinto;
		if (push->state->pushtofloor)
		{
			ytry = leftinto+16;
		}
		// BUG: ytry should be set to 0 otherwise
		ClipToWalls(push);
		push->hiteast = 1;
		return;
	}
	if (rightinto > 0 && rightinto <= -xmove)
	{
		xtry = -rightinto;
		if (push->state->pushtofloor)
		{
			ytry = rightinto+16;
		}
		// BUG: ytry should be set to 0 otherwise
		ClipToWalls(push);
		push->hitwest = 1;
		return;
	}
}

//==========================================================================


/*
==================
=
= ClipToSpriteTop
=
= Clips push to solid
=
==================
*/

void ClipToSpriteTop(objtype *push, objtype *solid)
{
	Sint16 /*temp, */ymove, bottominto;
	pushtype temp;

	ymove = push->ymove - solid->ymove;
	bottominto = push->bottom - solid->top;
	if (bottominto >= 0 && bottominto <= ymove)
	{
		if (push == player)
		{
			gamestate.riding = solid;
		}
		ytry = -bottominto;
		temp = push->state->pushtofloor;
		push->state->pushtofloor = push_none;
		//BUG: xtry should be set to 0 here
		ClipToWalls(push);
		push->state->pushtofloor = temp;
		if (!push->hitsouth)
		{
			push->hitnorth = PLATFORMEDGE;
		}
	}
}

//==========================================================================


/*
==================
=
= ClipToSprite
=
= Clips push to solid
=
==================
*/

void ClipToSprite(objtype *push, objtype *solid, boolean squish)
{
	Sint16 xmove, ymove, leftinto, rightinto, topinto, bottominto;
	
	xmove = solid->xmove - push->xmove;
	xtry = ytry = 0;
	
	//
	// left / right
	//
	leftinto = solid->right - push->left;
	rightinto = push->right - solid->left;
	if (leftinto > 0 && xmove+1 >= leftinto)
	{
		xtry = leftinto;
		push->xspeed = 0;
		PushObj(push);
		if (squish && push->hitwest)
		{
			SD_PlaySound(CHUNKSPLATSND);
			KillPlayer();
		}
		push->hiteast = 1;
		return;
	}
	else if (rightinto > 0 && -xmove+1 >= rightinto)
	{
		xtry = -rightinto;
		push->xspeed = 0;
		PushObj(push);
		if (squish && push->hiteast)
		{
			SD_PlaySound(CHUNKSPLATSND);
			KillPlayer();
		}
		push->hitwest = 1;
		return;
	}
	
	//
	// top / bottom
	//
	ymove = push->ymove - solid->ymove;
	topinto = solid->bottom - push->top;
	bottominto = push->bottom - solid->top;
	if (bottominto >= 0 && bottominto <= ymove)
	{
		if (push == player)
		{
			gamestate.riding = solid;
		}
		ytry = -bottominto;
		PushObj(push);
		if (squish && push->hitsouth)
		{
			SD_PlaySound(CHUNKSPLATSND);
			KillPlayer();
		}
		if (!push->hitsouth)
		{
			push->hitnorth = PLATFORMEDGE;
		}
	}
	else if (topinto >= 0 && -ymove >= topinto)
	{
		ytry = topinto;
		ClipToWalls(push);
		if (squish && push->hitnorth)
		{
			SD_PlaySound(CHUNKSPLATSND);
			KillPlayer();
		}
		push->hitsouth = PLATFORMEDGE;
	}
}

//==========================================================================


/*
==================
=
= ClipPushSprite
=
= Clips push to solid
=
==================
*/

void ClipPushSprite(objtype *push, objtype *solid, boolean squish)
{
	Sint16 xmove, ymove, leftinto, rightinto, topinto, bottominto, amount;
	
	//
	// can't push solid if it's already blocked on the other side
	//
	if ((solid->hiteast && push->xdir == -1) || (solid->hitwest && push->xdir == 1))
	{
		ClipToSprite(push, solid, squish);
		return;
	}
	
	// Note: push will be the player, solid will be the pushable block, therefore
	// solid->xmove will usually be 0 and xmove will be the value that would need
	// to be added to push->x to undo its last x-movement. If solid->xmove is NOT
	// 0, then xmove is the value that would need to be added to push->x to make
	// it keep the same distance to solid that it had before the objects moved.
	xmove = solid->xmove - push->xmove;
	xtry = ytry = 0;
	
	//
	// top / bottom
	//
	ymove = push->ymove - solid->ymove;
	topinto = solid->bottom - push->top;
	bottominto = push->bottom - solid->top;
	if (bottominto >= 0 && bottominto <= ymove)
	{
		if (push == player)
		{
			gamestate.riding = solid;
		}
		ytry = -bottominto;
		PushObj(push);
		
		// Note: squish is always false when this function is called in BioMenace
		if (squish && push->hitsouth)
		{
			KillPlayer();
		}
		if (!push->hitsouth)
		{
			push->hitnorth = PLATFORMEDGE;
		}
		return;
	}
	else if (topinto >= 0 && -ymove >= topinto)
	{
		ytry = topinto;
		ClipToWalls(push);
		
		// Note: squish is always false when this function is called in BioMenace
		if (squish && push->hitnorth)
		{
			KillPlayer();
		}
		push->hitsouth = PLATFORMEDGE;
		return;
	}
	
	//
	// left / right
	//
	leftinto = solid->right - push->left;
	rightinto = push->right - solid->left;
	if (leftinto > 0 && push->left > solid->left)
	{
		// The left side of push's hit rect is inside solid's hit rect in this
		// branch, which means xmove will be positive (and >= leftinto) here
		// unless push couldn't get moved far enough out of solid's hit rect the
		// last time this code was executed.
		amount = (leftinto-xmove)+1;
		if (amount < 0)
		{
			amount = 0;
		}
		// amount will most likely be 0 at this point and definitely cannot be
		// negative, which means the push object will be moved at least twice as
		// far back as the solid object gets pushed in the other direction. This
		// might have been implemented to make sure the player cannot get too
		// close to the pushable block. Since the player can stand on the pushable
		// block, the block's hit rect had to be a trade-off between which pixels
		// the player was supposed to be able to stand on and how far the player
		// would be able to walk "into" the block from the sides. But it might
		// also have been designed like this to make the player walk a lot slower
		// than normal when pushing a block around.
		push->xspeed = 0;
		xtry = leftinto+amount;
		PushObj(push);
		xtry = -leftinto/2;
		PushObj(solid);
		
		// Note: squish is always false when this function is called in BioMenace
		if (squish && push->hitwest)
		{
			KillPlayer();
		}
		// Note that this does NOT set push->hiteast to 1.
		return;
	}
	else if (rightinto > 0 && push->right < solid->right)
	{
		// The right side of push's hit rect is inside solid's hit rect in this
		// branch, which means xmove will be negative (and <= -rightinto) here
		// unless push couldn't get moved far enough out of solid's hit rect the
		// last time this code was executed.
		amount = rightinto+xmove+1;
		if (amount < 0)
		{
			amount = 0;
		}
		// amount will most likely be 0 at this point, same as above
		push->xspeed = 0;
		xtry = -rightinto - amount;
		PushObj(push);
		xtry = rightinto/2;
		PushObj(solid);
		
		// Note: squish is always false when this function is called in BioMenace
		if (squish && push->hiteast)
		{
			KillPlayer();
		}
		// Note that this does NOT set push->hitwest to 1.
		return;
	}
}

//==========================================================================


/*
==================
=
= DoActor
=
= Moves an actor in its current state by a given number of tics.
= If that time takes it into the next state, it changes the state
= and returns the number of excess tics after the state change
=
==================
*/

static Sint16 DoActor(objtype *ob, Sint16 numtics)
{
	Sint16 ticcount, usedtics, excesstics;
	FARSTATE *state;
	
	state = ob->state;
	// REFKEEN: Vanilla BioMenace bug emulation (state may be 0).
	//
	// If state == NULL then, under DOS (with Borland C++ 2.0-3.1),
	// state->progress is a part of a small Borland C++ string.
	//
	// As commented in StateMachine, this may actually cause a lock-up in
	// BioMenace on vintage DOS machines, due to states being in far memory.
	if (state && state->progress == think)
//	if (state->progress == think)
	{
		if (state->thinkptr)
		{
			if (ob->nothink)
			{
				ob->nothink--;
			}
			else
			{
				state->thinkptr(ob);
			}
		}
		return 0;
	}
	ticcount = ob->ticcount+numtics;
	// REFKEEN: Vanilla BioMenace bug emulation (state may be 0)
	// Magic number is based on what would occur if there were a
	// null-pointer dereference, assuming the value was never overwritten.
	//
	// As commented in StateMachine, this may actually cause a lock-up in
	// BioMenace on vintage DOS machines, due to states being in far memory.
	if ((!state && ticcount < 0x2064) || (state->tictime > ticcount || state->tictime == 0))
//	if (state->tictime > ticcount || state->tictime == 0)
	{
		ob->ticcount = ticcount;
		// REFKEEN: Vanilla BioMenace bug (state may be 0)
		// state->progress would differ from all compared values with
		// the original DOS versions in that case, unless modified.
		if (!state)
			return 0;
		if (state->progress == slide || state->progress == slidethink)
		{
			if (ob->xdir)
			{
				xtry += ob->xdir == 1? numtics*state->xmove : -numtics*state->xmove;
			}
			if (ob->ydir)
			{
				ytry += ob->ydir == 1? numtics*state->ymove : -numtics*state->ymove;
			}
		}
		if ((state->progress == slidethink || state->progress == stepthink) && state->thinkptr)
		{
			if (ob->nothink)
			{
				ob->nothink--;
			}
			else
			{
				state->thinkptr(ob);
			}
		}
		return 0;
	}
	else
	{
		usedtics = state->tictime - ob->ticcount;
		excesstics = ticcount - state->tictime;
		ob->ticcount = 0;
		if (state->progress == slide || state->progress == slidethink)
		{
			if (ob->xdir)
			{
				xtry += ob->xdir == 1? usedtics*state->xmove : -usedtics*state->xmove;
			}
			if (ob->ydir)
			{
				ytry += ob->ydir == 1? usedtics*state->ymove : -usedtics*state->ymove;
			}
		}
		else
		{
			if (ob->xdir)
			{
				xtry += ob->xdir == 1? state->xmove : -state->xmove;
			}
			if (ob->ydir)
			{
				ytry += ob->ydir == 1? state->ymove : -state->ymove;
			}
		}
		if (state->thinkptr)
		{
			if (ob->nothink)
			{
				ob->nothink--;
			}
			else
			{
				state->thinkptr(ob);
			}
		}
		if (state == ob->state)
		{
			ob->state = state->nextstate;
		}
		else if (!ob->state)
		{
			return 0;
		}
		return excesstics;
	}
}

//==========================================================================


/*
====================
=
= StateMachine
=
= Change state and give directions
=
====================
*/

void StateMachine(objtype *ob)
{
	Sint16 excesstics, oldshapenum;
	FARSTATE *state;
	
	ob->xmove=ob->ymove=xtry=ytry=0;
	oldshapenum = ob->shapenum;
	state = ob->state;
	excesstics = DoActor(ob, tics);
	if (ob->state != state)
	{
		ob->ticcount = 0;		// start the new state at 0, then use excess
		state = ob->state;
	}
	while (excesstics)
	{
	//
	// passed through to next state
	//
		// REFKEEN: Vanilla BioMenace bug fix (state may be 0).
		// state->skippable would be true with the original EXEs if state
		// was 0, unless modified earlier.
		// Side-note: Inherited from the Keens, but not originally
		// reproduced in them with the original DOS executables.
		// That could change at times in BM with the migration to 32-bit
		// far state pointers, e.g., upon shooting somewhere.
		if (state && !state->skippable && state->tictime <= excesstics)
//		if (!state->skippable && state->tictime <= excesstics)
		{
			excesstics = DoActor(ob, state->tictime-1);
		}
		else
		{
			excesstics = DoActor(ob, excesstics);
		}
		if (ob->state != state)
		{
			ob->ticcount = 0;		// start the new state at 0, then use excess
			state = ob->state;
		}
	}
	
	if (!state)			// object removed itself
	{
		RemoveObj(ob);
		return;
	}
	
	//
	// if state->rightshapenum == NULL, the state does not have a standard
	// shape (the think routine should have set it)
	//
	if (state->rightshapenum)
	{
		if (ob->xdir > 0)
		{
			ob->shapenum = state->rightshapenum;
		}
		else
		{
			ob->shapenum = state->leftshapenum;
		}
	}
	if ((Sint16)ob->shapenum == -1)
	{
		ob->shapenum = 0;		// make it invisable this time
	}
	if (xtry != 0 || ytry != 0 || ob->shapenum != oldshapenum || ob->hitnorth == PLATFORMEDGE)
	{
	//
	// actor moved or changed shape
	// make sure the movement is within limits (one tile)
	//
		if (ob->needtoclip == cl_fullclip)
		{
			FullClipToWalls(ob);
		}
		else
		{
			ClipToWalls(ob);
		}
	}
}

//==========================================================================


/*
====================
=
= NewState
=
====================
*/

void NewState(objtype *ob, FARSTATE *state)
{
	cliptype oldclip;
	
	ob->state = state;
	if (state->rightshapenum)
	{
		if (ob->xdir > 0)
		{
			ob->shapenum = state->rightshapenum;
		}
		else
		{
			ob->shapenum = state->leftshapenum;
		}
	}
	if ((Sint16)ob->shapenum == -1)
	{
		ob->shapenum = 0;
	}
	
	oldclip = ob->needtoclip;
	ob->needtoclip = cl_noclip;
	xtry=ytry=0;					// no movement
	ClipToWalls(ob);					// just calculate values
	ob->needtoclip = oldclip;
	
	if (ob->needtoclip == cl_fullclip)
	{
		FullClipToWalls(ob);
	}
	else if (ob->needtoclip == cl_midclip)
	{
		ClipToWalls(ob);
	}
}

//==========================================================================


/*
====================
=
= ChangeState
=
====================
*/

void ChangeState(objtype *ob, FARSTATE *state)
{
	ob->state = state;
	ob->ticcount = 0;
	if (state->rightshapenum)
	{
		if (ob->xdir > 0)
		{
			ob->shapenum = state->rightshapenum;
		}
		else
		{
			ob->shapenum = state->leftshapenum;
		}
	}
	if ((Sint16)ob->shapenum == -1)
	{
		ob->shapenum = 0;
	}
	ob->needtoreact = true;			// it will need to be redrawn this frame
	xtry=ytry=0;					// no movement
	if (ob->hitnorth != PLATFORMEDGE)
	{
		ClipToWalls(ob);
	}
}

//==========================================================================


/*
====================
=
= OnScreen
=
====================
*/

boolean OnScreen(objtype *ob)
{
	if (ob->tileright < originxtile || ob->tilebottom < originytile
		|| ob->tileleft > originxtilemax || ob->tiletop > originytilemax)
	{
		return false;
	}
	return true;
}

//==========================================================================


/*
====================
=
= DoGravity
=
====================
*/

void DoGravity(objtype *ob)
{
	Sint32 i;
//
// only accelerate on odd tics, because of limited precision
//
	for (i = lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i&1)
		{
			if (ob->yspeed < 0 && ob->yspeed >= -4)
			{
				ytry += ob->yspeed;
				ob->yspeed = 0;
				return;
			}
			ob->yspeed += 4;
			if (ob->yspeed > 70)
			{
				ob->yspeed = 70;
			}
		}
		ytry += ob->yspeed;
	}
}

//==========================================================================


/*
====================
=
= DoWeakGravity	(never actually used in BioMenace)
=
====================
*/

static void DoWeakGravity(objtype *ob)
{
	Sint32 i;
//
// only accelerate on odd tics, because of limited precision
//
	for (i = lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i&1)
		{
			if (ob->yspeed < 0 && ob->yspeed >= -3)
			{
				ytry += ob->yspeed;
				ob->yspeed = 0;
				return;
			}
			ob->yspeed += 3;
			if (ob->yspeed > 70)
			{
				ob->yspeed = 70;
			}
		}
		ytry += ob->yspeed;
	}
}

//==========================================================================


/*
====================
=
= DoTinyGravity	(never actually used in BioMenace)
=
====================
*/

static void DoTinyGravity(objtype *ob)
{
	Sint32 i;
//
// only accelerate every 4 tics, because of limited precision
//
	for (i = lasttimecount-tics; i<lasttimecount; i++)
	{
		if (!i&3)	// BUG? this means "if ((!i) & 3)", not "if (!(i & 3))"
		{
			ob->yspeed++;
			if (ob->yspeed > 70)
			{
				ob->yspeed = 70;
			}
		}
		ytry += ob->yspeed;
	}
}

//==========================================================================


/*
===============
=
= AccelerateX
=
===============
*/

void AccelerateX(objtype *ob, Sint16 dir, Sint16 maxspeed)
{
	Sint32 i;
	Uint16 oldsign;
	
	oldsign = ob->xspeed & 0x8000;
//
// only accelerate on odd tics, because of limited precision
//
	for (i=lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i & 1)
		{
			ob->xspeed += dir;
			if ((ob->xspeed & 0x8000) != oldsign)
			{
				oldsign = ob->xspeed & 0x8000;
				ob->xdir = oldsign? -1 : 1;
			}
			if (ob->xspeed > maxspeed)
			{
				ob->xspeed = maxspeed;
			}
			else if (ob->xspeed < -maxspeed)
			{
				ob->xspeed = -maxspeed;
			}
		}
		xtry += ob->xspeed;
	}
}

/*
===============
=
= AccelerateXv	(never used in BioMenace)
=
= Doesn't change object's xdir
=
===============
*/

static void AccelerateXv(objtype *ob, Sint16 dir, Sint16 maxspeed)
{
	Sint32 i;

//
// only accelerate on odd tics, because of limited precision
//
	for (i=lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i & 1)
		{
			ob->xspeed += dir;
			if (ob->xspeed > maxspeed)
			{
				ob->xspeed = maxspeed;
			}
			else if (ob->xspeed < -maxspeed)
			{
				ob->xspeed = -maxspeed;
			}
		}
		xtry += ob->xspeed;
	}
}

/*
===============
=
= AccelerateY
=
===============
*/

void AccelerateY(objtype *ob, Sint16 dir, Sint16 maxspeed)
{
	Sint32 i;

//
// only accelerate on odd tics, because of limited precision
//
	for (i=lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i & 1)
		{
			ob->yspeed += dir;
			if (ob->yspeed > maxspeed)
			{
				ob->yspeed = maxspeed;
			}
			else if (ob->yspeed < -maxspeed)
			{
				ob->yspeed = -maxspeed;
			}
		}
		ytry += ob->yspeed;
	}
}

/*
===============
=
= FrictionX
=
===============
*/

void FrictionX(objtype *ob)
{
	Sint16 friction, oldsign;
	Sint32 i;

	oldsign = ob->xspeed & 0x8000;
	if (ob->xspeed > 0)
	{
		friction = -1;
	}
	else if (ob->xspeed < 0)
	{
		friction = 1;
	}
	else
	{
		friction = 0;
	}
//
// only accelerate on odd tics, because of limited precision
//
	for (i=lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i & 1)
		{
			ob->xspeed += friction;
			if ((ob->xspeed & 0x8000) != oldsign)
			{
				ob->xspeed = 0;
			}
		}
		xtry += ob->xspeed;
	}
}

/*
===============
=
= FrictionY	(never used in BioMenace)
=
===============
*/

static void FrictionY(objtype *ob)
{
	Sint16 friction, oldsign;
	Sint32 i;

	if (ob->yspeed > 0)
	{
		friction = -1;
	}
	else if (ob->yspeed < 0)
	{
		friction = 1;
	}
	else
	{
		friction = 0;
	}
//
// only accelerate on odd tics, because of limited precision
//
	for (i=lasttimecount-tics; i<lasttimecount; i++)
	{
		if (i & 1)
		{
			ob->yspeed += friction;
			if ((ob->yspeed & 0x8000) != oldsign)	//BUG: oldsign is not initialized!
			{
				ob->yspeed = 0;
			}
		}
		ytry += ob->yspeed;
	}
}

//==========================================================================

/*
===============
=
= ProjectileThink
=
===============
*/

void ProjectileThink(objtype *ob)
{
	DoGravity(ob);
	xtry = ob->xspeed*tics;
}

/*
===============
=
= WeakProjectileThink	(never used in BioMenace)
=
===============
*/

static void WeakProjectileThink(objtype *ob)
{
	DoWeakGravity(ob);
	xtry = ob->xspeed*tics;
}

/*
===============
=
= TinyProjectileThink	(never used in BioMenace)
=
===============
*/

static void TinyProjectileThink(objtype *ob)
{
	DoTinyGravity(ob);
	xtry = ob->xspeed*tics;
}

/*
===============
=
= VelocityThink
=
===============
*/

void VelocityThink(objtype *ob)
{
	xtry = ob->xspeed*tics;
	ytry = ob->yspeed*tics;
}

/*
===============
=
= LethalContact	(never used in BioMenace)
=
===============
*/

//#pragma argsused
static void LethalContact(objtype *ob, objtype *hit)
{
	if (hit == player)
	{
		KillPlayer();
	}
}

/*
===============
=
= DrawReact
=
===============
*/

void DrawReact(objtype *ob)
{
	PLACESPRITE;
}

/*
===============
=
= WalkReact
=
===============
*/

void WalkReact(objtype *ob)
{
#ifdef BETA
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
	else if (!ob->hitnorth)
	{
		ob->x -= ob->xmove*2;
		ob->xdir = -ob->xdir;
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
	}
#else
	if (ob->xdir == 1 && ob->hitwest)
	{
		ob->x -= ob->xmove;
		if (ob->temp7 == 0)
		{
			ob->xdir = -1;
		}
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
		if (ob->temp7 == 0)
		{
			ob->temp7 = 10;
		}
	}
	else if (ob->xdir == -1 && ob->hiteast)
	{
		ob->x -= ob->xmove;
		if (ob->temp7 == 0)
		{
			ob->xdir = 1;
		}
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
		if (ob->temp7 == 0)
		{
			ob->temp7 = 10;
		}
	}
	else if (!ob->hitnorth)
	{
		if (ob->temp7 == 0)
		{
			ob->xdir = -ob->xdir;
			ChangeState(ob, ob->state);
		}
		ob->x -= ob->xmove*2;
		ob->nothink = US_RndT() >> 3;
		if (ob->temp7 == 0)
		{
			ob->temp7 = 10;
		}
	}
	if (ob->temp7 > 0)
	{
		ob->temp7--;
	}
#endif
	PLACESPRITE;
}

/*
===============
=
= R_WalkNormal	(never used in BioMenace)
=
= Actor will not walk onto tiles with special (e.g. deadly) north walls
= (BioMenace doesn't have any tiles with special north walls)
=
===============
*/

static void R_WalkNormal(objtype *ob)
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
	else if (!ob->hitnorth || ob->hitnorth & ~SLOPEMASK)
	{
		ob->x -= ob->xmove*2;
		ob->xdir = -ob->xdir;
		ob->nothink = US_RndT() >> 5;
		ChangeState(ob, ob->state);
	}
	PLACESPRITE;
}

#if 0 // REFKEEN: Unused
/*
===============
=
= BadState
=
===============
*/

//#pragma argsused
void BadState(objtype *ob)
{
	Quit("Object with bad state!");
}
#endif

REFKEEN_NS_E
