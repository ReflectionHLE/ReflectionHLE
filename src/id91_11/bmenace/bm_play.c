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

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define INACTIVATEDIST 6

//===========================================================================

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

static Sint16 songs[] =
{
#ifdef BETA
	BIOTHEME_MUS, // 0	 6
	CRUISING_MUS, // 1	 0
	BAYOU_MUS,    // 2	 9
	BAYOU_MUS,    // 3	 9
	CHASING_MUS,  // 4	13
	SQUEAKY_MUS,  // 5	 2
	CHASING_MUS,  // 6	13
	ROBOTY_MUS,   // 7	 8
	PRISONER_MUS, // 8	 7
	MUTCMPUT_MUS, // 9	11
	CHASING_MUS,  //10	13
	WHODOL_MUS,   //11	12
	CHASING_MUS,  //12	13
	SAVED_MUS,    //13	15
	RESTING_MUS,  //14	16
	-1,           //15   n/a
	CRUISING_MUS  //16   0
#else
#if (EPISODE == 1)
	XCITY_MUS,    // 0	17		(1st number is index in this array, 2nd number is value of the enum name)
	WEASEL_MUS,   // 1	 1
	BIOTHEM1_MUS, // 2	 2
	XCITY_MUS,    // 3	17
	DIRTYH2O_MUS, // 4	 4
	ROCKINIT_MUS, // 5	 5
	CHASING_MUS,  // 6	10
	BAYOU_MUS,    // 7	 6
	ROBOTY_MUS,   // 8	 7
	PRISONER_MUS, // 9	 8
	DRSHOCK_MUS,  //10	 9
	CHASING_MUS,  //11	10
	LIKITWAS_MUS, //12	11
	//unused songs in E1: 3 (SNAKSAVE)
#elif (EPISODE == 2)
	BIOTHEME_MUS, // 0	 6
	CRUISING_MUS, // 1	 0
	BAYOU_MUS,    // 2	 9
	BAYOU_MUS,    // 3	 9
	CHASING_MUS,  // 4	13
	SQUEAKY_MUS,  // 5	 2
	CHASING_MUS,  // 6	13
	ROBOTY_MUS,   // 7	 8
	PRISONER_MUS, // 8	 7
	MUTCMPUT_MUS, // 9	11
	CHASING_MUS,  //10	13
	WHODOL_MUS,   //11	12
	CHASING_MUS,  //12	13
	//unused songs in E2: 1 (ANTSPANT), 3 (BITBYTE), 4 (SNAKSAVE), 5 (DRSHOCK), 10 (DIRTYH2O), 14 (GUESS)
#elif (EPISODE == 3)
	CRUISING_MUS, // 0	 0
	XSEWER_MUS,   // 1	 1
	XUNCRTN_MUS,  // 2	 2
	BITBYTE_MUS,  // 3	 3
	XSLUDGE_MUS,  // 4	 4
	SNAKSAVE_MUS, // 5	 5
	BIOTHEME_MUS, // 6	 6
	XSEWER_MUS,   // 7	 1
	XUNCRTN_MUS,  // 8	 2
	XCIRCLES_MUS, // 9	 9
	XSNEAKY_MUS,  //10	10
	XTHEEVIL_MUS, //11	11
	XSLUDGE_MUS,  //12	 4
	//unused songs in E3: 7 (ROCKINIT), 8 (BAYOU)
#endif
	APOGFANF_MUS, //13	varies
	SAVED_MUS,    //14	varies
	RESTING_MUS,  //15	varies
	CANTGET_MUS,  //16	varies
	NONVINCE_MUS, //17	varies
	-1,           //18	n/a
	// The following entries exist but are never actually used in the games
	// because there is no level 19 in any of the episodes and the secret levels
	// 20 to 24 are mapped to index 6 in StartMusic(). This also means CRUISING
	// is present but never actually played in episode one.
#if (EPISODE == 1)
	CRUISING_MUS, //19	 0
#endif
	CRUISING_MUS  //20	 0
#endif
//Note: ANTSPANT and GUESS are never used in any of the episodes.
};

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

