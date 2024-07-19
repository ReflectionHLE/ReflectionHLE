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

// WL_GAME.C

#include "wl_def.h"
//#pragma hdrstop

#ifdef MYPROFILE
#include <time.h>
#endif

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

// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
id0_boolean_t	screensplit;
#endif
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_boolean_t		ingame;
id0_int_t		endtics,fizzlein;
#else
id0_boolean_t		ingame,fizzlein;
#endif
id0_unsigned_t	latchpics[NUMLATCHPICS];
gametype	gamestate;

// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
id0_long_t		spearx,speary;
id0_unsigned_t	spearangle;
id0_boolean_t		spearflag;
#endif

//
// ELEVATOR BACK MAPS - REMEMBER (-1)!!
//

// *** SHAREWARE V1.0 APOGEE + ALPHA + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
id0_int_t ElevatorBackTo[]={1,1,7,0,0,0};
#elif (!defined GAMEVER_NOAH3D)
id0_int_t ElevatorBackTo[]={1,1,7,3,5,3};
#endif
#endif

void ScanInfoPlane (void);
void SetupGameLevel (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
void GameLoop (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/



//===========================================================================
//===========================================================================


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
/*
==========================
=
= SetSoundLoc - Given the location of an object (in terms of global
=	coordinates, held in globalsoundx and globalsoundy), munges the values
=	for an approximate distance from the left and right ear, and puts
=	those values into leftchannel and rightchannel.
=
= JAB
=
==========================
*/

	fixed	globalsoundx,globalsoundy;
	id0_int_t		leftchannel,rightchannel;
#define ATABLEMAX 15
// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
id0_int_t righttable[ATABLEMAX][ATABLEMAX * 2] = {
{14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  6,  8, 10, 12, 15},
{14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 12,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  5,  7,  9, 11, 13, 15},
{14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 12, 10,  6,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  5,  7,  9, 11, 13, 15},
{14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 12, 11,  9,  5,  2,  0,  0,  0,  0,  0,  0,  0,  2,  4,  5,  7,  9, 11, 13, 15},
{14, 14, 14, 14, 14, 14, 14, 13, 13, 12, 11, 10,  8,  6,  3,  0,  0,  0,  0,  0,  0,  1,  3,  5,  6,  8, 10, 12, 14, 15},
{14, 14, 14, 14, 14, 14, 14, 13, 13, 12, 11, 10,  8,  6,  4,  2,  1,  0,  0,  1,  2,  3,  4,  6,  7,  9, 11, 12, 14, 15},
{15, 14, 14, 14, 14, 14, 13, 13, 12, 12, 11, 10,  8,  7,  5,  4,  3,  2,  2,  2,  3,  4,  5,  7,  8, 10, 11, 13, 15, 15},
{15, 14, 14, 14, 14, 14, 13, 13, 12, 12, 11, 10,  9,  7,  6,  5,  4,  4,  4,  4,  5,  6,  7,  8,  9, 11, 12, 14, 15, 15},
{15, 15, 14, 14, 14, 14, 13, 13, 12, 12, 11, 10,  9,  8,  7,  6,  6,  6,  6,  6,  6,  7,  8,  9, 11, 12, 13, 15, 15, 15},
{15, 15, 15, 14, 14, 14, 13, 13, 13, 12, 11, 11, 10,  9,  8,  8,  7,  7,  7,  7,  8,  9, 10, 11, 12, 13, 15, 15, 15, 15},
{15, 15, 15, 14, 14, 14, 14, 13, 13, 12, 12, 11, 11, 10,  9,  9,  9,  9,  9,  9,  9, 10, 11, 12, 13, 14, 15, 15, 15, 15},
{15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 12, 13, 14, 15, 15, 15, 15, 15},
{15, 15, 15, 15, 15, 15, 14, 14, 14, 13, 13, 12, 12, 12, 11, 11, 11, 11, 11, 12, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15},
{15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15},
{15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15}
};
id0_int_t lefttable[ATABLEMAX][ATABLEMAX * 2] = {
{15, 12, 10,  8,  6,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0, 11, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14},
{15, 13, 11,  9,  7,  5,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 12, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14},
{15, 13, 11,  9,  7,  5,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  6, 10, 12, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14},
{15, 13, 11,  9,  7,  5,  4,  2,  0,  0,  0,  0,  0,  0,  0,  2,  5,  9, 11, 12, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14},
{15, 14, 12, 10,  8,  6,  5,  3,  1,  0,  0,  0,  0,  0,  0,  3,  6,  8, 10, 11, 12, 13, 13, 14, 14, 14, 14, 14, 14, 14},
{15, 14, 12, 11,  9,  7,  6,  4,  3,  2,  1,  0,  0,  1,  2,  4,  6,  8, 10, 11, 12, 13, 13, 14, 14, 14, 14, 14, 14, 14},
{15, 15, 13, 11, 10,  8,  7,  5,  4,  3,  2,  2,  2,  3,  4,  5,  7,  8, 10, 11, 12, 12, 13, 13, 14, 14, 14, 14, 14, 15},
{15, 15, 14, 12, 11,  9,  8,  7,  6,  5,  4,  4,  4,  4,  5,  6,  7,  9, 10, 11, 12, 12, 13, 13, 14, 14, 14, 14, 14, 15},
{15, 15, 15, 13, 12, 11,  9,  8,  7,  6,  6,  6,  6,  6,  6,  7,  8,  9, 10, 11, 12, 12, 13, 13, 14, 14, 14, 14, 15, 15},
{15, 15, 15, 15, 13, 12, 11, 10,  9,  8,  7,  7,  7,  7,  8,  8,  9, 10, 11, 11, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15},
{15, 15, 15, 15, 14, 13, 12, 11, 10,  9,  9,  9,  9,  9,  9,  9, 10, 11, 11, 12, 12, 13, 13, 14, 14, 14, 14, 15, 15, 15},
{15, 15, 15, 15, 15, 14, 13, 12, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15},
{15, 15, 15, 15, 15, 15, 15, 14, 13, 12, 12, 11, 11, 11, 11, 11, 12, 12, 12, 13, 13, 14, 14, 14, 15, 15, 15, 15, 15, 15},
{15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15},
{15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15}
};
#else
id0_byte_t righttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
id0_byte_t lefttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
#endif

void
SetSoundLoc(fixed gx,fixed gy)
{
	fixed	xt,yt;
	id0_int_t		x,y;

//
// translate point to view centered coordinates
//
	gx -= viewx;
	gy -= viewy;

//
// calculate newx
//
	xt = FixedByFrac(gx,viewcos);
	yt = FixedByFrac(gy,viewsin);
	x = (xt - yt) >> TILESHIFT;

//
// calculate newy
//
	xt = FixedByFrac(gx,viewsin);
	yt = FixedByFrac(gy,viewcos);
	y = (yt + xt) >> TILESHIFT;

	if (y >= ATABLEMAX)
		y = ATABLEMAX - 1;
	else if (y <= -ATABLEMAX)
		y = -ATABLEMAX;
	if (x < 0)
		x = -x;
	if (x >= ATABLEMAX)
		x = ATABLEMAX - 1;
	leftchannel  =  lefttable[x][y + ATABLEMAX];
	rightchannel = righttable[x][y + ATABLEMAX];
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (leftchannel > 5)
		leftchannel = 5;
	if (rightchannel > 5)
		rightchannel = 5;
#endif
#if 0
	CenterWindow(8,1);
	US_PrintSigned(leftchannel);
	US_Print(",");
	US_PrintSigned(rightchannel);
	VW_UpdateScreen();
#endif
}

/*
==========================
=
= SetSoundLocGlobal - Sets up globalsoundx & globalsoundy and then calls
=	UpdateSoundLoc() to transform that into relative channel volumes. Those
=	values are then passed to the Sound Manager so that they'll be used for
=	the next sound played (if possible).
=
= JAB
=
==========================
*/
void PlaySoundLocGlobal(soundnames s,fixed gx,fixed gy)
//void PlaySoundLocGlobal(id0_word_t s,fixed gx,fixed gy)
{
	SetSoundLoc(gx,gy);
	SD_PositionSound(leftchannel,rightchannel);
	if (SD_PlaySound(s))
	{
		globalsoundx = gx;
		globalsoundy = gy;
	}
}

void UpdateSoundLoc(void)
{
	if (SoundPositioned)
	{
		SetSoundLoc(globalsoundx,globalsoundy);
		SD_SetPosition(leftchannel,rightchannel);
	}
}

/*
**	JAB End
*/


/*
==========================
=
= ClearMemory
=
==========================
*/

void ClearMemory (void)
{
	PM_UnlockMainMem();
	SD_StopDigitized();
	MM_SortMem ();
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


/*
==========================
=
= ScanInfoPlane
=
= Spawn all actors and mark down special places
=
==========================
*/

void ScanInfoPlane (void)
{
	id0_unsigned_t	x,y,i,j;
	id0_int_t			tile;
	id0_unsigned_t	id0_far	*start;

	start = mapsegs[1];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *start++;
			if (!tile)
				continue;

			switch (tile)
			{
			case 19:
			case 20:
			case 21:
			case 22:
				SpawnPlayer(x,y,NORTH+tile-19);
				break;

			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:

			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
			case 38:

			case 39:
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:

			case 47:
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:

			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			case 63:
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
			case 70:
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			case 71:
			case 72:
			case 73:						// TRUCK AND SPEAR!
			case 74:
#endif
#endif

				SpawnStatic(x,y,tile-23);
				break;
// *** ALPHA RESTORATION ***
// Apparently we need the redundant break for code recreation
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			case 74:
				break;
#else

//
// P wall
//
			case 98:
				if (!loadedgame)
				  gamestate.secrettotal++;
				break;
#endif

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			case 99:
				*(start-1) = 0;
				break;
			case 100:
			case 101:
				SpawnExit(x,y,tile==101);
				break;
#endif

//
// guard
//
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			case 108:
			case 126:
				SpawnStand(en_guard,x,y,0,tile==126);
				break;
#else
			case 180:
			case 181:
			case 182:
			case 183:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 144:
			case 145:
			case 146:
			case 147:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 108:
			case 109:
			case 110:
			case 111:
				SpawnStand(en_guard,x,y,tile-108);
				break;


			case 184:
			case 185:
			case 186:
			case 187:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 148:
			case 149:
			case 150:
			case 151:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 112:
			case 113:
			case 114:
			case 115:
				SpawnPatrol(en_guard,x,y,tile-112);
				break;

			case 124:
				SpawnDeadGuard (x,y);
				break;
#endif
//
// officer
//
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			case 109:
			case 127:
				SpawnStand(en_officer,x,y,0,tile==127);
				break;
#else
			case 188:
			case 189:
			case 190:
			case 191:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 152:
			case 153:
			case 154:
			case 155:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 116:
			case 117:
			case 118:
			case 119:
				SpawnStand(en_officer,x,y,tile-116);
				break;


			case 192:
			case 193:
			case 194:
			case 195:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 156:
			case 157:
			case 158:
			case 159:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 120:
			case 121:
			case 122:
			case 123:
				SpawnPatrol(en_officer,x,y,tile-120);
				break;
#endif


//
// ss
//
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			case 110:
			case 128:
				SpawnStand(en_ss,x,y,0,tile==128);
				break;
#else
			case 198:
			case 199:
			case 200:
			case 201:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 162:
			case 163:
			case 164:
			case 165:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 126:
			case 127:
			case 128:
			case 129:
				SpawnStand(en_ss,x,y,tile-126);
				break;


			case 202:
			case 203:
			case 204:
			case 205:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 166:
			case 167:
			case 168:
			case 169:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 130:
			case 131:
			case 132:
			case 133:
				SpawnPatrol(en_ss,x,y,tile-130);
				break;
#endif

//
// dogs
//
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			case 111:
			case 129:
				SpawnStand(en_dog,x,y,0,tile==129);
				break;
#else
			case 206:
			case 207:
			case 208:
			case 209:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 170:
			case 171:
			case 172:
			case 173:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 134:
			case 135:
			case 136:
			case 137:
				SpawnStand(en_dog,x,y,tile-134);
				break;


			case 210:
			case 211:
			case 212:
			case 213:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 174:
			case 175:
			case 176:
			case 177:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 138:
			case 139:
			case 140:
			case 141:
				SpawnPatrol(en_dog,x,y,tile-138);
				break;
#endif

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
//
// mutants
//
			case 112:
			case 130:
				SpawnStand(en_mutant,x,y,0,tile==130);
				break;
#endif
//
// boss
//
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			case 113:
			case 131:
				SpawnBoss (x,y);
				break;
			case 114:
			case 132:
				SpawnSchabbs (x,y);
				break;
			case 115:
			case 133:
				SpawnGretel (x,y);
				break;
			case 116:
			case 134:
				SpawnGift (x,y);
				break;
			case 117:
			case 135:
				SpawnFat (x,y);
				break;
			case 118:
			case 136:
				SpawnHitler (x,y);
				break;
#elif (!defined SPEAR)
//#ifndef SPEAR
			case 214:
				SpawnBoss (x,y);
				break;
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			case 197:
				SpawnGretel (x,y);
				break;
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			case 215:
				SpawnGift (x,y);
				break;
#endif
			case 179:
				SpawnFat (x,y);
				break;
#endif
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			case 196:
				SpawnSchabbs (x,y);
				break;
			case 160:
				SpawnFakeHitler (x,y);
				break;
			case 178:
				SpawnHitler (x,y);
				break;
#endif
#else
			case 106:
				SpawnSpectre (x,y);
				break;
			case 107:
				SpawnAngel (x,y);
				break;
			case 125:
				SpawnTrans (x,y);
				break;
			case 142:
				SpawnUber (x,y);
				break;
			case 143:
				SpawnWill (x,y);
				break;
			case 161:
				SpawnDeath (x,y);
				break;

#endif

			// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			default:
				sprintf (str,"ScanInfoPlane: Invalid object $%02X at %d, %d!\n",
					tile,x,y);
				Quit (str);
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
//
// mutants
//
			case 252:
			case 253:
			case 254:
			case 255:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 234:
			case 235:
			case 236:
			case 237:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 216:
			case 217:
			case 218:
			case 219:
				SpawnStand(en_mutant,x,y,tile-216);
				break;

			case 256:
			case 257:
			case 258:
			case 259:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 238:
			case 239:
			case 240:
			case 241:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 220:
			case 221:
			case 222:
			case 223:
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
				SpawnStand(en_mutant,x,y,tile-220);
#else
				SpawnPatrol(en_mutant,x,y,tile-220);
#endif
				break;

//
// ghosts
//
#ifndef SPEAR
			case 224:
				SpawnGhosts (en_blinky,x,y);
				break;
			case 225:
				SpawnGhosts (en_clyde,x,y);
				break;
			case 226:
				SpawnGhosts (en_pinky,x,y);
				break;
			case 227:
				SpawnGhosts (en_inky,x,y);
				break;
#endif
#endif // GAMEVER_NOAH3D
			}

		}
}

//==========================================================================

/*
==================
=
= SetupGameLevel
=
==================
*/

void SetupGameLevel (void)
{
	id0_int_t	x,y,i;
	id0_unsigned_t	id0_far *map,tile,spot;


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (!loadedgame)
	{
	 gamestate.TimeCount=
	 gamestate.secrettotal=
	 gamestate.killtotal=
	 gamestate.treasuretotal=
	 gamestate.secretcount=
	 gamestate.killcount=
	 gamestate.treasurecount=0;
	 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	 pwallstate=0;
#endif
	}
#endif

	if (demoplayback || demorecord)
		US_InitRndT (false);
	else
		US_InitRndT (true);

	/* REFKEEN: Originally, if mapheaderseg[gamestate.mapon] were null,
	   then there would be null pointer dereferences from CA_CacheMap
	   and this function. There's also the risk that CA_CacheMap would
	   try to Carmack-expand too much (invalid) map data, depending on
	   factors like what was the expanded size read by CA_CacheMap.
	   from a temporary buffer. Assuming execution would complete,
	   though, mapwidth and mapheight would both differ from 64, due
	   to the EXE layout. Therefore, this function would eventually
	   abort with an error.
	   For the source port, just show the error instead. */
	if (mapheaderseg[gamestate.mapon])
	{
//
// load the level
//
		// *** S3DNA + ALPHA RESTORATION ***
#if (defined GAMEVER_NOAH3D)
		CA_CacheMap (gamestate.mapon);

		mapwidth = mapheaderseg[gamestate.mapon]->width;
		mapheight = mapheaderseg[gamestate.mapon]->height;
#else
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		CA_CacheMap (gamestate.mapon);
#else
		CA_CacheMap (gamestate.mapon+10*gamestate.episode);
		mapon-=gamestate.episode*10;
#endif

		mapwidth = mapheaderseg[mapon]->width;
		mapheight = mapheaderseg[mapon]->height;
#endif
	}

	// REFKEEN: As written above, simply show the error if there's no map
	if (!mapheaderseg[gamestate.mapon] || mapwidth != 64 || mapheight != 64)
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		Quit ("SetupGameLevel(): Map not 64*64!");
#else
		Quit ("Map not 64*64!");
#endif


//
// copy the wall data to a data segment array
//
	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<AREATILE)
			{
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				if (tile == 89)
					tile = 31;
				else
					tile &= 31;
#endif
			// solid wall
				tilemap[x][y] = tile;
				/*(id0_unsigned_t)*/actorat[x][y] = tile;
			}
			else
			{
			// area floor
				tilemap[x][y] = 0;
				/*(id0_unsigned_t)*/actorat[x][y] = 0;
			}
		}

//
// spawn doors
//
	InitActorList ();			// start spawning things with a clean slate
	InitDoorList ();
	InitStaticList ();

	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile >= 90 && tile <= 101)
			{
			// door
				switch (tile)
				{
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				case 90:
				case 91:
					SpawnDoor (x,y,!(tile&1),0);
					break;
				case 92:
				case 93:
					SpawnDoor (x,y,!(tile&1),1);
					break;
				case 94:
				case 95:
					SpawnDoor (x,y,!(tile&1),2);
					break;
				case 96:
				case 97:
					SpawnDoor (x,y,!(tile&1),5);
					break;
#else
				case 90:
				case 92:
				case 94:
				case 96:
				case 98:
				case 100:
					SpawnDoor (x,y,1,(tile-90)/2);
					break;
				case 91:
				case 93:
				case 95:
				case 97:
				case 99:
				case 101:
					SpawnDoor (x,y,0,(tile-91)/2);
					break;
#endif
				}
			}
		}

