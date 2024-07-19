/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2024 NY00123
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

// WOLFHACK.C

#include "wl_def.h"

#ifdef GAMEVER_NOAH3D // REFKEEN: Limit compiled code to S3DNA only for now
REFKEEN_NS_B

#define	MAXVIEWHEIGHT	200

id0_int_t		spanstart[MAXVIEWHEIGHT/2];

fixed	stepscale[MAXVIEWHEIGHT/2];
fixed	basedist[MAXVIEWHEIGHT/2];

extern	id0_char_t	id0_far	planepics[8192];	// 4k of ceiling, 4k of floor

id0_int_t		halfheight = 0;

id0_word_t	planeylookup[MAXVIEWHEIGHT/2]; // REFKEEN: These are just offsets now
#if 0
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_byte_t	*planeylookup[MAXVIEWHEIGHT/2];
#else
id0_byte_t	id0_far *planeylookup[MAXVIEWHEIGHT/2];
#endif
#endif
id0_unsigned_t	mirrorofs[MAXVIEWHEIGHT/2];

fixed	psin, pcos;

fixed FixedMul (fixed a, fixed b)
{
	return (a>>8)*(b>>8);
}


id0_int_t		mr_rowofs;
id0_int_t		mr_count;
id0_int_t		mr_xstep;
id0_int_t		mr_ystep;
id0_int_t		mr_xfrac;
id0_int_t		mr_yfrac;
id0_int_t		mr_dest;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void	MapRow (int plane); // REFKEEN: We need the VGA plane
//void	MapRow (void);
#endif

/*
==============
=
= DrawSpans
=
= Height ranges from 0 (infinity) to viewheight/2 (nearest)
==============
*/

void DrawSpans (id0_int_t x1, id0_int_t x2, id0_int_t height)
{
	fixed		length;
	id0_int_t			ofs;
	id0_int_t			prestep;
	fixed		startxfrac, startyfrac;

	id0_int_t			x, startx, count, plane, startplane;
	// REFKEEN: toprow is simplfy an offset now, and we don't need dest.
	id0_unsigned_t		toprow;
#if 0
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_byte_t		*toprow;
#else
	id0_byte_t		id0_far	*toprow, id0_far *dest;
#endif
#endif

	toprow = planeylookup[height]+bufferofs;
	mr_rowofs = mirrorofs[height];
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!height)
		Quit ("DrawSpans(): Zero height!");
#endif

	mr_xstep = (psin<<1)/height;
	mr_ystep = (pcos<<1)/height;

	length = basedist[height];
	startxfrac = (viewx + FixedMul(length,pcos));
	startyfrac = (viewy - FixedMul(length,psin));

// draw two spans simultaniously

	plane = startplane = x1&3;
	prestep = viewwidth/2 - x1;
	do
	{
//		outportb (SC_INDEX+1,1<<plane);
		mr_xfrac = startxfrac - (mr_xstep>>2)*prestep;
		mr_yfrac = startyfrac - (mr_ystep>>2)*prestep;

		startx = x1>>2;
		mr_dest = (id0_unsigned_t)toprow + startx;
		mr_count = ((x2-plane)>>2) - startx + 1;
		x1++;
		prestep--;
		if (mr_count)
			MapRow (plane); // REFKEEN: We need the VGA plane
//			MapRow ();
		plane = (plane+1)&3;
	} while (plane != startplane);

}




// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void LoadFloorTiles (id0_int_t tile)
{
	id0_unsigned_char_t	id0_far *dest, id0_far *src;
	id0_int_t		x;

	tile = tile*2+(PMSpriteStart-8);
	if (tile+1 >= PMSpriteStart)
	{
		sprintf (str,"LoadFloorTiles(): Invalid floor tile %u!", (tile-(PMSpriteStart-8))/2);
		Quit (str);
	}
	src = (id0_byte_t *)PM_GetPage (tile+1);
	dest = (id0_byte_t *)planepics;
	for (x=0;x<PMPageSize;x++)
	{
		*dest = *src++;
		dest += 2;
	}
	
	src = (id0_byte_t *)PM_GetPage (tile);
	dest = (id0_byte_t *)planepics+1;
	for (x=0;x<PMPageSize;x++)
	{
		*dest = *src++;
		dest += 2;
	}
}
#endif

/*
===================
=
= SetPlaneViewSize
=
===================
*/

void SetPlaneViewSize (void)
{
	id0_int_t		x,y;
	id0_byte_t 	id0_far *dest, id0_far *src;

	halfheight = viewheight>>1;


	for (y=0 ; y<halfheight ; y++)
	{
		// REFKEEN: It's just an offset now
		planeylookup[y] = (halfheight-1-y)*SCREENBWIDE;
#if 0
		// *** S3DNA RESTORATION ***
		// It's technically useless to cast 0xa0000000l to a 16-bit near
		// pointer, but this helps to (re)generate some ASM instruction
#ifdef GAMEVER_NOAH3D
		planeylookup[y] = (id0_byte_t *)0xa0000000l + (halfheight-1-y)*SCREENBWIDE;
#else
		planeylookup[y] = (id0_byte_t id0_far *)0xa0000000l + (halfheight-1-y)*SCREENBWIDE;;
#endif
#endif
#if 0
	asm	nop
	asm	nop
	asm	nop
#endif
		mirrorofs[y] = (y*2+1)*SCREENBWIDE;

		stepscale[y] = y*GLOBAL1/32;
		if (y>0)
			basedist[y] = GLOBAL1/2*scale/y;
	}

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	src = PM_GetPage(0);
	dest = (id0_byte_t *)planepics;
	for (x=0 ; x<4096 ; x++)
	{
		*dest = *src++;
		dest += 2;
	}
	src = PM_GetPage(1);
	dest = (id0_byte_t *)planepics+1;
	for (x=0 ; x<4096 ; x++)
	{
		*dest = *src++;
		dest += 2;
	}
#endif

}


/*
===================
=
= DrawPlanes
=
===================
*/

void DrawPlanes (void)
{
	id0_int_t		height, lastheight;
	id0_int_t		x;

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	if (viewheight>>1 != halfheight)
		SetPlaneViewSize ();		// screen size has changed
#endif


	psin = viewsin;
	if (psin < 0)
		psin = -(psin&0xffff);
	pcos = viewcos;
	if (pcos < 0)
		pcos = -(pcos&0xffff);

//
// loop over all columns
//
	lastheight = halfheight;

	for (x=0 ; x<viewwidth ; x++)
	{
		height = wallheight[x]>>3;
		if (height < lastheight)
		{	// more starts
			do
			{
				spanstart[--lastheight] = x;
			} while (lastheight > height);
		}
		else if (height > lastheight)
		{	// draw spans
			if (height > halfheight)
				height = halfheight;
			for ( ; lastheight < height ; lastheight++)
				DrawSpans (spanstart[lastheight], x-1, lastheight);
		}
	}

	height = halfheight;
	for ( ; lastheight < height ; lastheight++)
		DrawSpans (spanstart[lastheight], x-1, lastheight);
}

REFKEEN_NS_E
#endif