ControlInfo c;
objtype *scoreobj;
Uint16 objectcount;
objtype *objfreelist;
Uint16 originxtilemax;
Uint16 originytilemax;
objtype *player;
Sint16 inactivateleft;
Sint16 inactivatetop;
boolean upheld;
objtype *newobj;
Sint16 inactivateright;
Sint16 inactivatebottom;
objtype *lastobj;
Uint16 playstate;
objtype dummyobj;
#ifndef BETA
boolean throwkeyheld;
#endif
gametype gamestate;
Uint16 centerlevel;
objtype objarray[MAXACTORS];
boolean button0held;
boolean button1held;

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
==================
=
= PrintNumbers
=
==================
*/

static void PrintNumbers(Sint16 x, Sint16 y, Sint16 maxlen, Sint16 basetile, Sint32 number)
{
	register Sint16 i;
	Sint16 len;
	id0_char_t buffer[20];

	BE_Cross_ltoa_dec(number, buffer);
	len = strlen(buffer);
	i = maxlen;
	while (i>len)
	{
		VWB_DrawTile8(x, y, basetile);
		i--;
		x += 8;
	}
	while (i>0)
	{
		VWB_DrawTile8(x, y, basetile+buffer[len-i]+(1-'0'));
		i--;
		x += 8;
	}
}

/*
==================
=
= DrawStatusWindow
=
==================
*/

