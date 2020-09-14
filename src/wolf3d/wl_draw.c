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

// WL_DRAW.C

#include "wl_def.h"
//#include <DOS.H>
//#pragma hdrstop

//#define DEBUGWALLS
//#define DEBUGTICS

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

// *** S3DNA RESTORATION ***
// This seems to be required for S3DNA
#ifdef GAMEVER_NOAH3D
#define DOORWALL(x)	((PMSpriteStart-(x))-8)
#else
// the door is the last picture before the sprites
#define DOORWALL	(PMSpriteStart-8)
#endif

#define ACTORSIZE	0x4000

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
// Not sure how were these values picked, but here they are
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#ifdef DEBUGWALLS
id0_unsigned_t screenloc[3]= {0,0,0};
#else
id0_unsigned_t screenloc[3]= {3328,16128,28928};
#endif
id0_unsigned_t freelatch = 41728;
#else
#ifdef DEBUGWALLS
id0_unsigned_t screenloc[3]= {0,0,0};
#else
id0_unsigned_t screenloc[3]= {PAGE1START,PAGE2START,PAGE3START};
#endif
id0_unsigned_t freelatch = FREESTART;
#endif

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
id0_int_t 	screenpage;
#endif

id0_long_t 	lasttimecount;
id0_long_t 	frameon;

id0_unsigned_t	wallheight[MAXVIEWWIDTH];

fixed	tileglobal	= TILEGLOBAL;
fixed	mindist		= MINDIST;


//
// math tables
//
id0_int_t			pixelangle[MAXVIEWWIDTH];
id0_long_t		id0_far finetangent[FINEANGLES/4];
fixed 		id0_far sintable[ANGLES+ANGLES/4],id0_far *costable = sintable+(ANGLES/4);

//
// refresh variables
//
fixed	viewx,viewy;			// the focal point
id0_int_t		viewangle;
fixed	viewsin,viewcos;



fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
id0_int_t		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);



//
// wall optimization variables
//
id0_int_t		lastside;		// true for vertical
id0_long_t	lastintercept;
id0_int_t		lasttilehit;

// *** ALPHA RESTORATION *** - A couple of unused variables
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
id0_int_t	someUnusedDrawVar1, someUnusedDrawVar2;
#endif

//
// ray tracing variables
//
id0_int_t			focaltx,focalty,viewtx,viewty;

id0_int_t			midangle,angle;
id0_unsigned_t	xpartial,ypartial;
id0_unsigned_t	xpartialup,xpartialdown,ypartialup,ypartialdown;
id0_unsigned_t	xinttile,yinttile;

id0_unsigned_t	tilehit;
id0_unsigned_t	pixx;

id0_int_t		xtile,ytile;
id0_int_t		xtilestep,ytilestep;
id0_long_t	xintercept,yintercept;
id0_long_t	xstep,ystep;

id0_int_t		horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


void AsmRefresh (void);			// in WL_DR_A.ASM

/*
============================================================================

			   3 - D  DEFINITIONS

============================================================================
*/


//==========================================================================


/*
========================
=
= FixedByFrac
=
= multiply a 16/16 bit, 2's complement fixed point number by a 16 bit
= fraction, passed as a signed magnitude 32 bit number
=
========================
*/

//#pragma warn -rvl			// I stick the return value in with ASMs

fixed FixedByFrac (fixed a, fixed b)
{
	int result_sign = (b < 0) ? -1 : 1; // sign of result == sign of fraction
	id0_longword_t a_as_unsigned = a;
	if (a < 0) // negative?
	{
		//2's complement...
		a_as_unsigned = -a;
		//a_as_unsigned ^= -1;
		//++a_as_unsigned;
		result_sign *= -1; // toggle sign of result
	}
	//
	// Multiply a_as_unsigned by the low 8 bits of b
	//
	id0_word_t b_lo = b&0xFFFF;
	id0_longword_t result = b_lo*(a_as_unsigned>>16) + ((b_lo*(a_as_unsigned&0xFFFF))>>16);
	//id0_longword_t result = b_lo*(a_as_unsigned>>16) + b_lo*(a_as_unsigned&0xFFFF);
	//
	// put result in 2's complement
	//
	if (result_sign < 0) // Is the result negative?
	{
		//2's complement...
		result = -result;
		//result ^= -1;
		//++result;
	}
	return result;
#if 0
//
// setup
//
asm	mov	si,[WORD PTR b+2]	// sign of result = sign of fraction

asm	mov	ax,[WORD PTR a]
asm	mov	cx,[WORD PTR a+2]

asm	or	cx,cx
asm	jns	aok:				// negative?
asm	neg	cx
asm	neg	ax
asm	sbb	cx,0
asm	xor	si,0x8000			// toggle sign of result
aok:

//
// multiply  cx:ax by bx
//
asm	mov	bx,[WORD PTR b]
asm	mul	bx					// fraction*fraction
asm	mov	di,dx				// di is low word of result
asm	mov	ax,cx				//
asm	mul	bx					// units*fraction
asm add	ax,di
asm	adc	dx,0

//
// put result dx:ax in 2's complement
//
asm	test	si,0x8000		// is the result negative?
asm	jz	ansok:
asm	neg	dx
asm	neg	ax
asm	sbb	dx,0

ansok:;
#endif
}