//
// spawn actors
//
	ScanInfoPlane ();

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
//
// take out the ambush markers
//
	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile == AMBUSHTILE)
			{
				tilemap[x][y] = 0;
				if ( (id0_unsigned_t)actorat[x][y] == AMBUSHTILE)
					actorat[x][y] = 0;

				if (*map >= AREATILE)
					tile = *map;
				if (*(map-1-mapwidth) >= AREATILE)
					tile = *(map-1-mapwidth);
				if (*(map-1+mapwidth) >= AREATILE)
					tile = *(map-1+mapwidth);
				if ( *(map-2) >= AREATILE)
					tile = *(map-2);

				*(map-1) = tile;
			}
		}
#endif



//
// have the caching manager load and purge stuff to make sure all marks
// are in memory
//
	CA_LoadAllSounds ();

}


//==========================================================================

// *** SHAREWARE V1.0 APOGEE RESTORATION *** - A function that is used in v1.0
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
void ResetSplitScreen (void)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	screensplit = false;
#endif
	VW_SetSplitScreen(200);
	bufferofs = displayofs = 0;
	VW_Bar(0,0,320,200,0);
	bufferofs = 19200;
	VW_Bar(0,0,320,200,0);
	VW_SetScreen(displayofs, 0);
}
#endif