static void DrawStatusWindow(void)
{
	Sint16 x, y, w, h, i;
	Uint16 width, height;
	id0_char_t buffer[50];

//
//	window frame and background
//
	x = 64;
	y = 16;
	w = 184;
	h = 144;
	VWB_DrawTile8(x, y, 11);
	VWB_DrawTile8(x, y+h, 17);
	for (i=x+8; i<=x+w-8; i+=8)
	{
		VWB_DrawTile8(i, y, 12);
		VWB_DrawTile8(i, y+h, 18);
	}
	VWB_DrawTile8(i, y, 13);
	VWB_DrawTile8(i, y+h, 19);
	for (i=y+8; i<=y+h-8; i+=8)
	{
		VWB_DrawTile8(x, i, 14);
		VWB_DrawTile8(x+w, i, 16);
	}
	VWB_Bar(72, 24, 176, 136, BE_CGA_LIGHTGRAY);
	
//
//	level name
//
	PrintY = 28;
	WindowX = 80;
	WindowW = 160;
	US_CPrint(str_location);
	VWB_Bar(79, 38, 162, 20, WHITE);
#ifdef BETA
	strcpy(buffer, levelnames[gamestate.mapon]);
//	_fstrcpy(buffer, levelnames[gamestate.mapon]);
#else
	if (gamestate.mapon < 20)
	{
		strcpy(buffer, levelnames[gamestate.mapon]);
//		_fstrcpy(buffer, levelnames[gamestate.mapon]);
	}
	else
	{
		strcpy(buffer, "Secret Level");
//		_fstrcpy(buffer, "Secret Level");
	}
#endif
	SizeText(buffer, &width, &height);
	PrintY = (20-height)/2+40-2;
	US_CPrint(buffer);
	
//
//	score
//
	PrintY = 61;
	WindowX = 80;
	WindowW = 64;
	US_CPrint(str_score);
	VWB_Bar(79, 71, 66, 10, BLACK);
	PrintNumbers(80, 72, 8, 0, gamestate.score);
	
//
// next extra at
//
	PrintY = 61;
	WindowX = 176;
	WindowW = 64;
	US_CPrint(str_extra);
	VWB_Bar(175, 71, 66, 10, BLACK);
	PrintNumbers(176, 72, 8, 0, gamestate.nextextra);
	
//
// difficulty level
//
	PrintY = 85;
	WindowX = 176;
	WindowW = 64;
	US_CPrint(str_level);
	VWB_Bar(175, 95, 66, 10, WHITE);
	PrintY = 96;
	WindowX = 176;
	WindowW = 64;
	switch (gamestate.difficulty)
	{
	case gd_Easy:
		US_CPrint(str_leasy);
		break;
	case gd_Normal:
		US_CPrint(str_lnormal);
		break;
	case gd_Hard:
		US_CPrint(str_lhard);
		break;
	}
	
//
//	crystal shard keys
//
	PrintX = 80;
	PrintY = 96;
	strcpy(buffer, str_shards);
//	_fstrcpy(buffer, str_shards);
	US_Print(buffer);
	VWB_Bar(127, 95, 34, 10, BLACK);
	if (gamestate.gotshard[0])
	{
		VWB_DrawTile8(128, 96, 24);
	}
	if (gamestate.gotshard[1])
	{
		VWB_DrawTile8(136, 96, 25);
	}
	if (gamestate.gotshard[2])
	{
		VWB_DrawTile8(144, 96, 26);
	}
	if (gamestate.gotshard[3])
	{
		VWB_DrawTile8(152, 96, 27);
	}
	
//
//	generic keys
//
	PrintX = 80;
	PrintY = 112;
	VWB_Bar(79, 111, 34, 10, BLACK);
	VWB_DrawTile8(84, 112, 22);
	PrintNumbers(96, 112, 2, 0, gamestate.keys.keys);
	
//
// key cards
//
	PrintX = 128;
	PrintY = 112;
	VWB_Bar(127, 111, 34, 10, BLACK);
	VWB_DrawTile8(132, 112, 23);
	PrintNumbers(144, 112, 2, 0, gamestate.keys.keycards);
	
#ifdef BETA
//
//	food
//
	PrintX = 184;
	PrintY = 112;
	strcpy(buffer, str_food);
//	_fstrcpy(buffer, str_food);
	US_Print(buffer);
	VWB_Bar(215, 111, 18, 10, BLACK);
	PrintNumbers(216, 112, 2, 0, gamestate.food);
	
//
//	special items
//
	PrintX = 168;
	PrintY = 128;
	US_Print("ITEMS");
	VWB_Bar(207, 127, 34, 10, BLACK);
	if (gamestate.specialkeys)
	{
		VWB_DrawTile8(208, 128, 28);
	}
	if (gamestate.nukestate)
	{
		VWB_DrawTile8(216, 128, 29);
	}
	if (gamestate.got_pill)
	{
		VWB_DrawTile8(224, 128, 30);
	}
	if (gamestate.got_exitkey)
	{
		VWB_DrawTile8(232, 128, 32);
	}
	
//
//	potions
//
	PrintX = 80;
	PrintY = 128;
	strcpy(buffer, str_potions);
//	_fstrcpy(buffer, str_potions);
	US_Print(buffer);
	VWB_Bar(135, 127, 26, 10, BLACK);
	PrintNumbers(144, 128, 2, 0, gamestate.potions);
	
#else
	
//
//	gems
//
	PrintX = 184;
	PrintY = 112;
	strcpy(buffer, str_gems);
//	_fstrcpy(buffer, str_gems);
	US_Print(buffer);
	VWB_Bar(215, 111, 18, 10, BLACK);
	PrintNumbers(216, 112, 2, 0, gamestate.gems);
	PrintX = 152;
	PrintY = 128;
	
//
//	special items
//
	US_Print("ITEMS");
	VWB_Bar(191, 127, 50, 10, BLACK);
	if (gamestate.trianglekey)
	{
		VWB_DrawTile8(192, 128, 56);
	}
	if (gamestate.specialkeys)
	{
		VWB_DrawTile8(200, 128, 28);
	}
	if (gamestate.nukestate == 1)	// nuke collected but not placed yet
	{
		VWB_DrawTile8(208, 128, 29);
	}
	if (gamestate.got_pill)
	{
		VWB_DrawTile8(216, 128, 30);
	}
	if (gamestate.got_exitkey)
	{
		VWB_DrawTile8(224, 128, 32);
	}
	if (gamestate.got_warpgem)
	{
		VWB_DrawTile8(232, 128, 65);
	}

//
//	clips
//
	PrintX = 80;
	PrintY = 128;
	strcpy(buffer, str_clips);
//	_fstrcpy(buffer, str_clips);
	US_Print(buffer);
	VWB_Bar(119, 127, 26, 10, BLACK);
	if (gamestate.autofire == true && gamestate.ammo > 3)
	{
		PrintNumbers(128, 128, 2, 0, gamestate.clips+1);
	}
	else
	{
		PrintNumbers(128, 128, 2, 0, gamestate.clips);
	}
#endif	// ifdef BETA ... else ...

//
//	draw tiles for the "Press a Key" image
//
	for (i=0; i<10; i++)
	{
		VWB_DrawTile8((i+15)*8, 140, i+36);
		VWB_DrawTile8((i+15)*8, 148, i+46);
	}
}

/*
==================
=
= StatusWindow
=
==================
*/