//#pragma warn +rvl

//==========================================================================

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy		: globalx/globaly of point
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale		: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/


//
// transform actor
//
void TransformActor (objtype *ob)
{
	id0_int_t ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	id0_long_t	temp;

//
// translate point to view centered coordinates
//
	gx = ob->x-viewx;
	gy = ob->y-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-ACTORSIZE;		// fudge the shape forward a bit, because
								// the midpoint could put parts of the shape
								// into an adjacent wall

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;

//
// calculate perspective ratio
//
	ob->transx = nx;
	ob->transy = ny;

	if (nx<mindist)			// too close, don't overflow the divide
	{
	  ob->viewheight = 0;
	  return;
	}

	ob->viewx = centerx + ny*scale/nx;	// DEBUG: use assembly divide

//
// calculate height (heightnumerator/(nx>>8))
//
	temp = heightnumerator / ((nx>>8)&0xFFFF);
#if 0
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx
#endif

	ob->viewheight = temp;
}

//==========================================================================

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty		: tile the object is centered in
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale		: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
========================
*/

id0_boolean_t TransformTile (id0_int_t tx, id0_int_t ty, id0_int_t *dispx, id0_int_t *dispheight)
{
	id0_int_t ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	id0_long_t	temp;

//
// translate point to view centered coordinates
//
	gx = ((id0_long_t)tx<<TILESHIFT)+0x8000-viewx;
	gy = ((id0_long_t)ty<<TILESHIFT)+0x8000-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-0x2000;		// 0x2000 is size of object

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;


//
// calculate perspective ratio
//
	if (nx<mindist)			// too close, don't overflow the divide
	{
		*dispheight = 0;
		return false;
	}

	*dispx = centerx + ny*scale/nx;	// DEBUG: use assembly divide

//
// calculate height (heightnumerator/(nx>>8))
//
	temp = heightnumerator / ((nx>>8)&0xFFFF);
#if 0
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx
#endif

	*dispheight = temp;

//
// see if it should be grabbed
//
	if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
		return true;
	else
		return false;
}

//==========================================================================

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/

//#pragma warn -rvl			// I stick the return value in with ASMs

id0_int_t	CalcHeight (void)
{
	id0_int_t	transheight;
	id0_int_t ratio;
	fixed gxt,gyt,nx,ny;
	id0_long_t	gx,gy;

	gx = xintercept-viewx;
	gxt = FixedByFrac(gx,viewcos);

	gy = yintercept-viewy;
	gyt = FixedByFrac(gy,viewsin);

	nx = gxt-gyt;

  //
  // calculate perspective ratio (heightnumerator/(nx>>8))
  //
	if (nx<mindist)
		nx=mindist;			// don't let divide overflow

	return heightnumerator / ((nx>>8)&0xFFFF);
#if 0
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]			// nx>>8
#endif
}


//==========================================================================

/*
===================
=
= ScalePost
=
===================
*/

id0_byte_t		*postsource;
id0_unsigned_t		postsourceoff;
//id0_long_t		postsource;
id0_unsigned_t	postx;
id0_unsigned_t	postwidth;