/*
===================
=
= DrawPlayBorderSides
=
= To fix window overwrites
=
===================
*/

void DrawPlayBorderSides (void)
{
	id0_int_t	xl,yl;

	xl = 160-viewwidth/2;
	yl = (200-STATUSLINES-viewheight)/2;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Bar (0,0,xl-1,200-STATUSLINES,VIEWCOLOR);
	VWB_Bar (xl+viewwidth+1,0,xl-2,200-STATUSLINES,VIEWCOLOR);

	VWB_Vlin (yl-1,yl+viewheight,xl-1,220);
	VWB_Vlin (yl-1,yl+viewheight,xl+viewwidth,216);
#else
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VWB_Bar (0,0,xl-2,200-STATUSLINES,127);
#else
	VWB_Bar (0,0,xl-1,200-STATUSLINES,127);
#endif
	VWB_Bar (xl+viewwidth+1,0,xl-2,200-STATUSLINES,127);

	VWB_Vlin (yl-1,yl+viewheight,xl-1,0);
	VWB_Vlin (yl-1,yl+viewheight,xl+viewwidth,125);
#endif
}


// *** SHAREWARE V1.0 APOGEE RESTORATION *** - NOT used in v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
/*
===================
=
= DrawAllPlayBorderSides
=
===================
*/

