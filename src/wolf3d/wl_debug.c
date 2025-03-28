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

// WL_DEBUG.C

#include "wl_def.h"
//#pragma hdrstop
//#include <BIOS.H>

REFKEEN_NS_B

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWTILEX	(viewwidth/16)
#define VIEWTILEY	(viewheight/16)

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


id0_int_t DebugKeys (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


id0_int_t	maporgx;
id0_int_t	maporgy;
// REFKEEN: Define viewtype as static.
// Fixed a possible compilation warning for unnamed type.
static enum {mapview,tilemapview,actoratview,visview}	viewtype;

void ViewMap (void);

//===========================================================================

/*
==================
=
= DebugMemory
=
==================
*/

void DebugMemory (void)
{
	id0_int_t	i;
	id0_char_t    scratch[80],str[10];
	id0_long_t	mem;
	spritetype id0_seg	*block;

	CenterWindow (16,7);

	US_CPrint ("Memory Usage");
	US_CPrint ("------------");
	US_Print ("Total     :");
	US_PrintUnsigned (mminfo.mainmem/1024);
	US_Print ("k\nFree      :");
	US_PrintUnsigned (MM_UnusedMemory()/1024);
	US_Print ("k\nWith purge:");
	US_PrintUnsigned (MM_TotalFree()/1024);
	US_Print ("k\n");
	VW_UpdateScreen();
	IN_Ack ();
}

//===========================================================================

/*
==================
=
= CountObjects
=
==================
*/

void CountObjects (void)
{
	id0_int_t	i,total,count,active,inactive,doors;
	objtype	*obj;

	CenterWindow (16,7);
	active = inactive = count = doors = 0;

	US_Print ("Total statics :");
	total = laststatobj-&statobjlist[0];
	US_PrintUnsigned (total);

	US_Print ("\nIn use statics:");
	for (i=0;i<total;i++)
		if (statobjlist[i].shapenum != -1)
			count++;
		else
			doors++;	//debug
	US_PrintUnsigned (count);

	US_Print ("\nDoors         :");
	US_PrintUnsigned (doornum);

	for (obj=player->next;obj;obj=obj->next)
	{
		if (obj->active)
			active++;
		else
			inactive++;
	}

	US_Print ("\nTotal actors  :");
	US_PrintUnsigned (active+inactive);

	US_Print ("\nActive actors :");
	US_PrintUnsigned (active);

	VW_UpdateScreen();
	IN_Ack ();
}

//===========================================================================

/*
================
=
= PicturePause
=
================
*/

void PicturePause (void)
{
	id0_int_t			i;
	id0_byte_t		p;
	id0_unsigned_t	x;
	id0_byte_t		id0_far	*dest,id0_far *src;
	memptr		buffer;

	VW_ColorBorder (15);
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	FinishPaletteShifts ();
#endif

	LastScan = 0;
	while (!LastScan)
	;
	if (LastScan != sc_Enter)
	{
		VW_ColorBorder (0);
		return;
	}

	VW_ColorBorder (1);
	VW_SetScreen (0,0);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	PM_UnlockMainMem ();
#endif
//
// vga stuff...
//

	// APOGEE + FORMGEN + ALPHA RESTORATION
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	ClearMemory ();
#if (GAMEVER_WOLFREV > GV_WR_SODFG14A)
	CA_SetAllPurge();
#endif
#endif
#if 0 // TODO (REFKEEN) IMPLEMENT
	MM_GetPtr (&buffer,64000);
	for (p=0;p<4;p++)
	{
	   src = MK_FP(0xa000,displayofs);
	   dest = (id0_byte_t id0_far *)buffer+p;
	   VGAREADMAP(p);
	   // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	   for (x=0;x<12800;x++,dest+=4)
#else
	   for (x=0;x<16000;x++,dest+=4)
#endif
		   *dest = *src++;
	}


	// *** PRE-V1.4 APOGEE RESTORATION ***
	// Re-enable code for pre-v1.4 Apogee releases
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
//#if 0
	for (p=0;p<4;p++)
	{
		src = MK_FP(0xa000,0);
		dest = (id0_byte_t id0_far *)buffer+51200+p;
		VGAREADMAP(p);
		for (x=0;x<3200;x++,dest+=4)
			*dest = *src++;
	}
#endif

	asm	mov	ax,0x13
	asm	int	0x10

	dest = MK_FP(0xa000,0);
	_fmemcpy (dest,buffer,64000);

#endif // TODO (REFKEEN) IMPLEMENT
	VL_SetPalette (gamepal);


	IN_Shutdown ();

	VW_WaitVBL(70);
	BE_ST_BiosScanCode(0);
	VW_WaitVBL(70);
	Quit (NULL);
}


//===========================================================================


/*
================
=
= ShapeTest
=
================
*/

//#pragma warn -pia
void ShapeTest (void)
{
extern	id0_word_t	NumDigi;
extern	id0_word_t	id0_seg *DigiList;
static	id0_char_t	buf[10];

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_boolean_t			autopage = false;
	id0_int_t sound; // Define sound variable in S3DNA release here
#endif
	id0_boolean_t			done;
	ScanCode		scan;
	id0_int_t				i,j,k,x;
	// *** PRE-V1.4 APOGEE RESTORATION ***
	// Define sound variable in pre-v1.4 Apogee releases here
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	id0_int_t sound;
#endif
	id0_longword_t		l;
	memptr			addr;
	PageListStruct	id0_far *page;

	CenterWindow(20,16);
	VW_UpdateScreen();
	for (i = 0,done = false;!done;)
	{
		US_ClearWindow();
		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
		// Do set sound in pre-v1.4 Apogee and S3DNA releases
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11) || (defined GAMEVER_NOAH3D)
		sound = -1;
#else
//		sound = -1;
#endif

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (autopage)
			PM_GetPage(i);
#endif
		page = &PMPages[i];
		US_Print(" Page #");
		US_PrintUnsigned(i);
		if (i < PMSpriteStart)
			US_Print(" (Wall)");
		else if (i < PMSoundStart)
			US_Print(" (Sprite)");
		else if (i == ChunksInFile - 1)
			US_Print(" (Sound Info)");
		else
			US_Print(" (Sound)");

		US_Print("\n XMS: ");
		if (page->xmsPage != -1)
			US_PrintUnsigned(page->xmsPage);
		else
			US_Print("No");

		US_Print("\n Main: ");
		if (page->mainPage != -1)
			US_PrintUnsigned(page->mainPage);
		else if (page->emsPage != -1)
		{
			US_Print("EMS ");
			US_PrintUnsigned(page->emsPage);
		}
		else
			US_Print("No");

		US_Print("\n Last hit: ");
		US_PrintUnsigned(page->lastHit);

		US_Print("\n Address: ");
		addr = PM_GetPageAddress(i);
		sprintf(buf,"0x%04x",addr ? BE_Cross_GetPtrNormalizedSeg(addr) : 0);
		US_Print(buf);

		if (addr)
		{
			if (i < PMSpriteStart)
			{
			//
			// draw the wall
			//
				bufferofs += 32*SCREENWIDTH;
				postx = 128;
				postwidth = 1;
				postsource = (id0_byte_t *)addr;
				postsourceoff = 0;
//				postsource = ((id0_long_t)((id0_unsigned_t)addr))<<16;
				for (x=0;x<64;x++,postx++,postsourceoff+=64)
//				for (x=0;x<64;x++,postx++,postsource+=64)
				{
					wallheight[postx] = 256;
					FarScalePost ();
				}
				bufferofs -= 32*SCREENWIDTH;
			}
			else if (i < PMSoundStart)
			{
			//
			// draw the sprite
			//
				bufferofs += 32*SCREENWIDTH;
				SimpleScaleShape (160, i-PMSpriteStart, 64);
				bufferofs -= 32*SCREENWIDTH;
			}
			else if (i == ChunksInFile - 1)
			{
				US_Print("\n\n Number of sounds: ");
				US_PrintUnsigned(NumDigi);
				for (l = j = k = 0;j < NumDigi;j++)
				{
					l += DigiList[(j * 2) + 1];
					k += (DigiList[(j * 2) + 1] + (PMPageSize - 1)) / PMPageSize;
				}
				US_Print("\n Total bytes: ");
				US_PrintUnsigned(l);
				US_Print("\n Total pages: ");
				US_PrintUnsigned(k);
			}
			else
			{
				id0_byte_t id0_far *dp = (id0_byte_t *)addr;
//				id0_byte_t id0_far *dp = (id0_byte_t id0_far *)MK_FP(addr,0);
				for (j = 0;j < NumDigi;j++)
				{
					k = (DigiList[(j * 2) + 1] + (PMPageSize - 1)) / PMPageSize;
					if
					(
						(i >= PMSoundStart + DigiList[j * 2])
					&&	(i < PMSoundStart + DigiList[j * 2] + k)
					)
						break;
				}
				if (j < NumDigi)
				{
					// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
					// Do set sound in pre-v1.4 Apogee and S3DNA releases
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11) || (defined GAMEVER_NOAH3D)
					sound = j;
#else
//					sound = j;
#endif
					US_Print("\n Sound #");
					US_PrintUnsigned(j);
					US_Print("\n Segment #");
					US_PrintUnsigned(i - PMSoundStart - DigiList[j * 2]);
				}
				for (j = 0;j < page->length;j += 32)
				{
					id0_byte_t v = dp[j];
					id0_int_t v2 = (id0_unsigned_t)v;
					v2 -= 128;
					v2 /= 4;
					if (v2 < 0)
						VWB_Vlin(WindowY + WindowH - 32 + v2,
								WindowY + WindowH - 32,
								WindowX + 8 + (j / 32),BLACK);
					else
						VWB_Vlin(WindowY + WindowH - 32,
								WindowY + WindowH - 32 + v2,
								WindowX + 8 + (j / 32),BLACK);
				}
			}
		}

		VW_UpdateScreen();

		while (!(scan = LastScan))
		{
			SD_Poll();
			BE_ST_ShortSleep();
		}

		IN_ClearKey(scan);
		switch (scan)
		{
		case sc_LeftArrow:
			if (i)
				i--;
			break;
		case sc_RightArrow:
			if (++i >= ChunksInFile)
				i--;
			break;
		case sc_W:	// Walls
			i = 0;
			break;
		case sc_S:	// Sprites
			i = PMSpriteStart;
			break;
		case sc_D:	// Digitized
			i = PMSoundStart;
			break;
		case sc_I:	// Digitized info
			i = ChunksInFile - 1;
			break;
		case sc_L:	// Load all pages
			for (j = 0;j < ChunksInFile;j++)
				PM_GetPage(j);
			break;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		case sc_A:
			autopage = true;
			break;
#endif
		case sc_P:
			// *** S3DNA + ALPHA RESTORATION ***
			// Uncomment code for alpha version,
			// and also add code for S3DNA
#ifdef GAMEVER_NOAH3D
			if (sound != -1)
			{
				PM_GetPage(sound);
				SD_PlayDigitized(sound,0,0);
			}
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
//#else
			if (sound != -1)
				SD_PlayDigitized(sound);
#endif
			break;
		case sc_Escape:
			done = true;
			break;
		case sc_Enter:
			PM_GetPage(i);
			break;
		}
	}
	SD_StopDigitized();
}
//#pragma warn +pia