void	id0_near ScalePost (void)		// VGA version
{
	// fractional height (low 3 bits fraction)
	// h = heightscaler*4
	id0_unsigned_t h = (wallheight[postx] & 0xfff8) >> 1;

	h = BE_Cross_TypedMin(id0_int_t, h, maxscaleshl2);
	id0_byte_t *linescale = (id0_byte_t *)fullscalefarcall[h/4];
//	id0_byte_t *linescale = (id0_byte_t *)(fullscalefarcall[h/sizeof(*fullscalefarcall)]);
	if (!linescale)
	{
		void BadScale(void);
		BadScale();
	}

	//
	// scale a byte wide strip of wall
	//
	id0_unsigned_t destoff = (postx >> 2) + bufferofs;

	ExecuteCompScale(linescale, destoff, postsource + postsourceoff, mapmasks1[postx&3][postwidth-1]);
	if (mapmasks2[postx&3][postwidth-1])
	{
		ExecuteCompScale(linescale, destoff + 1, postsource + postsourceoff, mapmasks2[postx&3][postwidth-1]);
		if (mapmasks3[postx&3][postwidth-1])
			ExecuteCompScale(linescale, destoff + 2, postsource + postsourceoff, mapmasks2[postx&3][postwidth-1]);
	}
#if 0
	asm	mov	ax,SCREENSEG
	asm	mov	es,ax

	asm	mov	bx,[postx]
	asm	shl	bx,1
	asm	mov	bp,WORD PTR [wallheight+bx]		// fractional height (low 3 bits frac)
	asm	and	bp,0xfff8				// bp = heightscaler*4
	asm	shr	bp,1
	asm	cmp	bp,[maxscaleshl2]
	asm	jle	heightok
	asm	mov	bp,[maxscaleshl2]
heightok:
	asm	add	bp,OFFSET fullscalefarcall
	//
	// scale a byte wide strip of wall
	//
	asm	mov	bx,[postx]
	asm	mov	di,bx
	asm	shr	di,2						// X in bytes
	asm	add	di,[bufferofs]

	asm	and	bx,3
	asm	shl	bx,3						// bx = pixel*8+pixwidth
	asm	add	bx,[postwidth]

	asm	mov	al,BYTE PTR [mapmasks1-1+bx]	// -1 because no widths of 0
	asm	mov	dx,SC_INDEX+1
	asm	out	dx,al						// set bit mask register
	asm	lds	si,DWORD PTR [postsource]
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	al,BYTE PTR [ss:mapmasks2-1+bx]   // -1 because no widths of 0
	asm	or	al,al
	asm	jz	nomore

	//
	// draw a second byte for vertical strips that cross two bytes
	//
	asm	inc	di
	asm	out	dx,al						// set bit mask register
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	al,BYTE PTR [ss:mapmasks3-1+bx]	// -1 because no widths of 0
	asm	or	al,al
	asm	jz	nomore
	//
	// draw a third byte for vertical strips that cross three bytes
	//
	asm	inc	di
	asm	out	dx,al						// set bit mask register
	asm	call DWORD PTR [bp]				// scale the line of pixels


nomore:
	asm	mov	ax,ss
	asm	mov	ds,ax
#endif
}

void  FarScalePost (void)				// just so other files can call
{
	ScalePost ();
}

/*
====================
=
= HitVertWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

void HitVertWall (void)
{
	id0_int_t			wallpic;
	id0_unsigned_t	texture;

	texture = (yintercept>>4)&0xfc0;
	if (xtilestep == -1)
	{
		texture = 0xfc0-texture;
		xintercept += TILEGLOBAL;
	}
	wallheight[pixx] = CalcHeight();

	if (lastside==1 && lastintercept == xtile && lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		if (texture == postsourceoff/*(id0_unsigned_t)postsource*/)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			postsourceoff/*(id0_unsigned_t)postsource*/ = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lastside = true;
		lastintercept = xtile;

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		if (tilehit & 0x40)
		{								// check for adjacent doors
			ytile = yintercept>>TILESHIFT;
			if ( tilemap[xtile-xtilestep][ytile]&0x80 )
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				wallpic = DOORWALL(1);
#else
				wallpic = DOORWALL+3;
#endif
			else
				wallpic = vertwall[tilehit & ~0x40];
		}
		else
			wallpic = vertwall[tilehit];

		postsource = (id0_byte_t *)PM_GetPage(wallpic);
		postsourceoff = texture;
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(wallpic);
//		(id0_unsigned_t)postsource = texture;

	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	tilemap[xtile][ytile] |= 0x20;
#endif
}


/*
====================
=
= HitHorizWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

void HitHorizWall (void)
{
	id0_int_t			wallpic;
	id0_unsigned_t	texture;

	texture = (xintercept>>4)&0xfc0;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL;
	else
		texture = 0xfc0-texture;
	wallheight[pixx] = CalcHeight();

	if (lastside==0 && lastintercept == ytile && lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		if (texture == postsourceoff/*(id0_unsigned_t)postsource*/)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			postsourceoff/*(id0_unsigned_t)postsource*/ = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lastside = 0;
		lastintercept = ytile;

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		if (tilehit & 0x40)
		{								// check for adjacent doors
			xtile = xintercept>>TILESHIFT;
			if ( tilemap[xtile][ytile-ytilestep]&0x80 )
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				wallpic = DOORWALL(1);
#else
				wallpic = DOORWALL+2;
#endif
			else
				wallpic = horizwall[tilehit & ~0x40];
		}
		else
			wallpic = horizwall[tilehit];

		postsource = (id0_byte_t *)PM_GetPage(wallpic);
		postsourceoff = texture;
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(wallpic);
//		(id0_unsigned_t)postsource = texture;
	}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	tilemap[xtile][ytile] |= 0x20;