void DrawAllPlayBorderSides (void)
{
	id0_unsigned_t	i,temp;

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorderSides ();
	}
	bufferofs = temp;
}

/*
===================
=
= DrawPlayBorder
=
===================
*/
void DrawAllPlayBorder (void)
{
	id0_unsigned_t	i,temp;

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10


/*
===================
=
= DrawPlayBorder
=
===================
*/

void DrawPlayBorder (void)
{
	id0_int_t	xl,yl;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Bar (0,0,320,200-STATUSLINES,VIEWCOLOR);
#else
	VWB_Bar (0,0,320,200-STATUSLINES,127);
#endif

	xl = 160-viewwidth/2;
	yl = (200-STATUSLINES-viewheight)/2;
	VWB_Bar (xl,yl,viewwidth,viewheight,0);

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Hlin (xl-1,xl+viewwidth,yl-1,220);
	VWB_Hlin (xl-1,xl+viewwidth,yl+viewheight,216);
	VWB_Vlin (yl-1,yl+viewheight,xl-1,220);
	VWB_Vlin (yl-1,yl+viewheight,xl+viewwidth,216);
	VWB_Plot (xl-1,yl+viewheight,214);
#else
	VWB_Hlin (xl-1,xl+viewwidth,yl-1,0);
	VWB_Hlin (xl-1,xl+viewwidth,yl+viewheight,125);
	VWB_Vlin (yl-1,yl+viewheight,xl-1,0);
	VWB_Vlin (yl-1,yl+viewheight,xl+viewwidth,125);
	VWB_Plot (xl-1,yl+viewheight,124);
#endif
}



/*
===================
=
= DrawPlayScreen
=
===================
*/

void DrawPlayScreen (void)
{
	id0_int_t	i,j,p,m;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	id0_unsigned_t	temp;
#endif

	VW_FadeOut ();

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	screenpage = 0;
	bufferofs = 0;
#else
	temp = bufferofs;
#endif

	CA_CacheGrChunk (STATUSBARPIC);

	// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	VWB_DrawPic (0,0,STATUSBARPIC);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	VW_Hlin (0,319,STATUSLINES,127);
#endif
	UNCACHEGRCHUNK (STATUSBARPIC);
#endif

	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		VWB_DrawPic (0,200-STATUSLINES,STATUSBARPIC);
#endif
	}

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	bufferofs = temp;

	UNCACHEGRCHUNK (STATUSBARPIC);
#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	DrawTreasure ();
#endif
	DrawFace ();
	DrawHealth ();
	DrawLives ();
	DrawLevel ();
	DrawAmmo ();
	DrawKeys ();
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	DrawWeapon ();
#endif
	DrawScore ();

	// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	screensplit = true;
#endif
	VW_SetSplitScreen(200-STATUSLINES);
	bufferofs = displayofs = screenloc[0];
	VW_SetScreen(displayofs, 0);	
#endif
}