void StatusWindow(void)
{
	if (debugUnlocked && Keyboard[sc_J] && Keyboard[sc_I] && Keyboard[sc_M])
	{
		US_CenterWindow(20, 2);
		PrintY += 2;
		US_Print(" Debug keys active!");
		VW_UpdateScreen();
		IN_Ack();
		DebugOk = true;
	}
	WindowX = 0;
	WindowW = 320;
	WindowY = 0;
	WindowH = 200;
	DrawStatusWindow();
	VW_UpdateScreen();
	IN_ClearKeysDown();
	IN_Ack();
}

//===========================================================================

/*
==================
=
= CenterActor
=
==================
*/

void CenterActor(objtype *ob)
{
	Uint16 orgx, orgy;

	centerlevel = 140;
	if (ob->x < 152*PIXGLOBAL)
	{
		orgx = 0;
	}
	else
	{
		orgx = ob->x - 152*PIXGLOBAL;
	}
	if (ob->bottom < 140*PIXGLOBAL)
	{
		orgy = 0;
	}
	else
	{
		orgy = ob->bottom-140*PIXGLOBAL;
	}
	if (!scorescreenkludge)
	{
		RF_NewPosition(orgx, orgy);
	}
	
//
// calculate new inactivation limits
//
	originxtilemax = originxtile+PORTTILESWIDE-1;
	originytilemax = originytile+PORTTILESHIGH-1;
	inactivateleft = originxtile-INACTIVATEDIST;
	if (inactivateleft < 0)
	{
		inactivateleft = 0;
	}
	inactivateright = originxtilemax+INACTIVATEDIST;
	if (inactivateright < 0)
	{
		inactivateright = 0;
	}
	inactivatetop = originytile-INACTIVATEDIST;
	if (inactivatetop < 0)
	{
		inactivatetop = 0;
	}
	inactivatebottom = originytilemax+INACTIVATEDIST;
	if (inactivatebottom < 0)
	{
		inactivatebottom = 0;
	}
}

//===========================================================================

/*
==================
=
= ScrollScreen
=
= Scroll if Snake is nearing an edge
= Set playstate to ex_completed
=
==================
*/