#endif
}

//==========================================================================

/*
====================
=
= HitHorizDoor
=
====================
*/

void HitHorizDoor (void)
{
	id0_unsigned_t	texture,doorpage,doornum;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	doornum = tilehit&0x1f;
	doorobjlist[doornum].seen = true;
#else
	doornum = tilehit&0x7f;
#endif
	texture = ( (xintercept-doorposition[doornum]) >> 4) &0xfc0;

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
	// in the same door as last time, so check for optimized draw
		if (texture == postsourceoff/*(id0_unsigned_t)postsource*/)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			postsourceoff/*(id0_unsigned_t)postsource*/ = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();			// draw last post
	// first pixel in this door
		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		switch (doorobjlist[doornum].lock)
		{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		case dr_normal:
			doorpage = DOORWALL(5);
			break;
		case dr_lock1:
			doorpage = DOORWALL(4);
			break;
		case dr_lock2:
			doorpage = DOORWALL(3);
			break;
		case dr_elevator:
			doorpage = DOORWALL(2);
			break;
#else
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_lock4:
			doorpage = DOORWALL+6;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
#endif
		}

		postsource = (id0_byte_t *)PM_GetPage(doorpage);
		postsourceoff = texture;
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(doorpage);
//		(id0_unsigned_t)postsource = texture;
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	tilemap[xtile][ytile] |= 0x20;
#endif
}

//==========================================================================

/*
====================
=
= HitVertDoor
=
====================
*/

void HitVertDoor (void)
{
	id0_unsigned_t	texture,doorpage,doornum;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	doornum = tilehit&0x1f;
	doorobjlist[doornum].seen = true;
#else
	doornum = tilehit&0x7f;
#endif
	texture = ( (yintercept-doorposition[doornum]) >> 4) &0xfc0;

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
	// in the same door as last time, so check for optimized draw
		if (texture == postsourceoff/*(id0_unsigned_t)postsource*/)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			postsourceoff/*(id0_unsigned_t)postsource*/ = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();			// draw last post
	// first pixel in this door
		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		switch (doorobjlist[doornum].lock)
		{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		case dr_normal:
			doorpage = DOORWALL(5);
			break;
		case dr_lock1:
			doorpage = DOORWALL(4);
			break;
		case dr_lock2:
			doorpage = DOORWALL(3);
			break;
		case dr_elevator:
			doorpage = DOORWALL(2);
			break;
#else
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_lock4:
			doorpage = DOORWALL+6;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
#endif
		}

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		postsource = (id0_byte_t *)PM_GetPage(doorpage);
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(doorpage);
#else
		postsource = (id0_byte_t *)PM_GetPage(doorpage+1);
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(doorpage+1);
#endif
		postsourceoff = texture;
//		(id0_unsigned_t)postsource = texture;
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	tilemap[xtile][ytile] |= 0x20;
#endif
}

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
//==========================================================================


/*
====================
=
= HitHorizPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitHorizPWall (void)
{
	id0_int_t			wallpic;
	id0_unsigned_t	texture,offset;

	texture = (xintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL-offset;
	else
	{
		texture = 0xfc0-texture;
		yintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		if (texture == postsourceoff/*(id0_unsigned_t)postsource*/)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			postsourceoff/*(id0_unsigned_t)postsource*/ = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = horizwall[tilehit&63];

		postsource = (id0_byte_t *)PM_GetPage(wallpic);
		postsourceoff = texture;
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(wallpic);
//		(id0_unsigned_t)postsource = texture;
	}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	tilemap[xtile][ytile] |= 0x20;
#endif
}