// *** ALPHA RESTORATION ***
// An empty stub called from GameLoop in the alpha;
// Its original purpose is unknown. However, given the fact
// that it precedes StartDemoRecord, using comparisons to Keen 4-6
// as a base, it might have been an implementation of finishcachebox.
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void SomeNullGameLoopFunc(void)
{
}
#endif

//==========================================================================

/*
==================
=
= StartDemoRecord
=
==================
*/

#define MAXDEMOSIZE	8192

void StartDemoRecord (id0_int_t levelnumber)
{
	MM_GetPtr (&demobuffer,MAXDEMOSIZE);
	MM_SetLock (&demobuffer,true);
	demoptr = (id0_char_t id0_far *)demobuffer;
	lastdemoptr = demoptr+MAXDEMOSIZE;

	*demoptr = levelnumber;
	demoptr += 4;				// leave space for length
	demorecord = true;
}


/*
==================
=
= FinishDemoRecord
=
==================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
id0_char_t	demoname[] = "DEMO?.WL1";
#else
id0_char_t	demoname[13] = "DEMO?.";
#endif

void FinishDemoRecord (void)
{
	id0_long_t	length,level;

	demorecord = false;

	length = demoptr - (id0_char_t id0_far *)demobuffer;

	demoptr = ((id0_char_t id0_far *)demobuffer)+1;
	*(id0_unsigned_t id0_far *)demoptr = length;

	CenterWindow(24,3);
	PrintY+=6;
	US_Print(" Demo number (0-9):");
	VW_UpdateScreen();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VW_FadeIn();
#endif

	if (US_LineInput (px,py,str,NULL,true,2,0))
	{
		level = atoi (str);
		if (level>=0 && level<=9)
		{
			demoname[4] = '0'+level;
			CA_WriteFile (demoname,(void id0_far *)demobuffer,length);
		}
	}


	MM_FreePtr (&demobuffer);
}

//==========================================================================

/*
==================
=
= RecordDemo
=
= Fades the screen out, then starts a demo.  Exits with the screen faded
=
==================
*/

void RecordDemo (void)
{
	id0_int_t level,esc;

	CenterWindow(26,3);
	PrintY+=6;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	CA_CacheGrChunk(STARTFONT);
	fontnumber=0;
#endif
	// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	US_Print("  Demo which level(1-30):");
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
	US_Print("  Demo which level(1-20):");
#else
	US_Print("  Demo which level(1-10):");
#endif
	VW_UpdateScreen();
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	VW_FadeIn ();
#endif
	esc = !US_LineInput (px,py,str,NULL,true,2,0);
	if (esc)
		return;

	level = atoi (str);
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if ((level < 1) || (level > 20))
		return;
#else
	level--;
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VW_FadeOut ();
	NewGame (gd_medium);
	SETFONTCOLOR(0,15);
#else
	SETFONTCOLOR(0,15);
	VW_FadeOut ();

	// *** SHAREWARE V1.0 APOGEE  RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	NewGame (gd_easy,0);
	gamestate.mapon = level-1;
	gamestate.difficulty = gd_hard;
	// *** S3DNA RESTORATION ***
#elif (!defined SPEAR) && (!defined GAMEVER_NOAH3D)
//#ifndef SPEAR
	NewGame (gd_hard,level/10);
	gamestate.mapon = level%10;
#else
	NewGame (gd_hard,0);
	gamestate.mapon = level;
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	StartDemoRecord (level-1);
#else
	StartDemoRecord (level);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

	DrawPlayScreen ();
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_CheckMainMem ();
#endif
	VW_FadeIn ();
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PreloadGraphics ();
	PM_UnlockMainMem ();
	gamestate.mapon = level-1;
	StartDemoRecord (level-1);
#endif

	startgame = false;
	demorecord = true;

	SetupGameLevel ();
	StartMusic ();
	PM_CheckMainMem ();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	fizzlein = 3;
	LoadFloorTiles (FloorTile[gamestate.mapon]);
#else
	fizzlein = true;
#endif

	PlayLoop ();

	demoplayback = false;

	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_UnlockMainMem ();
#endif
	StopMusic ();
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	FinishDemoRecord ();
#endif
	VW_FadeOut ();
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	FinishPaletteShifts ();
	screenfaded = true;
#else
	ClearMemory ();
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	ResetSplitScreen ();
#endif

	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	FinishDemoRecord ();
#endif
}