static void ScrollScreen(objtype *ob)
{
	Sint16 xscroll, yscroll, pix, speed;
	Uint16 bottom;

#ifndef BETA
//
// don't scroll the screen in the Apogee intro level
//
	if (gamestate.mapon == 13)
	{
		return;
	}
#endif

//
// walked off edge of map?
//
	if (ob->left < originxmin || ob->right > originxmax + 20*TILEGLOBAL)
	{
		playstate = ex_completed;
		return;
	}

//
// fallen off bottom of world?
//
	if (ob->bottom > originymax + 13*TILEGLOBAL && ob->health > 0)
	{
		ob->y -= ob->bottom - (originymax + 13*TILEGLOBAL);
		godmode = false;
		HurtPlayer(ob, 50);
		return;
	}
	
	xscroll=yscroll=0;
	if (ob->x < originxglobal + 8*TILEGLOBAL)
	{
		xscroll = ob->x - (originxglobal + 8*TILEGLOBAL);
	}
	else if (ob->x > originxglobal + 10*TILEGLOBAL)
	{
		xscroll = ob->x - (originxglobal + 10*TILEGLOBAL);
	}

//
// handle "looking" up or down
//
	if ((Keyboard[sc_PgUp] && !g_keybind_used_lookup) ||
            (Keyboard[sc_RShift] && c.yaxis == -1) || g_binding_value_lookup)
	{
		if (centerlevel+tics > 157)
		{
			pix = 157-centerlevel;
		}
		else
		{
			pix = tics;
		}
		centerlevel += pix;
		yscroll = CONVERT_PIXEL_TO_GLOBAL(-pix);
	}
	else if ((Keyboard[sc_PgDn] && !g_keybind_used_lookdown) ||
                 (Keyboard[sc_RShift] && c.yaxis == 1) || g_binding_value_lookdown)
	{
		if (centerlevel-tics < 43)
		{
			pix = centerlevel+-43;
		}
		else
		{
			pix = tics;
		}
		centerlevel -= pix;
		yscroll = CONVERT_PIXEL_TO_GLOBAL(pix);
	}
	
//
// keep player at centerlevel on screen while player is walking or standing
//
	if (ob->state->contactptr == SnakeStandContact)
	{
		yscroll += ob->ymove;
		bottom = originyglobal + yscroll + CONVERT_PIXEL_TO_GLOBAL(centerlevel);
		if (bottom == ob->bottom)
		{
			goto scroll;
		}
		if (ob->bottom < bottom)
		{
			pix = bottom - ob->bottom;
		}
		else
		{
			pix = ob->bottom - bottom;
		}
		speed = CONVERT_PIXEL_TO_GLOBAL(pix) >> 7;
		if (speed > 0x30)
		{
			speed = 0x30;
		}
		speed *= tics;
		if (speed < 0x10)
		{
			if (pix < 0x10)
			{
				speed = pix;
			}
			else
			{
				speed = 0x10;
			}
		}
		if (ob->bottom < bottom)
		{
			yscroll -= speed;
		}
		else
		{
			yscroll += speed;
		}
	}
	else
	{
		centerlevel = 140;
	}
scroll:
//
// don't scroll too far up or down
//
	pix = (ob->bottom-32*PIXGLOBAL)-(originyglobal+yscroll);
	if (pix < 0)
	{
		yscroll += pix;
	}
	pix = (ob->bottom+32*PIXGLOBAL)-(originyglobal+yscroll+200*PIXGLOBAL);
	if (pix > 0)
	{
		yscroll += pix;
	}
	
	if (xscroll != 0 || yscroll != 0)
	{
	//
	// don't scroll more than one tile per refresh
	//
		if (xscroll >= TILEGLOBAL)
		{
			xscroll = TILEGLOBAL-1;
		}
		else if (xscroll <= -TILEGLOBAL)
		{
			xscroll = -(TILEGLOBAL-1);
		}
		if (yscroll >= TILEGLOBAL)
		{
			yscroll = TILEGLOBAL-1;
		}
		else if (yscroll <= -TILEGLOBAL)
		{
			yscroll = -(TILEGLOBAL-1);
		}
		RF_Scroll(xscroll, yscroll);
		
	//
	// calculate new inactivation limits
	//
		originxtilemax = originxtile+PORTTILESWIDE-1;
		originytilemax = originytile+PORTTILESHIGH-1;
		inactivateleft = originxtile-INACTIVATEDIST;
		if (inactivateleft < 0)
		{
			inactivateleft = 0;
		}
		inactivateright = originxtilemax+INACTIVATEDIST;
		if (inactivateright < 0)
		{
			inactivateright = 0;
		}
		inactivatetop = originytile-INACTIVATEDIST;
		if (inactivatetop < 0)
		{
			inactivatetop = 0;
		}
		inactivatebottom = originytilemax+INACTIVATEDIST;
		if (inactivatebottom < 0)
		{
			inactivatebottom = 0;
		}
	}
}

//===========================================================================


/*
#############################################################################

				  The objarray data structure

#############################################################################

Objarray contains structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawns another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/

// The comment above was taken directly from the Keen Dreams source code. The
// line about the <backwardly linked free list> just means that the objfreelist
// uses the prev pointer to link to the next free object. This allows the real
// next pointer to remain intact as described above.


/*
=========================
=
= InitObjArray
=
= Call to clear out the entire object list, returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/

void InitObjArray(void)
{
	Sint16 i;

	for (i=0; i<MAXACTORS; i++)
	{
		objarray[i].prev = &objarray[i+1];
		objarray[i].next = NULL;
	}
	objarray[MAXACTORS-1].prev = NULL;
	objfreelist = &objarray[0];
	lastobj = NULL;
	objectcount = 0;
//
// give the player and score the first free spots
//
	GetNewObj(false);
	player = newobj;
	GetNewObj(false);
	scoreobj = newobj;
}

//===========================================================================

/*
=========================
=
= GetNewObj
=
= Sets the global variable new to point to a free spot in objarray.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out or
= return a dummy object pointer that will never get used
=
= Returns -1 when list was full, otherwise returns 0.
=
=========================
*/

Sint16 GetNewObj(boolean usedummy)
{
	if (!objfreelist)
	{
		if (usedummy)
		{
			newobj = &dummyobj;
			return -1;
		}
		Quit("GetNewObj: No free spots in objarray!");
	}
	newobj = objfreelist;
	objfreelist = newobj->prev;
	memset(newobj, 0, sizeof(*newobj));
	if (lastobj)
	{
		lastobj->next = newobj;
	}
	newobj->prev = lastobj;
	newobj->active = yes;
	newobj->needtoclip = cl_midclip;
	lastobj = newobj;
	objectcount++;
	return 0;
}