/*
====================
=
= HitVertPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitVertPWall (void)
{
	id0_int_t			wallpic;
	id0_unsigned_t	texture,offset;

	texture = (yintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (xtilestep == -1)
	{
		xintercept += TILEGLOBAL-offset;
		texture = 0xfc0-texture;
	}
	else
		xintercept += offset;

	wallheight[pixx] = CalcHeight();

	if (lasttilehit == tilehit)
	{
		// in the same wall type as last time, so check for optimized draw
		if (texture == postsourceoff/*(id0_unsigned_t)postsource*/)
		{
		// wide scale
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			postsourceoff/*(id0_unsigned_t)postsource*/ = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
	{
	// new wall
		if (lastside != -1)				// if not the first scaled post
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = vertwall[tilehit&63];

		postsource = (id0_byte_t *)PM_GetPage(wallpic);
		postsourceoff = texture;
//		*( ((id0_unsigned_t *)&postsource)+1) = (id0_unsigned_t)PM_GetPage(wallpic);
//		(id0_unsigned_t)postsource = texture;
	}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	tilemap[xtile][ytile] |= 0x20;
#endif
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

//==========================================================================

//==========================================================================

// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
// Re-enable unused EGA code, *and* restore egaFloor+egaCeiling (not in alpha)
#if 0 // REFKEEN: These are unused
//#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_unsigned_t egaFloor[] = {0,0,0,0,0,0,0,0,0,5};
id0_unsigned_t egaCeiling[] = {0x0808,0x0808,0x0808,0x0808,0x0808,0x0808,0x0808,0x0808,0x0808,0x0d0d};
#endif

//#if 0
/*
=====================
=
= ClearScreen
=
=====================
*/

void ClearScreen (void)
{
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
 id0_unsigned_t floor=egaFloor[gamestate.episode*10+mapon],
	  ceiling=egaCeiling[gamestate.episode*10+mapon];
#endif

  //
  // clear the screen
  //
asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*2		// read mode 0, write mode 2
asm	out	dx,ax
asm	mov	ax,GC_BITMASK + 255*256
asm	out	dx,ax

asm	mov	dx,40
asm	mov	ax,[viewwidth]
asm	shr	ax,3
asm	sub	dx,ax					// dx = 40-viewwidth/8

asm	mov	bx,[viewwidth]
asm	shr	bx,4					// bl = viewwidth/16
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height

#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	xor	ax,ax
#else
asm	mov	ax,[ceiling]
#endif
asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]

toploop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	mov	ax,0x0808
#else
asm	mov	ax,[floor]
#endif

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop


asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*10		// read mode 1, write mode 2
asm	out	dx,ax
asm	mov	al,GC_BITMASK
asm	out	dx,al

}
#endif
//==========================================================================

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_unsigned_t vgaCeiling[]=
{
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,
 0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,
 0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,
 0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2,0xd2d2
#elif (!defined SPEAR)
//#ifndef SPEAR
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xbfbf,
 0x4e4e,0x4e4e,0x4e4e,0x1d1d,0x8d8d,0x4e4e,0x1d1d,0x2d2d,0x1d1d,0x8d8d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x2d2d,0xdddd,0x1d1d,0x1d1d,//0x9898,
// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
 0x8d8d,
#else
 0x9898,
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
 0x1d1d,0x9d9d,0x2d2d,0xdddd,0xdddd,0x9d9d,0x2d2d,0x4d4d,0x1d1d,0xdddd,
 0x7d7d,0x1d1d,0x2d2d,0x2d2d,0xdddd,0xd7d7,0x1d1d,0x1d1d,0x1d1d,0x2d2d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xdddd,0xdddd,0x7d7d,0xdddd,0xdddd,0xdddd
#else
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xd7d7,0x1d1d,0x1d1d,0x1d1d,0x1d1d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
#else
 0x6f6f,0x4f4f,0x1d1d,0xdede,0xdfdf,0x2e2e,0x7f7f,0x9e9e,0xaeae,0x7f7f,
 0x1d1d,0xdede,0xdfdf,0xdede,0xdfdf,0xdede,0xe1e1,0xdcdc,0x2e2e,0x1d1d,0xdcdc
#endif
};
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

/*
=====================
=
= VGAClearScreen
=
=====================
*/

void VGAClearScreen (void)
{
 // *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 id0_unsigned_t ceiling=vgaCeiling[gamestate.mapon];
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
 id0_unsigned_t ceiling=vgaCeiling[gamestate.episode*10+mapon];
#endif
	// Ported from ASM. Screen updates were originally done with pairs
	// of equal bytes; Single bytes are used in the port. The destination
	// offset is updated as done here to match original behaviors.

	//
	// clear the screen
	//

	id0_unsigned_t destOff = bufferofs;
	// top loop
	for (int loopVar = (id0_byte_t)viewheight / 2; loopVar; --loopVar)
	{
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(destOff, 0x1d, (id0_byte_t)(viewwidth/8)*2);
#else
		BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(destOff, ceiling, (id0_byte_t)(viewwidth/8)*2);
#endif
		destOff += (id0_byte_t)(viewwidth/8)*2 + (80 - viewwidth/4);
	}
	// bottom loop
	for (int loopVar = (id0_byte_t)viewheight / 2; loopVar; --loopVar)
	{
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(destOff, 0xd9, (id0_byte_t)(viewwidth/8)*2);
#else
		BE_ST_VGAUpdateGFXBufferFrom8bitsPixel(destOff, 0x19, (id0_byte_t)(viewwidth/8)*2);
#endif
		destOff += (id0_byte_t)(viewwidth/8)*2 + (80 - viewwidth/4);
	}
#if 0
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	mov	dx,SC_INDEX+1
asm	mov	al,15
asm	out	dx,al
#else
asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256	// write through all planes
asm	out	dx,ax
#endif

asm	mov	dx,80
asm	mov	ax,[viewwidth]
asm	shr	ax,2
asm	sub	dx,ax					// dx = 40-viewwidth/2

asm	mov	bx,[viewwidth]
asm	shr	bx,3					// bl = viewwidth/8
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height

asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
asm	mov	ax,0x1d1d
#else
asm	mov	ax,[ceiling]
#endif

toploop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1					// half height
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
asm	mov	ax,0xd9d9
#else
asm	mov	ax,0x1919
#endif

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop
#endif
}