//==========================================================================

/*
==================
=
= PlayDemo
=
= Fades the screen out, then starts a demo.  Exits with the screen faded
=
==================
*/

void PlayDemo (id0_int_t demonumber)
{
	id0_int_t length;

#ifdef DEMOSEXTERN
// debug: load chunk
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_int_t dems[3]={T_DEMO0,T_DEMO1,T_DEMO2};
#elif (!defined SPEARDEMO)
//#ifndef SPEARDEMO
	id0_int_t dems[4]={T_DEMO0,T_DEMO1,T_DEMO2,T_DEMO3};
#else
	id0_int_t dems[1]={T_DEMO0};
#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (demonumber>3)
	{
		sprintf(str,"PlayDemo: Invalid demo number %d!",demonumber);
		Quit(str);
	}
#endif

	CA_CacheGrChunk(dems[demonumber]);
	demoptr = (id0_char_t *)grsegs[dems[demonumber]];
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	MM_SetLock (&grsegs[dems[demonumber]],true);
#endif
#else
	demoname[4] = '0'+demonumber;
	CA_LoadFile (demoname,&demobuffer);
	MM_SetLock (&demobuffer,true);
	demoptr = (id0_char_t id0_far *)demobuffer;
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	NewGame (1);
#else
	NewGame (1,0);
#endif
	gamestate.mapon = *demoptr++;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	gamestate.difficulty = gd_hard;
#endif
	length = *((id0_unsigned_t id0_far *)demoptr);
	demoptr += sizeof(id0_unsigned_t) + 1;
//	length = *((id0_unsigned_t id0_far *)demoptr)++;
//	demoptr++;
	lastdemoptr = demoptr-4+length;

	VW_FadeOut ();

	SETFONTCOLOR(0,15);
	DrawPlayScreen ();
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_CheckMainMem ();
#endif
	VW_FadeIn ();
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PreloadGraphics ();
	PM_UnlockMainMem ();
#endif

	startgame = false;
	demoplayback = true;

	SetupGameLevel ();
	StartMusic ();
	PM_CheckMainMem ();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	LoadFloorTiles (FloorTile[gamestate.mapon]);
	fizzlein = 3;
#else
	fizzlein = true;
#endif

	PlayLoop ();

#ifdef DEMOSEXTERN
	UNCACHEGRCHUNK(dems[demonumber]);
#else
	MM_FreePtr (&demobuffer);
#endif

	demoplayback = false;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_UnlockMainMem ();
#endif
	StopMusic ();
	VW_FadeOut ();
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	FinishPaletteShifts ();
	screenfaded = true;
#else
	ClearMemory ();
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	ResetSplitScreen ();
#endif
}

//==========================================================================

/*
==================
=
= Died
=
==================
*/

#define DEATHROTATE 2

void Died (void)
{
	float	fangle;
	id0_long_t	dx,dy;
	id0_int_t		iangle,curangle,clockwise,counter,change;

	gamestate.weapon = wp_none;			// take away weapon
	SD_PlaySound (PLAYERDEATHSND);
//
// swing around to face attacker
//
	// REFKEEN: If killerobj is 0 then emulate NULL-pointer dereferences
	if (killerobj)
	{
		dx = killerobj->x - player->x;
		dy = player->y - killerobj->y;
	}
	else
	{
		dx = 0x6F43202D - player->x;
		dy = player->y - 0x69727970;
	}

	fangle = atan2(dy,dx);			// returns -pi to pi
	if (fangle<0)
		fangle = ID0_M_PI*2+fangle;

	iangle = fangle/(ID0_M_PI*2)*ANGLES;

	if (player->angle > iangle)
	{
		counter = player->angle - iangle;
		clockwise = ANGLES-player->angle + iangle;
	}
	else
	{
		clockwise = iangle - player->angle;
		counter = player->angle + ANGLES-iangle;
	}

	curangle = player->angle;

	if (clockwise<counter)
	{
	//
	// rotate clockwise
	//
		if (curangle>iangle)
			curangle -= ANGLES;
		do
		{
			change = tics*DEATHROTATE;
			if (curangle + change > iangle)
				change = iangle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle >= ANGLES)
				player->angle -= ANGLES;

			ThreeDRefresh ();
			CalcTics ();
		} while (curangle != iangle);
	}
	else
	{
	//
	// rotate counterclockwise
	//
		if (curangle<iangle)
			curangle += ANGLES;
		do
		{
			change = -tics*DEATHROTATE;
			if (curangle + change < iangle)
				change = iangle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle < 0)
				player->angle += ANGLES;

			ThreeDRefresh ();
			CalcTics ();
		} while (curangle != iangle);
	}