//===========================================================================

/*
=========================
=
= RemoveObj
=
= Add the given object back into the free list, and unlink it from it's
= neighbors
=
=========================
*/

void RemoveObj(objtype *ob)
{
#ifndef BETA
	if (ob == player && gamestate.mapon == 13)
	{
		return;
	}
#endif
	if (ob == player)
	{
		Quit("RemoveObj: Tried to remove the player!");
	}
	RF_RemoveSprite(&ob->sprite);
	if (ob == lastobj)
	{
		lastobj = ob->prev;
	}
	else
	{
		ob->next->prev = ob->prev;
	}
	ob->prev->next = ob->next;
	ob->prev = objfreelist;
	objfreelist = ob;
}

//==========================================================================

/*
====================
=
= GivePoints
=
= Grants extra men at 20k,40k,80k,160k,320k
=
====================
*/

void GivePoints(Uint16 points)
{
	gamestate.score += points;
	if (!DemoMode && gamestate.score >= gamestate.nextextra)
	{
		gamestate.lives++;
		gamestate.nextextra *= 2;
	}
}

//==========================================================================


/*
=================
=
= StopMusic
=
=================
*/

void StopMusic(void)
{
	//Yes, this was empty. And yes, leaving this empty causes issues.
#ifdef FIX_MUSIC_MEMORY_ISSUES
	//bugfix:
	Uint16 i;

	SD_MusicOff();
	for (i=STARTMUSIC; i<STARTMUSIC+LASTMUSIC; i++)
	{
		if (audiosegs[i])
		{
			MM_SetPurge(&(memptr)audiosegs[i],3);
			MM_SetLock(&(memptr)audiosegs[i],false);
		}
	}
#endif
}

//==========================================================================

/*
=================
=
= StartMusic
=
=================
*/

void StartMusic(Uint16 num)
{
	Sint16 song;
	boolean wasfaded;

#ifndef BETA
	currentmusic = num;
	if (num >= 20 && num <= 24)
	{
		num = 6;
	}
#endif
	if (num >= ARRAYLENGTH(songs) && num != 0xFFFF)
	{
		Quit("StartMusic() - bad level number");
	}
#ifdef FIX_MUSIC_MEMORY_ISSUES
	//Since we cannot rely on the game calling StopMusic(), let's do
	//that here to free up some memory for the new song.
	StopMusic();
#else
	//original code:
	SD_MusicOff();
#endif
	song = songs[num];
	if (song == -1 || MusicMode != smm_AdLib)
	{
		return;
	}
	MM_BombOnError(false);
	CA_CacheAudioChunk(STARTMUSIC+song);
	MM_BombOnError(true);
	if (mmerror)
	{
		mmerror = false;
		if (!DemoMode)
		{
			US_CenterWindow(20, 8);
			PrintY += 20;
			US_CPrint(str_nomemmusic);
			VW_UpdateScreen();
			wasfaded = screenfaded;
			if (wasfaded)
			{
				VW_SetDefaultColors();
			}
			IN_ClearKeysDown();
			IN_UserInput(3*TickBase, false);
			if (wasfaded)
			{
				VW_FadeOut();
			}
		}
	}
	else
	{
#ifdef FIX_MUSIC_MEMORY_ISSUES
		//The current music should be locked, so the memory manager will not
		//mess with it when compressing memory blocks in MM_SortMem().
		MM_SetLock(&(memptr)audiosegs[STARTMUSIC+song],true);
#endif
		SD_StartMusic((MusicGroup id0_far *)audiosegs[STARTMUSIC+song]);
	}
}

//==========================================================================


/*
===================
=
= PlayLoop
=
===================
*/