//==========================================================================

/*
=====================
=
= CalcRotate
=
=====================
*/

id0_int_t	CalcRotate (objtype *ob)
{
	id0_int_t	angle,viewangle;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (ob->obclass == flameobj || ob->obclass == missileobj)
		return 0;
#endif

	// this isn't exactly correct, as it should vary by a trig value,
	// but it is close enough with only eight rotations

	viewangle = player->angle + (centerx - ob->viewx)/8;

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
	// Including special cases for Wolf3D v1.0 and S3DNA
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) && (!defined GAMEVER_NOAH3D)
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	if (ob->obclass == rocketobj)
#else
	if (ob->obclass == rocketobj || ob->obclass == hrocketobj)
#endif
		angle =  (viewangle-180)- ob->angle;
	else
#endif
		angle =  (viewangle-180)- dirangle[ob->dir];

	angle+=ANGLES/16;
	while (angle>=ANGLES)
		angle-=ANGLES;
	while (angle<0)
		angle+=ANGLES;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (ob->state->rotate == 2)
		return 0;
#else
	if (ob->state->rotate == 2)             // 2 rotation pain frame
		return 4*(angle/(ANGLES/2));        // seperated by 3 (art layout...)
#endif

	return angle/(ANGLES/8);
}


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

#define MAXVISABLE	50

typedef struct
{
	id0_int_t	viewx,
		viewheight,
		// *** S3DNA RESTORATION ***
		shapenum
#ifdef GAMEVER_NOAH3D
		,
		snoring
#endif
		;
} visobj_t;

// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION *** - Move back into function body
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
visobj_t	vislist[MAXVISABLE],*visptr,*visstep,*farthest;
#endif

void DrawScaleds (void)
{
	id0_int_t 		i,j,least,numvisable,height;
	memptr		shape;
	id0_byte_t		*tilespot,*visspot;
	id0_int_t			shapenum;
	id0_unsigned_t	spotloc;

	statobj_t	*statptr;
	objtype		*obj;

// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION *** - Moved back into function body from outside
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
	visobj_t	vislist[MAXVISABLE],*visptr,*visstep,*farthest;

	if (nospr)
		return;
#endif

	visptr = &vislist[0];

//
// place static objects
//
	for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if ((visptr->shapenum = statptr->shapenum) == -1)
			continue;						// object has been deleted

		if (!*statptr->visspot)
			continue;						// not visable

		if (TransformTile (statptr->tilex,statptr->tiley
			,&visptr->viewx,&visptr->viewheight) && statptr->flags & FL_BONUS)
		{
			GetBonus (statptr);
			continue;
		}

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (statptr->shapenum == -2)
			continue;

#endif
		if (!visptr->viewheight)
			continue;						// to close to the object
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		visptr->snoring = 0;
#endif

		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
		if (visptr < &vislist[MAXVISABLE])
#else
		if (visptr < &vislist[MAXVISABLE-1])	// don't let it overflow
#endif
			visptr++;
	}