//
// fade to red
//
	FinishPaletteShifts ();

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (++screenpage == 3)
		screenpage = 0;
	bufferofs = screenloc[screenpage]+screenofs;
#else
	bufferofs += screenofs;
#endif
	VW_Bar (0,0,viewwidth,viewheight,4);
	IN_ClearKeysDown ();
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,70,false);
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	bufferofs -= screenofs;
#endif
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	IN_UserInput(300);
#else
	IN_UserInput(100);
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	displayofs = bufferofs-screenofs;
	VW_SetCRTC(displayofs);
#endif
	SD_WaitSoundDone ();

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (tedlevel == false)	// SO'S YA DON'T GET KILLED WHILE LAUNCHING!
#endif
#endif // GAMEVER_NOAH3D
	  gamestate.lives--;

	if (gamestate.lives > -1)
	{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		VL_FadeOut (0,255,0,0,0,64);
		SD_WaitSoundDone ();
		SD_StopSound ();
#endif
		gamestate.health = 100;
		gamestate.weapon = gamestate.bestweapon
			= gamestate.chosenweapon = wp_pistol;
		gamestate.ammo = STARTAMMO;
		gamestate.keys = 0;
		gamestate.attackframe = gamestate.attackcount =
		gamestate.weaponframe = 0;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		gamestate.maxammo = 99;
		gamestate.gas = gamestate.missiles = 0;
		gamestate.machinegun = 0;
		gamestate.chaingun = 0;
		gamestate.flamethrower = 0;
		gamestate.missile = 0;
		gamestate.automap = false;
		DrawTreasure ();
#endif

		DrawKeys ();
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		DrawWeapon ();
#endif
		DrawAmmo ();
		DrawHealth ();
		DrawFace ();
		DrawLives ();
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		return;
#endif
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	endtics = 0;
	NewState (player,&s_gameover);
	while (endtics < 192)
	{
		UpdatePaletteShifts ();
		ThreeDRefresh ();
		CalcTics ();
	}
#endif

}

//==========================================================================

/*
===================
=
= GameLoop
=
===================
*/