//===========================================================================


/*
================
=
= DebugKeys
=
================
*/

id0_int_t DebugKeys (void)
{
	id0_boolean_t esc;
	id0_int_t level,i;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (Keyboard[sc_A])		// A = allocation test
	{
		memptr nullblock;
		CenterWindow(12,3);
		US_PrintCentered("1k allocated");
		VW_UpdateScreen();
		IN_Ack();
		MM_GetPtr(&nullblock, 1024);
		return 1;
	}
#endif

	if (Keyboard[sc_B])		// B = border color
	{
		CenterWindow(24,3);
		PrintY+=6;
		US_Print(" Border color (0-15):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=15)
				VW_ColorBorder (level);
		}
		return 1;
	}

	if (Keyboard[sc_C])		// C = count objects
	{
		CountObjects();
		return 1;
	}

	if (Keyboard[sc_E])		// E = quit level
	{
		if (tedlevel)
			Quit (NULL);
		playstate = ex_completed;
		// *** PRE-V1.4 APOGEE RESTORATION ***
		// Do increment map in pre-v1.4 Apogee releases
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		gamestate.mapon++;
#else
//		gamestate.mapon++;
#endif
	}

	if (Keyboard[sc_F])		// F = facing spot
	{
		CenterWindow (14,4);
		US_Print ("X:");
		US_PrintUnsigned (player->x);
		US_Print ("\nY:");
		US_PrintUnsigned (player->y);
		US_Print ("\nA:");
		US_PrintUnsigned (player->angle);
		VW_UpdateScreen();
		IN_Ack();
		return 1;
	}

	if (Keyboard[sc_G])		// G = god mode
	{
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		CenterWindow (20,2);
		if (godmode)
		  US_PrintCentered ("Invulnerability OFF");
		else
		  US_PrintCentered ("Invulnerability ON");
#else
		CenterWindow (12,2);
		if (godmode)
		  US_PrintCentered ("God mode OFF");
		else
		  US_PrintCentered ("God mode ON");
#endif
		VW_UpdateScreen();
		IN_Ack();
		godmode ^= 1;
		return 1;
	}
	if (Keyboard[sc_H])		// H = hurt self
	{
		IN_ClearKeysDown ();
		TakeDamage (16,NULL);
	}
	else if (Keyboard[sc_I])			// I = item cheat
	{
		CenterWindow (12,3);
		US_PrintCentered ("Free items!");
		VW_UpdateScreen();
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		GivePoints (1000);
#else
		GivePoints (100000);
#endif
		HealSelf (99);
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		gamestate.keys = 3;
		DrawKeys ();
		gamestate.gas += 10;
		if (gamestate.gas > 99)
			gamestate.gas = 99;
		gamestate.missiles += 10;
		if (gamestate.missiles > 99)
			gamestate.missiles = 99;
		gamestate.maxammo = 299;
		gamestate.machinegun = 1;
		gamestate.chaingun = 1;
		gamestate.flamethrower = 1;
		gamestate.missile = 1;
		gamestate.automap = true;
		GiveWeapon (wp_chaingun);
		gamestate.ammo += 50;
		if (gamestate.ammo > gamestate.maxammo)
			gamestate.ammo = gamestate.maxammo;
#else
		if (gamestate.bestweapon<wp_chaingun)
			GiveWeapon (gamestate.bestweapon+1);
		gamestate.ammo += 50;
		if (gamestate.ammo > 99)
			gamestate.ammo = 99;
#endif
		DrawAmmo ();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_M])			// M = memory info
	{
		DebugMemory();
		return 1;
	}
	// *** SHAREWARE V1.0+1.1 APOGEE + S3DNA RESTORATION ***