//
// place active objects
//
	for (obj = player->next;obj;obj=obj->next)
	{
		if (!(visptr->shapenum = obj->state->shapenum))
			continue;						// no shape

		spotloc = (obj->tilex<<6)+obj->tiley;	// optimize: keep in struct?
		visspot = &spotvis[0][0]+spotloc;
		tilespot = &tilemap[0][0]+spotloc;

		//
		// could be in any of the nine surrounding tiles
		//
		if (*visspot
		|| ( *(visspot-1) && !*(tilespot-1) )
		|| ( *(visspot+1) && !*(tilespot+1) )
		|| ( *(visspot-65) && !*(tilespot-65) )
		|| ( *(visspot-64) && !*(tilespot-64) )
		|| ( *(visspot-63) && !*(tilespot-63) )
		|| ( *(visspot+65) && !*(tilespot+65) )
		|| ( *(visspot+64) && !*(tilespot+64) )
		|| ( *(visspot+63) && !*(tilespot+63) ) )
		{
			obj->active = ac_yes/*true*/;
			TransformActor (obj);
			if (!obj->viewheight)
				continue;						// too close or far away

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;
			// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
			if (visptr->shapenum == -1)
				visptr->shapenum = obj->temp1;	// special shape
#endif

			if (obj->state->rotate)
				visptr->shapenum += CalcRotate (obj);

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if ((obj->hitpoints > 0) ||
			    (obj->obclass == missileobj) || (obj->obclass == flameobj) || (obj->obclass == needleobj) || (obj->obclass == rocketobj))
				visptr->snoring = 0;
			else
			{
				// 2.6 number. Upper is frame, Lower is tics*2
				visptr->snoring = obj->snore>>6;
				obj->snore += 3*tics;
			}
#endif

		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
			if (visptr < &vislist[MAXVISABLE])
#else
			if (visptr < &vislist[MAXVISABLE-1])	// don't let it overflow
#endif
				visptr++;
			obj->flags |= FL_VISABLE;
		}
		else
			obj->flags &= ~FL_VISABLE;
	}

//
// draw from back to front
//
	numvisable = visptr-&vislist[0];

	if (!numvisable)
		return;									// no visable objects

	for (i = 0; i<numvisable; i++)
	{
		least = 32000;
		for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
		{
			height = visstep->viewheight;
			if (height < least)
			{
				least = height;
				farthest = visstep;
			}
		}
		//
		// draw farthest
		//
		ScaleShape(farthest->viewx,farthest->shapenum,farthest->viewheight);

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (farthest->snoring)
			ScaleShape(farthest->viewx,farthest->snoring+SPR_SNOOZE_1-1,farthest->viewheight);
#endif
		farthest->viewheight = 32000;
	}

}

//==========================================================================

/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hands
=
==============
*/

id0_int_t	weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY,SPR_PISTOLREADY
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	,SPR_MACHINEGUNREADY,SPR_CHAINREADY,SPR_CANTAREADY,SPR_WATERREADY};
#else
	,SPR_MACHINEGUNREADY,SPR_CHAINREADY};
#endif

void DrawPlayerWeapon (void)
{
	id0_int_t	shapenum;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (player->state == &s_deathcam)
	{
		if (endtics >= 192)
			playstate = ex_victorious;
		SimpleScaleShape(viewwidth/2,SPR_YOUWIN,endtics);
		endtics += tics;
	}
	else if (player->state == &s_gameover)
	{
		SimpleScaleShape(viewwidth/2,SPR_GAMEOVER,endtics);
		endtics += tics;
	}
	// *** ALPHA RESTORATION ***
#elif (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
	if (gamestate.victoryflag)
	{
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		if (player->state == &s_deathcam && (SD_GetTimeCount()&32) )
			SimpleScaleShape(viewwidth/2,SPR_DEATHCAM,viewheight+1);
#endif
		return;
	}
#endif

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
		SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
	}

	if (demorecord || demoplayback)
		SimpleScaleShape(viewwidth/2,SPR_DEMO,viewheight+1);
}


//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
	id0_long_t	newtime,oldtimecount;

//
// calculate tics since last refresh for adaptive timing
//
	// REFKEEN - Looks like this is an unsigned comparison in original EXE
	if ((id0_longword_t)lasttimecount > SD_GetTimeCount())
		SD_SetTimeCount(lasttimecount);		// if the game was paused a LONG time

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	if (DemoMode != demo_Off)
	{
		oldtimecount = lasttimecount;
		SD_TimeCountWaitForDest(oldtimecount + 2*DEMOTICS);
#if 0
		while (oldtimecount + 2*DEMOTICS > TimeCount) ;
#endif

		lasttimecount = oldtimecount + DEMOTICS;
		SD_SetTimeCount(lasttimecount + DEMOTICS);
		tics = DEMOTICS;
		return;
	}
#endif

	// REFKEEN - Some replacement
	SD_TimeCountWaitFromSrc(lasttimecount, 1);
	newtime = SD_GetTimeCount();
	tics = newtime-lasttimecount;
#if 0
	do
	{
		newtime = SD_GetTimeCount();
		tics = newtime-lasttimecount;
	} while (!tics);			// make sure at least one tic passes
#endif

	lasttimecount = newtime;

#ifdef FILEPROFILE
		strcpy (scratch,"\tTics:");
		BE_Cross_itoa_dec (tics,str);
		strcat (scratch,str);
		strcat (scratch,"\n");
		write (profilehandle,scratch,strlen(scratch));
