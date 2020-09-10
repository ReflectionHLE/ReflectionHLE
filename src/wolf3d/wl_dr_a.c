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

#include "wl_def.h"

// Ported from ASM

REFKEEN_NS_B

#define FINEANGLES 3600
#define DEG90 900
#define DEG180 1800
#define DEG270 2700
#define DEG360 3600

void HitHorizWall (void);
void HitVertWall (void);
void HitHorizDoor (void);
void HitVertDoor (void);
void HitHorizPWall (void);
void HitVertPWall (void);

extern id0_int_t midangle;

extern id0_int_t focaltx, focalty;

// REFKEEN: Originally, these two variables were defined
// but not used at all; Registers were used instead.
extern id0_unsigned_t xpartial, ypartial;

extern id0_unsigned_t xpartialup, xpartialdown, ypartialup, ypartialdown;

extern id0_unsigned_t tilehit;
extern id0_unsigned_t pixx;

extern id0_int_t xtile, ytile, xtilestep, ytilestep;
extern id0_long_t xintercept, yintercept;
extern id0_long_t xstep, ystep;

/*
============
=
= AsmRefresh
=
============
*/

void AsmRefresh (void)
{
	id0_longword_t pos;
	id0_long_t newi;
	id0_unsigned_t xspot, yspot;
	id0_int_t ang;
	int doorpos;
	pixx = 0;
	do
	{
		ang = midangle + pixelangle[pixx];
		if (ang < 0) // -90 - -1 degree arc
		{
			ang += FINEANGLES; // -90 is the same a 270
			goto entry360;
		}
		else if (ang < DEG90) // 0-89 degree arc
		{
entry90:
			xtilestep = 1;
			ytilestep = -1;
			xstep = finetangent[DEG90-1-ang];
			ystep = -finetangent[ang];
			xpartial = xpartialup;
			ypartial = ypartialdown;
		}
		else if (ang < DEG180) // 90-179 degree arc
		{
			xtilestep = -1;
			ytilestep = -1;
			xstep = -finetangent[ang-DEG90];
			ystep = -finetangent[DEG180-1-ang];
			xpartial = xpartialdown;
			ypartial = ypartialdown;
		}
		else if (ang < DEG270) // 180-269 degree arc
		{
			xtilestep = -1;
			ytilestep = 1;
			xstep = -finetangent[DEG270-1-ang];
			ystep = finetangent[ang-DEG180];
			xpartial = xpartialdown;
			ypartial = ypartialup;
		}
		else if (ang < DEG360) // 270-359 degree arc
		{
entry360:
			xtilestep = 1;
			ytilestep = 1;
			xstep = finetangent[ang-DEG270];
			ystep = finetangent[DEG360-1-ang];
			xpartial = xpartialup;
			ypartial = ypartialup;
		}
		else // 360-449 degree arc
		{
			ang -= FINEANGLES; // 449 is the same as 89
			goto entry90;
		}

		yintercept = FixedByFrac(ystep, xpartial) + viewy;
//		yintercept = xpartialbyystep() + viewy;
		xtile = focaltx + xtilestep;
		xspot = (xtile<<6) + ((yintercept>>16)&0xFFFF/*yinttile*/);

		xintercept = FixedByFrac(xstep, ypartial) + viewx;
//		xintercept = ypartialbyxstep() + viewx;
		ytile = focalty + ytilestep;
		yspot = (((xintercept>>16)&0xFFFF/*xinttile*/)<<6) + ytile;

		/*
		==========================================
		trace along this angle until we hit a wall

		CORE LOOP!
		==========================================
		*/

		// check intersections with vertical walls
vertcheck:
		if ((((id0_int_t)(yintercept >> 16) <= ytile) && (ytilestep == -1)) ||
		    (((id0_int_t)(yintercept >> 16) >= ytile) && (ytilestep == 1)))
			goto horizentry;
vertentry:
#ifdef GAMEVER_NOAH3D
		if (!(((id0_byte_t *)tilemap)[xspot] & 0xdf))
#else
		if (!(((id0_byte_t *)tilemap)[xspot] & 0xff))
#endif
		{
passvert:
			((id0_byte_t *)spotvis)[xspot] = 1;
			xtile += xtilestep;
			yintercept += ystep;
			xspot = (xtile<<6)+((yintercept>>16)&0xFFFF/*yinttile*/);
			goto vertcheck;
		}
		tilehit = ((id0_byte_t *)tilemap)[xspot];
#ifdef GAMEVER_NOAH3D
		tilehit &= 0xdf;
#endif
		if (!(tilehit & 0x80)) // no vert door
		{
			xintercept = xtile << 16;
			ytile = yintercept >> 16;
			HitVertWall();
			continue;
		}
		// hit a special vertical wall, so find which coordinate a door would be
		// intersected at, and check to see if the door is open past that point
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (tilehit & 0x40) // both high bits set == pushable wall
		{
			// multiply ystep by pwallmove (0-63)
			newi = (id0_longword_t)ystep * pwallpos;
			// divide by 64 to accomplish a fixed point multiplication
			newi >>= 6;
			// add partial step to the intercept
			newi += yintercept;
			// is it still in the same tile?
			if ((id0_longword_t)(yintercept & 0xFFFF0000) == (newi & 0xFFFF0000))
			{ // draw the pushable wall at the new height
				yintercept = newi; // save pixel intercept position
				xintercept = xtile << 16;
				HitVertPWall();
				continue;
			}
			goto passvert; // no, it hit the side
		}
#endif
		doorpos = tilehit & 0x7f;
		// add half step (to door position) to current intercept pos,
		// and check if it is still in the same tile
		pos = (ystep >> 1) + yintercept;
		if ((pos >> 16) != ((yintercept >> 16)&0xFFFF))
			// midpoint is outside tile, so it hit
			// the side of the wall before a door
			goto passvert;
		// the trace hit the door plane at given pixel position,
		// see if the door is closed that much
		if ((pos & 0xFFFF) < doorposition[doorpos]/*leading edge*/)
			goto passvert;
		// draw the door
		yintercept = (pos & 0xFFFF) | (yintercept & 0xFFFF0000L);
		xintercept = 0x8000 | (xtile << 16); // middle of tile
		HitVertDoor();
		continue;

horizcheck:
		if ((((id0_int_t)(xintercept >> 16) <= xtile) && (xtilestep == -1)) ||
		    (((id0_int_t)(xintercept >> 16) >= xtile) && (xtilestep == 1)))
			goto vertentry;
horizentry:
#ifdef GAMEVER_NOAH3D
		if (!(((id0_byte_t *)tilemap)[yspot] & 0xdf))
#else
		if (!(((id0_byte_t *)tilemap)[yspot] & 0xff))
#endif
		{
passhoriz:
			((id0_byte_t *)spotvis)[yspot] = 1;
			ytile += ytilestep;
			xintercept += xstep;
			yspot = (((xintercept>>16)&0xFFFF/*xinttile*/)<<6)+ytile;
			goto horizcheck;
		}
		tilehit = ((id0_byte_t *)tilemap)[yspot];
#ifdef GAMEVER_NOAH3D
		tilehit &= 0xdf;
#endif
		if (!(tilehit & 0x80)) // no horiz door
		{
			xtile = xintercept >> 16;
			yintercept = ytile << 16;
			HitHorizWall();
			continue;
		}
		// hit a special horizontal wall, so find which coordinate a door would be
		// intersected at, and check to see if the door is open past that point
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (tilehit & 0x40) // both high bits set == pushable wall
		{
			// multiply xstep by pwallmove (0-63)
			newi = (id0_longword_t)xstep * pwallpos;
			// divide by 64 to accomplish a fixed point multiplication
			newi >>= 6;
			// add partial step to the intercept
			newi += xintercept;
			// is it still in the same tile?
			if ((id0_longword_t)(xintercept & 0xFFFF0000) == (newi & 0xFFFF0000))
			{ // draw the pushable wall at the new height
				xintercept = newi; // save pixel intercept position
				yintercept = ytile << 16;
				HitHorizPWall();
				continue;
			}
			goto passhoriz; // no, it hit the side
		}
#endif
		doorpos = tilehit & 0x7f;
		// add half step (to door position) to current intercept pos,
		// and check if it is still in the same tile
		pos = (xstep >> 1) + xintercept;
		if ((pos >> 16) != ((xintercept >> 16)&0xFFFF))
			// midpoint is outside tile, so it hit
			// the side of the wall before a door
			goto passhoriz;
		// the trace hit the door plane at given pixel position,
		// see if the door is closed that much
		if ((pos & 0xFFFF) < doorposition[doorpos]/*leading edge*/)
			goto passhoriz;
		// draw the door
		xintercept = (pos & 0xFFFF) | (xintercept & 0xFFFF0000L);
		yintercept = 0x8000 | (ytile << 16); // middle of tile
		HitHorizDoor();
	} while (++pixx < viewwidth);
}

REFKEEN_NS_E