#if (defined SPEAR) || (GAMEVER_WOLFREV <= GV_WR_WL1AP11) || (defined GAMEVER_NOAH3D)
//#ifdef SPEAR
	else if (Keyboard[sc_N])			// N = no clip
	{
		noclip^=1;
		CenterWindow (18,3);
		if (noclip)
			US_PrintCentered ("No clipping ON");
		else
			US_PrintCentered ("No clipping OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
//#if 0
	else if (Keyboard[sc_O])			// O = overhead
	{
		ViewMap();
		return 1;
	}
#endif
	else if (Keyboard[sc_P])			// P = pause with no screen disruptioon
	{
		PicturePause ();
		return 1;
	}
	else if (Keyboard[sc_Q])			// Q = fast quit
		Quit (NULL);
	else if (Keyboard[sc_S])			// S = slow motion
	{
		singlestep^=1;
		CenterWindow (18,3);
		if (singlestep)
			US_PrintCentered ("Slow motion ON");
		else
			US_PrintCentered ("Slow motion OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_T])			// T = shape test
	{
		ShapeTest ();
		return 1;
	}
	else if (Keyboard[sc_V])			// V = extra VBLs
	{
		CenterWindow(30,3);
		PrintY+=6;
		US_Print("  Add how many extra VBLs(0-8):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=8)
				extravbls = level;
		}
		return 1;
	}
	else if (Keyboard[sc_W])			// W = warp to level
	{
		CenterWindow(26,3);
		PrintY+=6;
		// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		US_Print("  Warp to which level(1-30):");
#elif (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
		US_Print("  Warp to which level(1-10):");
#else
		US_Print("  Warp to which level(1-21):");
#endif
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if (level>0 && level<31)
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			if (level>0 && level<21)
#elif (!defined SPEAR)
//#ifndef SPEAR
			if (level>0 && level<11)
#else
			if (level>0 && level<22)
#endif
			{
				gamestate.mapon = level-1;
				playstate = ex_warped;
			}
		}
		return 1;
	}
	else if (Keyboard[sc_X])			// X = item cheat
	{
		CenterWindow (12,3);
		US_PrintCentered ("Extra stuff!");
		VW_UpdateScreen();
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		gamestate.killcount = gamestate.killtotal;
		gamestate.secretcount = gamestate.secrettotal;
		gamestate.treasurecount = gamestate.treasuretotal;
		gamestate.TimeCount = 0;
#else
		// DEBUG: put stuff here
#endif
		IN_Ack ();
		return 1;
	}
	/// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	else if (Keyboard[sc_Y])			// Y = Set view size
	{
		CenterWindow (26,3);
		PrintY+=6;
		US_Print("  View tiles wide (1-19):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi(str);
			if (level>0 && level<20)
			{
				PM_UnlockMainMem ();
				SetViewSize (level*16,level*16*HEIGHTRATIO);
				for (i=0;i<3;i++)
				{
					bufferofs = screenloc[i];
					DrawPlayBorder ();
				}
				PM_CheckMainMem ();
			}
		}
		return 1;
	}
	else if (Keyboard[sc_Z])			// Z = Kill player
	{
		playstate = ex_died;
	}
		// *** S3DNA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
	else if (Keyboard[sc_Z])			// Z = Wait for key input
	{
		IN_Ack ();
		return 1;
	}
#endif

	return 0;
}


// *** PRE-V1.4 APOGEE RESTORATION ***
// Do compile this in pre-v1.4, even if it's never called
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
//#if 0
/*
===================
=
= OverheadRefresh
=
===================
*/

void OverheadRefresh (void)
{
	id0_unsigned_t	x,y,endx,endy,sx,sy;
	id0_unsigned_t	tile;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (++screenpage == 3)
		screenpage = 0;
	bufferofs = screenloc[screenpage]+screenofs;
#endif

	endx = maporgx+VIEWTILEX;
	endy = maporgy+VIEWTILEY;

	for (y=maporgy;y<endy;y++)
		for (x=maporgx;x<endx;x++)
		{
			sx = (x-maporgx)*16;
			sy = (y-maporgy)*16;

			switch (viewtype)
			{
#if 0
			case mapview:
				tile = *(mapsegs[0]+farmapylookup[y]+x);
				break;

			case tilemapview:
				tile = tilemap[x][y];
				break;

			case visview:
				tile = spotvis[x][y];
				break;
#endif
			case actoratview:
				tile = (id0_unsigned_t)actorat[x][y];
				break;
			}

			if (tile<MAXWALLTILES)
				LatchDrawTile(sx,sy,tile);
			else
			{
				LatchDrawChar(sx,sy,NUMBERCHARS+((tile&0xf000)>>12));
				LatchDrawChar(sx+8,sy,NUMBERCHARS+((tile&0x0f00)>>8));
				LatchDrawChar(sx,sy+8,NUMBERCHARS+((tile&0x00f0)>>4));
				LatchDrawChar(sx+8,sy+8,NUMBERCHARS+(tile&0x000f));
			}
		}

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	displayofs = bufferofs-screenofs;
	VW_SetScreen(displayofs,0);
#endif
}
#endif

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
// Do compile this in v1.0 of Wolfenstein 3D
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
//#if 0
/*
===================
=
= ViewMap
=
===================
*/

void ViewMap (void)
{
	id0_boolean_t		button0held;

	viewtype = actoratview;
//	button0held = false;


	maporgx = player->tilex - VIEWTILEX/2;
	if (maporgx<0)
		maporgx = 0;
	if (maporgx>MAPSIZE-VIEWTILEX)
		maporgx=MAPSIZE-VIEWTILEX;
	maporgy = player->tiley - VIEWTILEY/2;
	if (maporgy<0)
		maporgy = 0;
	if (maporgy>MAPSIZE-VIEWTILEY)
		maporgy=MAPSIZE-VIEWTILEY;

	do
	{
//
// let user pan around
//
		PollControls ();
		if (controlx < 0 && maporgx>0)
			maporgx--;
		if (controlx > 0 && maporgx<mapwidth-VIEWTILEX)
			maporgx++;
		if (controly < 0 && maporgy>0)
			maporgy--;
		if (controly > 0 && maporgy<mapheight-VIEWTILEY)
			maporgy++;

#if 0
		if (c.button0 && !button0held)
		{
			button0held = true;
			viewtype++;
			if (viewtype>visview)
				viewtype = mapview;
		}
		if (!c.button0)
			button0held = false;
#endif

		OverheadRefresh ();

	} while (!Keyboard[sc_Escape]);

	IN_ClearKeysDown ();
}
#endif

REFKEEN_NS_E