#endif

	if (tics>MAXTICS)
	{
		SD_AddToTimeCount(-(tics-MAXTICS));
		tics = MAXTICS;
	}
}


//==========================================================================


/*
========================
=
= FixOfs
=
========================
*/

void	FixOfs (void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (++screenpage == 3)
		screenpage = 0;
	bufferofs = screenloc[screenpage];
#endif
	VW_ScreenToScreen (displayofs,bufferofs,viewwidth/8,viewheight);
}


//==========================================================================


/*
====================
=
= WallRefresh
=
====================
*/

void WallRefresh (void)
{
//
// set up variables for this view
//
	viewangle = player->angle;
	midangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(focallength,viewcos);
	viewy = player->y + FixedByFrac(focallength,viewsin);

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

	viewtx = player->x >> TILESHIFT;
	viewty = player->y >> TILESHIFT;

	xpartialdown = viewx&(TILEGLOBAL-1);
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = viewy&(TILEGLOBAL-1);
	ypartialup = TILEGLOBAL-ypartialdown;

	lastside = -1;			// the first pixel is on a new wall
	AsmRefresh ();
	ScalePost ();			// no more optimization on last post
}

// *** SHAREWARE V1.0 APOGEE RESTORATION *** - An unused function from v1.0
#if 0 // REFKEEN: Just don't define it
//#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
//==========================================================================

id0_int_t someUnusedDrawArray[] = {
	0x1b, 0x08, 0x1b, 0x1c, 0x00, 0x00, 0x00, 0x00,
	0x1c, 0x08, 0x1c, 0x1b, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void	SomeUnusedDrawFunc (void)
{
	id0_int_t *arrPtr;
	for (arrPtr=someUnusedDrawArray; arrPtr[0]; arrPtr+=8)
	{
		arrPtr[7] -= tics;
		if (arrPtr[7] >= 0)
			continue;
		arrPtr[7] += arrPtr[1];
		++(arrPtr[6]);
		if (arrPtr[arrPtr[6]+2] == 0)
			arrPtr[6] = 0;
		horizwall[arrPtr[0]] = (arrPtr[arrPtr[6]+2]-1)<<1;
		vertwall[arrPtr[0]] = (arrPtr[arrPtr[6]+2]<<1)-1;
	}
}

#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
//==========================================================================

/*
========================
=
= ThreeDRefresh
=
========================
*/

void	ThreeDRefresh (void)
{
	id0_int_t tracedir;

// this wouldn't need to be done except for my debugger/video wierdness
//	outportb (SC_INDEX,SC_MAPMASK); // REFKEEN: Disable

//
// clear out the traced array
//
	memset(spotvis, 0, 64*64);
#if 0
asm	mov	ax,ds
asm	mov	es,ax
asm	mov	di,OFFSET spotvis
asm	xor	ax,ax
asm	mov	cx,2048							// 64*64 / 2
asm	rep stosw
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (++screenpage == 3)
		screenpage = 0;
	bufferofs = screenloc[screenpage]+screenofs;
#else
	bufferofs += screenofs;
#endif

//
// follow the walls from there to the right, drawwing as we go
//
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (nofloors)
#endif
		VGAClearScreen ();

	WallRefresh ();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!nofloors)
		DrawPlanes ();
#endif

//
// draw all the scaled images
//
	DrawScaleds();			// draw scaled stuff
	DrawPlayerWeapon ();	// draw player's hands

//
// show screen and time last cycle
//
	if (fizzlein)
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (!(--fizzlein))
#endif
	{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		VW_FadeIn ();
#else
		FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,20,false);
		fizzlein = false;
#endif

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		lasttimecount = SD_GetTimeCount();
#else
		SD_SetTimeCount(0);
		lasttimecount = 0;
		//lasttimecount = TimeCount = 0;		// don't make a big tic count
#endif

	}

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	displayofs = bufferofs-screenofs;
#else
	bufferofs -= screenofs;
	displayofs = bufferofs;
#endif

#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	BE_ST_SetScreenStartAddress(displayofs);
#else
	BE_ST_SetScreenStartAddressHiPart(displayofs >> 8);

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;
#endif
#if 0
	asm	cli
	asm	mov	cx,[displayofs]
	asm	mov	dx,3d4h		// CRTC address register
	asm	mov	al,0ch		// start address high register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al   	// set the high byte
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	asm	dec	dx
	asm	mov	al,0dh
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,cl
	asm	out	dx,al
	asm	sti
#else
	asm	sti

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;
#endif
#endif

	frameon++;
	PM_NextFrame();
}


//===========================================================================