void PlayLoop(void)
{
	objtype *ob, *ob2;
#ifndef BETA
	Uint16 var_2 = 25;	//variable is never used again
#endif

	StartMusic(gamestate.mapon);
	ingame = true;
	playstate = ex_stillplaying;
#ifdef BETA
	upheld = button0held = button1held = false;
#else
	upheld = button0held = button1held = throwkeyheld = false;
#endif
	CenterActor(player);
	if (DemoMode)
	{
		US_InitRndT(false);
	}
	else
	{
		US_InitRndT(true);
	}
	SD_SetTimeCount(3);
	lasttimecount = tics = 3;
	ResetScoreObj();
	do
	{
#ifndef BETA
		cooldown--;
#endif
		PollControls();
		///////////////////////////
		// beta leftovers (shuttlecooldown is always 0 in the final game)
		if (shuttlecooldown > 0)
		{
			shuttlecooldown--;
			if (shuttlecooldown <= 0)
			{
				shuttlecooldown = 0;	//useless: variable is unsigned, so it will be zero in this branch
			}
		}
		//////////////////////////
		
//
// go through state changes and propose movements
//
		for (ob=player; ob; ob=ob->next)
		{
			//
			// activate any inactive objects that got scrolled into view
			//
			if (!ob->active && ob->tileright >= originxtile-1
				&& ob->tileleft <= originxtilemax+1 && ob->tiletop <= originytilemax+1
				&& ob->tilebottom >= originytile-1)
			{
				ob->needtoreact = true;
				ob->active = yes;
			}
			if (ob->active)
			{
				//
				// deactivate objects that are too far out of view
				//
				if (ob->tileright < inactivateleft
					|| ob->tileleft > inactivateright
					|| ob->tiletop > inactivatebottom
					|| ob->tilebottom < inactivatetop)
				{
					if (ob->active == removable)
					{
						RemoveObj(ob);				// temp thing (shots, etc)
						continue;
					}
					else if (ob->active != allways)
					{
						if (US_RndT() < tics*2 || screenfaded || loadedgame)
						{
							RF_RemoveSprite(&ob->sprite);
							ob->active = no;
							continue;
						}
					}
				}
				//
				// update the object
				//
				StateMachine(ob);
			}
		}
		if (gamestate.riding)
		{
			HandleRiding(player);
		}
//
// check for and handle collisions between objects
//
		for (ob=player; ob; ob=ob->next)
		{
			if (ob->active)
			{
				for (ob2=ob->next; ob2; ob2=ob2->next)
				{
					if (ob2->active)
					{
						if (ob->right > ob2->left && ob->left < ob2->right
							&& ob->top < ob2->bottom && ob->bottom > ob2->top)
						{
							if (ob->state->contactptr)
							{
								ob->state->contactptr(ob, ob2);
							}
							if (ob2->state->contactptr)
							{
								ob2->state->contactptr(ob2, ob);
							}
							if (ob->obclass == nothing)	//useless -- obclass is NOT set to nothing by RemoveObj
							{
								break;
							}
						}
					}
				}
			}
		}
//
// check intiles
//
		CheckInTiles(player);	//this is called a second time below
//
// react to whatever happened, and post sprites to the refresh manager
//
		for (ob=player; ob; ob=ob->next)
		{
			if (!ob->active)
			{
				continue;
			}
			// Let the object's white damage flash expire over time:
			if (ob->white)
			{
				ob->white -= tics;
				if (ob->white < 0)
				{
					ob->white = 0;
				}
				// BUG: This should set needtoreact to true when white reaches 0 to
				// make sure the object gets redrawn in the normal draw mode.
			}
			if (ob->tilebottom >= mapheight-1)
			{
				if (ob == player)
				{
					playstate = ex_died;
				}
				else
				{
					RemoveObj(ob);
				}
				continue;
			}
			if (ob->needtoreact && ob->state->reactptr)
			{
				ob->needtoreact = false;
				ob->state->reactptr(ob);
			}
		}
//
// make invincibility tick down
//
		if (invincibility)
		{
			// The duration of the temporary invincibility depends on the frame
			// rate in BioMenace. That means the 50 frames of mercy invincibility
			// after getting hit could last between 1.4 seconds (at 35 fps) and 3.5
			// seconds (at 14 fps). The invinciblility potion will last between
			// 42.8 seconds and 107.1 seconds. Playing on a slow system is a clear
			// advantage when it comes to BioMenace's invincibility system.
			//
			// This problem would DEFINITELY need to be addressed if you want to
			// make the game run at up to 70 fps instead of just 35 fps.
			//
			// Fixing this will require some changes. The easiest solution would be
			// to use "invincibility -= tics;" instead of "invincibility--;", but
			// that would cause the invincibility to run out at least twice as fast
			// as intended. You would have to double (or triple, if you want) the
			// invincibility values everywhere else in the code to compensate for
			// that, even in the music-related check below. Also make sure that you
			// do NOT let invincibility underflow! Or just turn it into a signed
			// variable to avoid that problem.
			//
			// Another solution could be to decrease invincibility only every other
			// tic, much like what the DoGravity() function in BM_STATE.C does
			// for acceleration. But that requires extra code. And you still might
			// want to balance the invincibility durations a bit.
			//
			// If you decide to fix this, the following check would of course need
			// to use a different condition to make sure the player sprite only
			// gets drawn every other frame. When you do this, make sure that the
			// RF_PlaceSprite branch gets used here when invincibility reaches 0,
			// so that you won't end up leaving the player invisible at the end.
			// Adding a flash variable at the beginning of the PlayLoop function
			// and using "if (invincibility > 0 && ++flash & 1)" as the condition
			// below should give you what you need.
			
			invincibility--;	//BUG: invincibility duration depends on frame rate!
			if (invincibility & 1)
			{
				RF_RemoveSprite(&player->sprite);
			}
			else
			{
				RF_PlaceSprite(&player->sprite, player->x, player->y, player->shapenum, spritedraw, 1);
			}
#ifndef BETA
			// Switch back to the regular level music when the invincibility is
			// about to run out:
			if ((invincibility > 100 && invincibility < 102) || (invincibility < 100 && currentmusic != gamestate.mapon))
			{
				StartMusic(gamestate.mapon);
			}
			// BUG: This will also cancel the victory music if the player had some
			// invincibility when the boss died or if the player takes damage or
			// uses the secret invincibility move any time after the boss died.
#endif
			if (invincibility <= 0)	//useless code (variable is unsigned)!
			{
				invincibility = 0;
			}
		}
//
// check intiles
//
		if (player->health > 0)
		{
			CheckInTiles(player);	//double check in case reactptr() moved the player, I guess...
		}
		
		// I'm not quite sure what the following code was supposed to accomplish.
		// The player object's temp7 field is not used anywhere else in the code.
		if (player->temp7 > 0)
		{
			player->temp7--;
			if (player->temp7 <= 0)
			{
				player->temp7 = 0;
			}
		}
//
// scroll the screen and update the score box
//
		ScrollScreen(player);
		UpdateScorebox(scoreobj);
		if (loadedgame)
		{
			loadedgame = false;
		}
//
// update the screen and calculate the number of tics it took to execute
// this cycle of events (for adaptive timing of next cycle)
//
		RF_Refresh();
//
// single step debug mode
//
		if (singlestep)
		{
			VW_WaitVBL(14);	//reduces framerate to 5 fps on VGA or 4.3 fps on EGA cards
			lasttimecount = SD_GetTimeCount();
		}
//
// extra VBLs debug mode
//
		if (extraVBLs)
		{
			VW_WaitVBL(extraVBLs);
		}
//
// handle user inputs
//
		if (DemoMode == demo_Playback)
		{
			if (
#ifndef BETA
			    gamestate.mapon != 13 &&	// ignore user input during Apogee intro...
			    gamestate.mapon != 24 &&	// ...and during secret level transition
#endif
			    !screenfaded && IN_IsUserInput())
			{
				playstate = ex_completed;
				if (LastScan != sc_F1)
				{
					LastScan = sc_Space;
				}
			}
		}
		else if (DemoMode == demo_PlayDone)
		{
				playstate = ex_completed;
		}
		else
		{
			CheckKeys();
		}
#ifndef BETA
//
// handle practice mode timer countdown
//
		if (practiceTimeLeft == 0)
		{
			practiceTimeLeft = -1;
#if 1
			//original code: (buggy)
			NewGame();
			gamestate.mapon = oldmapon;
			playstate = ex_resetgame;
			gamestate.score = 0;
			US_ControlPanel(0);	//BUG: this should be called at the end of this branch
			ingame = false;
			GameIsDirty = false;
#else
			//bugfix:
			ingame = false;
			GameIsDirty = false;
			playstate = ex_abort;	//this makes sure that "Return to Demo" will actually return to the demo
			US_ControlPanel(0);
#endif
		}
		else if (practiceTimeLeft > 0)
		{
			practiceTimeLeft--;	//BUG: practice mode duration depends on frame rate!
			// See the invincibility section above for possible ways to fix this.
		}
#endif
	} while (playstate == ex_stillplaying);
	ingame = false;
	StopMusic();
}

REFKEEN_NS_E