void GameLoop (void)
{
	id0_int_t i,xl,yl,xh,yh;
	id0_char_t num[20];
	id0_boolean_t	died;
#ifdef MYPROFILE
	clock_t start,end;
#endif

restartgame:
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	ClearMemory ();
#endif
	SETFONTCOLOR(0,15);
	DrawPlayScreen ();
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_CheckMainMem ();
	VW_FadeIn ();
	PreloadGraphics ();
	PM_UnlockMainMem ();
#else
	died = false;
#endif
restart:
	do
	{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		endtics = 0;
		if (!died && !loadedgame)
			switch (gamestate.mapon)
			{
				case 0:  Briefing (0,false);
					break;
				case 3:  Briefing (1,false);
					break;
				case 7:  Briefing (2,false);
					break;
				case 12: Briefing (3,false);
					break;
				case 17: Briefing (4,false);
					break;
				case 23: Briefing (5,false);
					break;
			}
#endif
		// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
		if (!loadedgame)
#endif
		  gamestate.score = gamestate.oldscore;
		DrawScore();
#endif

		startgame = false;
		if (loadedgame)
			loadedgame = false;
		else
			SetupGameLevel ();

#ifdef SPEAR
		if (gamestate.mapon == 20)	// give them the key allways
		{
			gamestate.keys |= 1;
			DrawKeys ();
		}
#endif

		ingame = true;
		StartMusic ();
		PM_CheckMainMem ();
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (!died)
			PreloadGraphics ();
		else
			died = false;
#endif

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		fizzlein = 3;
#else
		fizzlein = true;
#endif
		DrawLevel ();

startplayloop:
		// REFKEEN - Alternative controllers support
		BE_ST_AltControlScheme_Push();
		PrepareGamePlayControllerMapping();

		PlayLoop ();

		// REFKEEN - Alternative controllers support
		BE_ST_AltControlScheme_Pop();
#ifdef SPEAR
		if (spearflag)
		{
			// *** SOD VERSIONS RESTORATION ***
			// Don't stop sound
#if (GAMEVER_WOLFREV > GV_WR_SODFG10)
			SD_StopSound();
#endif
			SD_PlaySound(GETSPEARSND);
			if (DigiMode != sds_Off)
			{
				// *** SOD VERSIONS RESTORATION ***
				// Do loop over DigiPlaying
#if (GAMEVER_WOLFREV <= GV_WR_SODFG10)
				while(DigiPlaying!=false)
				{
					SD_Poll();
					BE_ST_ShortSleep();
				}
#else
				id0_long_t lasttimecount = SD_GetTimeCount();

				while(SD_GetTimeCount() < lasttimecount+150)
				{
				//while(DigiPlaying!=false)
					SD_Poll();
					BE_ST_ShortSleep();
				}
#endif
			}
			else
				SD_WaitSoundDone();

			ClearMemory ();
			gamestate.oldscore = gamestate.score;
			gamestate.mapon = 20;
			SetupGameLevel ();
			StartMusic ();
			PM_CheckMainMem ();
			player->x = spearx;
			player->y = speary;
			player->angle = spearangle;
			spearflag = false;
			Thrust (0,0);
			goto startplayloop;
		}
#endif

		/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		PM_UnlockMainMem ();
#endif
		StopMusic ();
		ingame = false;

		if (demorecord && playstate != ex_warped)
			FinishDemoRecord ();

		if (startgame || loadedgame)
			goto restartgame;

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if ((gamestate.mapon == 28) &&
		    ((playstate == ex_completed) || (playstate == ex_secretlevel)))
			playstate = ex_victorious;
#endif

		switch (playstate)
		{
		case ex_completed:
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP11)
		case ex_secretlevel:
#endif
			gamestate.keys = 0;
			DrawKeys ();
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			gamestate.automap = false;
#endif
			VW_FadeOut ();

			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			SD_StopDigitized();
			screenfaded = true;
			gamestate.mapon++;
#else
			ClearMemory ();

			LevelCompleted ();		// do the intermission
#ifdef SPEARDEMO
			if (gamestate.mapon == 1)
			{
				died = true;			// don't "get psyched!"

				VW_FadeOut ();

				ClearMemory ();

				CheckHighScore (gamestate.score,gamestate.mapon+1);

				#pragma warn -sus
				#ifndef JAPAN
				strcpy(MainMenu[viewscores].string,STR_VS);
//				_fstrcpy(MainMenu[viewscores].string,STR_VS);
				#endif
				MainMenu[viewscores].routine = CP_ViewScores;
				#pragma warn +sus

				return;
			}
#endif

#ifdef JAPDEMO
			if (gamestate.mapon == 3)
			{
				died = true;			// don't "get psyched!"

				VW_FadeOut ();

				ClearMemory ();

				CheckHighScore (gamestate.score,gamestate.mapon+1);

				#pragma warn -sus
				#ifndef JAPAN
				strcpy(MainMenu[viewscores].string,STR_VS);
//				_fstrcpy(MainMenu[viewscores].string,STR_VS);
				#endif
				MainMenu[viewscores].routine = CP_ViewScores;
				#pragma warn +sus

				return;
			}
#endif

			gamestate.oldscore = gamestate.score;

			// *** S3DNA RESTORATION ***
#if (!defined SPEAR) && (!defined GAMEVER_NOAH3D)
//#ifndef SPEAR
			//
			// COMING BACK FROM SECRET LEVEL
			//
			if (gamestate.mapon == 9)
				gamestate.mapon = ElevatorBackTo[gamestate.episode];	// back from secret
			else
			//
			// GOING TO SECRET LEVEL
			//

			// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
			if ((gamestate.mapon == 0) && (player->tilex == 10))
#else
			if (playstate == ex_secretlevel)
#endif
				gamestate.mapon = 9;
#else

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define FROMSECRET1		7
#define FROMSECRET2		25
#define GAMEVER_SECRET1 11
#define GAMEVER_SECRET2 29
#else
#define FROMSECRET1		3
#define FROMSECRET2		11
#define GAMEVER_SECRET1 18
#define GAMEVER_SECRET2 19
#endif

			//
			// GOING TO SECRET LEVEL
			//
			if (playstate == ex_secretlevel)
				switch(gamestate.mapon)
				{
				 case FROMSECRET1: gamestate.mapon = GAMEVER_SECRET1; break;
				 case FROMSECRET2: gamestate.mapon = GAMEVER_SECRET2; break;
				 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				 default:
					sprintf(str,"GameLoop(): Invalid secret exit on level %d!\n",gamestate.mapon);
					Quit(str);
#endif
				}
			else
			//
			// COMING BACK FROM SECRET LEVEL
			//
			if (gamestate.mapon == GAMEVER_SECRET1 || gamestate.mapon == GAMEVER_SECRET2)
				switch(gamestate.mapon)
				{
				 case GAMEVER_SECRET1: gamestate.mapon = FROMSECRET1+1; break;
				 case GAMEVER_SECRET2: gamestate.mapon = FROMSECRET2+1; break;
				}
#endif
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			else
			//
			// SKIPPING SECRET LEVEL
			//
			if (gamestate.mapon == 10)
				gamestate.mapon = 12;
#endif
			else
			//
			// GOING TO NEXT LEVEL
			//
				gamestate.mapon++;

#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

			break;

		case ex_died:
			Died ();
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			died = true;			// don't "get psyched!"
#endif

			if (gamestate.lives > -1)
				break;				// more lives left

			VW_FadeOut ();

			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			SD_StopDigitized ();
#else
			ClearMemory ();
#endif
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			ResetSplitScreen ();
#endif

			CheckHighScore (gamestate.score,gamestate.mapon+1);

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			#pragma warn -sus
			#ifndef JAPAN
			GAMEVER_COND_FSTRCPY(MainMenu[viewscores].string,STR_VS);
			#endif
			MainMenu[viewscores].routine = CP_ViewScores;
			#pragma warn +sus
#endif

			return;

		case ex_victorious:

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			endtics = 0;
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			SD_StopDigitized ();
			SomeNullGameLoopFunc ();
#endif
#ifndef SPEAR
			VW_FadeOut ();
#else
			VL_FadeOut (0,255,0,17,17,300);
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			ClearMemory ();
#endif

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			LevelCompleted ();

			ClearMemory ();
#endif

			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			Victory ();
#endif

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			ResetSplitScreen ();
#endif
			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			ClearMemory ();
#endif

			CheckHighScore (gamestate.score,gamestate.mapon+1);

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			#pragma warn -sus
			#ifndef JAPAN
			GAMEVER_COND_FSTRCPY(MainMenu[viewscores].string,STR_VS);
			#endif
			MainMenu[viewscores].routine = CP_ViewScores;
			#pragma warn +sus
#endif

			return;

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		default:
			ClearMemory ();
			break;
#endif
		}

	} while (1);
}

REFKEEN_NS_E
