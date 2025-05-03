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

//#include <BIOS.H>
#include "bm_def.h"
//#pragma hdrstop

REFKEEN_NS_B

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

#ifdef BETA
boolean showscorebox;
#else
boolean showscorebox = true;
#endif

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

boolean godmode;
Uint16 extraVBLs;
boolean DebugOk;
boolean jumpcheat;
boolean singlestep;

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

//===========================================================================

/*
==================
=
= CountObjects
=
==================
*/

static void CountObjects(void)
{
	Uint16 activeobjects, inactiveobjects;
	objtype *ob;

	activeobjects = inactiveobjects = 0;
	for (ob=player; ob; ob=ob->next)
	{
		if (ob->active)
		{
			activeobjects++;
		}
		else
		{
			inactiveobjects++;
		}
	}
	VW_FixRefreshBuffer();
	US_CenterWindow(18, 4);
	PrintY += 7;
	US_Print("Active Objects :");
	US_PrintUnsigned(activeobjects);
	US_Print("\nInactive Objects:");
	US_PrintUnsigned(inactiveobjects);
	VW_UpdateScreen();
	IN_Ack();
}

/*
==================
=
= DebugMemory
=
==================
*/

static void DebugMemory(void)
{
#if 1
	//original code:
	VW_FixRefreshBuffer();
	US_CenterWindow(16, 7);
	US_CPrint("Memory Usage");
	US_CPrint("------------");
	US_Print("Total     :");
	US_PrintUnsigned((mminfo.mainmem+mminfo.EMSmem+mminfo.XMSmem)/1024);
	US_Print("k\nFree      :");
	US_PrintUnsigned(MM_UnusedMemory()/1024);
	US_Print("k\nWith purge:");
	US_PrintUnsigned(MM_TotalFree()/1024);
	US_Print("k\n");
	VW_UpdateScreen();
	IN_Ack();
	MM_ShowMemory();
#else
	//new code (more detailed info):
	VW_FixRefreshBuffer();
	US_CenterWindow(16, 8);
	US_CPrint("Memory Usage");
	US_CPrint("------------");
	US_Print("Total:");
	PrintX = WindowX+80;
	US_PrintUnsigned((mminfo.mainmem+mminfo.EMSmem+mminfo.XMSmem));
	US_Print("\nFree:");
	PrintX = WindowX+80;
	US_PrintUnsigned(MM_UnusedMemory());
	US_Print("\nWith purge:");
	PrintX = WindowX+80;
	US_PrintUnsigned(MM_TotalFree());
	US_Print("\nUsed:");
	PrintX = WindowX+80;
	US_PrintUnsigned((mminfo.mainmem+mminfo.EMSmem+mminfo.XMSmem)-MM_TotalFree());
	VW_UpdateScreen();
	IN_Ack();
	//MM_ShowMemory();
#endif
}

/*
===================
=
= TestSprites
=
===================
*/

static void TestSprites(void)
{
	Uint16 infox, infoy;
	Sint16 chunk, oldchunk;
	Sint16 shift;
	Uint16 infobottom, drawx;
	spritetabletype id0_far *info;
	Uint8 id0_seg *block;
	Uint16 size;
	Uint16 scan;
	Uint32 totalsize;

	VW_FixRefreshBuffer();
	US_CenterWindow(30, 17);
	totalsize = 0;
	US_CPrint("Sprite Test");
	US_CPrint("-----------");
	infoy = PrintY;
	infox = (PrintX + 56) & SCREENXMASK;
	drawx = infox + 40;
	US_Print("Chunk:\nWidth:\nHeight:\nOrgx:\nOrgy:\nXl:\nYl:\nXh:\nYh:\nShifts:\nMem:\n");
	infobottom = PrintY;
	chunk = STARTSPRITES;
	shift = 0;
	while (1)
	{
		if (chunk >= STARTSPRITES+NUMSPRITES)
		{
			chunk = STARTSPRITES+NUMSPRITES-1;
		}
		else if (chunk < STARTSPRITES)
		{
			chunk = STARTSPRITES;
		}
		info = &spritetable[chunk-STARTSPRITES];
		block = (Uint8 id0_seg *)grsegs[chunk];
		VWB_Bar(infox, infoy, 40, infobottom-infoy, WHITE);
		PrintX = infox;
		PrintY = infoy;
		US_PrintUnsigned(chunk);
		US_Print("\n");
		PrintX = infox;
		US_PrintUnsigned(info->width);
		US_Print("\n");
		PrintX = infox;
		US_PrintUnsigned(info->height);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->orgx);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->orgy);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->xl);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->yl);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->xh);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->yh);
		US_Print("\n");
		PrintX = infox;
		US_PrintSigned(info->shifts);
		US_Print("\n");
		PrintX = infox;
		if (!block)
		{
			US_Print("-----");
		}
		else
		{
			size = ((spritetype id0_far *)block)->sourceoffset[3] + ((spritetype id0_far *)block)->planesize[3]*5;
			size = (size + 15) & ~15;	//round up to multiples of 16
			totalsize += size;	//useless: the value stored in 'totalsize' is never used
			US_PrintUnsigned(size);
			US_Print("=");
		}
		oldchunk = chunk;
		do
		{
			VWB_Bar(drawx, infoy, 110, infobottom-infoy, WHITE);
			if (block)
			{
				PrintX = drawx;
				PrintY = infoy;
				US_Print("Shift:");
				US_PrintUnsigned(shift);
				US_Print("\n");
				VWB_DrawSprite(drawx + 2*shift + 16, PrintY, chunk);
			}
			VW_UpdateScreen();
			scan = IN_WaitForKey();
			switch (scan)
			{
			case sc_UpArrow:
				chunk++;
				break;
			case sc_DownArrow:
				chunk--;
				break;
			case sc_PgUp:
				chunk += 10;
				if (chunk >= STARTSPRITES+NUMSPRITES)
				{
					chunk = STARTSPRITES+NUMSPRITES-1;
				}
				break;
			case sc_PgDn:
				chunk -= 10;
				if (chunk < STARTSPRITES)
				{
					chunk = STARTSPRITES;
				}
				break;
			case sc_LeftArrow:
				if (--shift == -1)
				{
					shift = 3;
				}
				break;
			case sc_RightArrow:
				if (++shift == 4)
				{
					shift = 0;
				}
				break;
			case sc_Escape:
				return;
			}
		} while (chunk == oldchunk);
	}
}

/*
===================
=
= PicturePause
=
===================
*/

static void PicturePause(void)
{
	Uint16 offset;
	Sint16 y;

//
// wait for a key press, abort if it's not Enter
//
	IN_ClearKeysDown();
	while (!LastScan)
		BE_ST_ShortSleep();
	if (LastScan != sc_Enter)
	{
		IN_ClearKeysDown();
		return;
	}
	SD_WaitSoundDone();
	
//
// rearrange onscreen image into base EGA layout, so that it
// can be grabbed correctly by an external screenshot tool
//
	offset = displayofs + panadjust;
	VW_ColorBorder(WHITE);
	VW_SetLineWidth(40);
	VW_SetScreen(0, 0);
	if (offset < 0xCE00)
	{
	//
	// copy top line first
	//
		for (y=0; y<200; y++)
		{
			VW_ScreenToScreen(offset+y*64, y*40, 40, 1);
		}
	}
	else
	{
	//
	// copy bottom line first
	//
		for (y=199; y>=0; y--)
		{
			VW_ScreenToScreen(offset+y*64, y*40, 40, 1);
		}
	}

//
// shut down input manager so that screenshot tool can see input again
//
	IN_Shutdown();
	SD_WaitSoundDone();

//
// shut down the remaining ID managers, except VW (stay in graphics mode!)
//
	US_Shutdown();
	SD_Shutdown();
	IN_Shutdown();
	RF_Shutdown();
	CA_Shutdown();
	MM_Shutdown();

//
// wait until user hits Escape
//
	while ((BE_ST_BiosScanCode(0)) != sc_Escape)
		BE_ST_ShortSleep();
//	while (((bioskey(0) >> 8) & 0xFF) != sc_Escape);

//
// back to text mode and exit to DOS
//
	VW_Shutdown();
	BE_ST_HandleExit(0);
}

/*
===================
=
= MaskOnTile
=
===================
*/

static void MaskOnTile(Uint16 dest, Uint16 source)
{
	Sint16 i;
	Uint16 id0_seg *sourceseg;
	Uint16 id0_seg *destseg;
	Uint16 sourceval, maskindex, sourcemask;

	sourceseg = (Uint16 id0_seg *)(grsegs+STARTTILE16M)[source];
	destseg = (Uint16 id0_seg *)(grsegs+STARTTILE16M)[dest];
	for (i=0; i<64; i++)
	{
		maskindex = i & 15;
		sourceval = (sourceseg+16)[i];
		sourcemask = sourceseg[maskindex];
		destseg[maskindex] &= sourcemask;
		(destseg+16)[i] &= sourcemask;
		(destseg+16)[i] |= sourceval;
	}
}

/*
===================
=
= WallDebug
=
===================
*/

static void WallDebug(void)
{
	Sint16 i, val;

	VW_FixRefreshBuffer();
	US_CenterWindow(24, 3);
	US_PrintCentered("WORKING");
	VW_UpdateScreen();
	for (i=STARTTILE16M+108; i<STARTTILE16M+124; i++)
	{
		CA_CacheGrChunk(i);
	}
	for (i=0; i<NUMTILE16M; i++)
	{
		if (!grsegs[STARTTILE16M+i])
		{
			continue;
		}
		val = tinf[i+NORTHWALL] & SLOPEMASK;
		if (val)
		{
			MaskOnTile(i, val+107);
		}
		val = tinf[i+SOUTHWALL] & SLOPEMASK;
		if (val)
		{
			MaskOnTile(i, val+115);
		}
		val = tinf[i+EASTWALL] & SLOPEMASK;
		if (val > 1)
		{
			strcpy(str, "WallDebug: East wall other than 1:");
			BE_Cross_itoa_dec(i, str2);
			strcat(str, str2);
			Quit(str);
		}
		if (val)
		{
			MaskOnTile(i, val+114);	//Note: val is always 1 here, so you could use 115 as 2nd arg
		}
		val = tinf[i+WESTWALL] & SLOPEMASK;
		if (val > 1)
		{
			strcpy(str, "WallDebug: West wall other than 1:");
			BE_Cross_itoa_dec(i, str2);
			strcat(str, str2);
			Quit(str);
		}
		if (val)
		{
			MaskOnTile(i, val+122);	//Note: val is always 1 here, so you could use 123 as 2nd arg
		}
	}
}

//===========================================================================

/*
================
=
= DebugKeys
=
================
*/

static boolean DebugKeys(void)
{
#if (defined VERSION_1_1_FREEWARE) || (defined BETA)
	id0_char_t buffer[50];
	Sint16 level, esc;

	if (Keyboard[sc_B] && ingame)		// B = border color
	{
		VW_FixRefreshBuffer();
		US_CenterWindow(24, 3);
		PrintY += 6;
		US_Print(" Border color (0-15):");
		VW_UpdateScreen();
		esc = !US_LineInput(px, py, buffer, NULL, true, 2, 0);
		if (!esc)
		{
			level = atoi(buffer);
			if (level >= 0 && level <= 15)
			{
				VW_ColorBorder(level);
			}
		}
		return true;
	}
	if (Keyboard[sc_C] && ingame)		// C = count objects
	{
		CountObjects();
		return true;
	}
	if (Keyboard[sc_D] && ingame)		// D = start / end demo record
	{
		if (DemoMode == demo_Off)
		{
			StartDemoRecord();
		}
		else if (DemoMode == demo_Record)
		{
			EndDemoRecord();
			playstate = ex_completed;
			gamestate.mapon--;
		}
		return true;
	}
	if (Keyboard[sc_E] && ingame)		// E = quit level
	{
		if (tedlevel)
		{
			TEDDeath();
		}
		playstate = ex_completed;
		//BUG? there is no return in this branch (should return false)
	}
	if (Keyboard[sc_G] && ingame)		// G = god mode
	{
		VW_FixRefreshBuffer();
		US_CenterWindow(12, 2);
		if (godmode)
		{
			strcpy(buffer, str_godmodeoff);
//			_fstrcpy(buffer, str_godmodeoff);
			US_PrintCentered(buffer);
		}
		else
		{
			strcpy(buffer, str_godmodeon);
//			_fstrcpy(buffer, str_godmodeon);
			US_PrintCentered(buffer);
		}
		VW_UpdateScreen();
		IN_Ack();
		godmode ^= true;
		return true;
	}
	else if (Keyboard[sc_I])			// I = item cheat
	{
		gamestate.ammo = 90;
		gamestate.autofire = true;
		gamestate.grenades.green = 99;
		gamestate.keys.keycards = 99;
		gamestate.keys.keys = 99;
		gamestate.gotshard[0] = 1;
		gamestate.gotshard[1] = 1;
		gamestate.gotshard[2] = 1;
		gamestate.gotshard[3] = 1;
		gamestate.specialkeys = 1;
		gamestate.nukestate = 1;
		gamestate.got_pill = true;
		gamestate.got_exitkey = true;
#ifndef BETA
		gamestate.trianglekey = 1;
#endif
		gamestate.lives++;
	}
	else if (Keyboard[sc_J])			// J = jump cheat
	{
		jumpcheat ^= true;
		VW_FixRefreshBuffer();
		US_CenterWindow(18, 3);
		if (jumpcheat)
		{
			strcpy(buffer, str_jumpcheaton);
//			_fstrcpy(buffer, str_jumpcheaton);
			US_PrintCentered(buffer);
		}
		else
		{
			strcpy(buffer, str_jumpcheatoff);
//			_fstrcpy(buffer, str_jumpcheatoff);
			US_PrintCentered(buffer);
		}
		VW_UpdateScreen();
		IN_Ack();
		return true;
	}
	else if (Keyboard[sc_M])			// M = memory info
	{
		DebugMemory();
		return true;
	}
	else if (Keyboard[sc_N])			// N = no clip
	{
		VW_FixRefreshBuffer();
		US_CenterWindow(18, 3);
		if (player->needtoclip)
		{
			US_PrintCentered("No clipping ON");
			player->needtoclip = cl_noclip;
		}
		else
		{
			US_PrintCentered("No clipping OFF");
			player->needtoclip = cl_midclip;
		}
		VW_UpdateScreen();
		IN_Ack();
		return true;
	}
	else if (Keyboard[sc_P])			// P = pause with no screen disruption
	{
		IN_ClearKeysDown();
		PicturePause();
		return true;
	}
	else if (Keyboard[sc_S] && ingame)	// S = slow motion
	{
		singlestep ^= true;
		VW_FixRefreshBuffer();
		US_CenterWindow(18, 3);
		if (singlestep)
		{
			US_PrintCentered("Slow motion ON");
		}
		else
		{
			US_PrintCentered("Slow motion OFF");
		}
		VW_UpdateScreen();
		IN_Ack();
		return true;
	}
	else if (Keyboard[sc_T])			// T = sprite test
	{
		TestSprites();
		return true;
	}
	else if (Keyboard[sc_V])			// V = extra VBLs
	{
		VW_FixRefreshBuffer();
		US_CenterWindow(30, 3);
		PrintY += 6;
		US_Print("  Add how many extra VBLs(0-8):");
		VW_UpdateScreen();
		esc = !US_LineInput(px, py, buffer, NULL, true, 2, 0);
		if (!esc)
		{
			level = atoi(buffer);
			if (level >= 0 && level <= 8)
			{
				extraVBLs = level;
			}
		}
		return true;
	}
	else if (Keyboard[sc_W] && ingame)	// W = warp to level
	{
		VW_FixRefreshBuffer();
		US_CenterWindow(26, 3);
		PrintY += 6;
		strcpy(buffer, str_warpprompt);
//		_fstrcpy(buffer, str_warpprompt);
		US_Print(buffer);
		VW_UpdateScreen();
		esc = !US_LineInput(px, py, buffer, NULL, true, 2, 0);
		if (!esc)
		{
			level = atoi(buffer);
#ifdef BETA
			if (level > 0 && level <= 13)
#else
			if (level > 0 && level <= 24)
#endif
			{
				gamestate.mapon = level-1;
				playstate = ex_warped;
			}
		}
		return true;
	}
	else if (Keyboard[sc_Y])			// Y = wall debug
	{
		WallDebug();
		return true;
	}
	else if (Keyboard[sc_Z])			// Z = game over
	{
		gamestate.lives = 0;
		KillPlayer();
		return false;
	}
	return false;
#endif
}

//===========================================================================

/*
================
=
= UserCheat
=
================
*/

static void UserCheat(void)
{
#if (!SHAREWARE) && !(defined GMS_VERSION)
	Sint16 i;

	for (i=sc_A; i<=sc_Z; i++)	//Note: this does NOT check the keys in alphabetical order!
	{
		if (i != sc_C && i != sc_A && i != sc_T && Keyboard[i])
		{
			return;
		}
	}
	US_CenterWindow(20, 9);
	PrintY += 2;
	US_CPrint(
		"Cheat Option!\n"
		"\n"
		"You now have:\n"
		"The machine gun\n"
		" with 90 shots,\n"
		"and 99 Grenades!");
	VW_UpdateScreen();
	IN_Ack();
	RF_ForceRefresh();
	gamestate.autofire = true;
	gamestate.ammo = 90;
	gamestate.grenades.green = 99;
#endif
}

//===========================================================================

/*
=====================
=
= CheckKeys
=
=====================
*/

void CheckKeys(void)
{
	id0_char_t buffer[50];
	Sint16 esc;

	if (screenfaded)			// don't do anything with a faded screen
	{
		return;
	}

//
// Space for status screen
//
	if ((Keyboard[sc_Space] && !g_keybind_used_stats) || g_binding_value_stats)
	{
		StatusWindow();
		IN_ClearKeysDown();
		RF_ForceRefresh();
		lasttimecount = SD_GetTimeCount();	// BUG: should be the other way around
	}

//
// pause key wierdness can't be checked as a scan code
//
	if (Paused)
	{
		SD_MusicOff();
		VW_FixRefreshBuffer();
		US_CenterWindow(8, 3);
		strcpy(buffer, str_paused);
//		_fstrcpy(buffer, str_paused);
		US_PrintCentered(buffer);
		VW_UpdateScreen();
		IN_Ack();
		RF_ForceRefresh();
		Paused = false;
		SD_MusicOn();
	}
	
//
// BackSpace to toggle score box
//
	if ((LastScan == sc_BackSpace && !g_keybind_used_scorebox) || g_binding_value_scorebox)
	{
		showscorebox ^= 1;
		if (!showscorebox && scoreobj->sprite)
		{
			RF_RemoveSprite(&scoreobj->sprite);
		}
		if (showscorebox)
		{
#if 1
			//original code:
			*((Sint32 *)&scoreobj->temp1) = -1;	//force score to update (probably unsafe code)
			scoreobj->temp3 = -1;	//force grenade number & icon to update
			scoreobj->temp4 = -1;	//force lives number to update
			//Note: this does not force all parts of the score box to update, but I think forcing any updates might not even be necessary here.
#else
			//safe code:
			ResetScoreObj();
#endif
		}
		IN_ClearKeysDown();
		RF_ForceRefresh();
		lasttimecount = SD_GetTimeCount();
	}
	
//
// F1 to enter help screens
//
	if (LastScan == sc_F1)
	{
		StopMusic();
		HelpScreens();
#ifdef BETA
		StartMusic(gamestate.mapon);
#else
		StartMusic(currentmusic);
#endif
		ResetScoreObj();
		RF_ForceRefresh();
	}
	
#ifdef BETA
//
// F7 to enter QuickSave menu
//
	if (LastScan == sc_F7)
	{
		US_ControlPanel(1);
		ResetScoreObj();
		RF_ForceRefresh();
	}
	
//
// F8 to enter QuickLoad menu
//
	if (LastScan == sc_F8)
	{
		US_ControlPanel(2);
		ResetScoreObj();
		RF_ForceRefresh();
	}
#endif

	if (!storedemo)
	{
//
// F2-F7/ESC to enter control panel
//
		if (LastScan >= sc_F2 &&
#ifdef BETA
		    LastScan <= sc_F7
#else
		    LastScan <= sc_F8
#endif
		    || LastScan == sc_Escape)
		{
			VW_FixRefreshBuffer();
			StopMusic();
#ifdef BETA
			US_ControlPanel(0);
#else
			if (LastScan == sc_F7 && practiceTimeLeft == -1)
			{
				US_ControlPanel(1);
			}
			else if (LastScan == sc_F8)
			{
				US_ControlPanel(2);
			}
			else
			{
				US_ControlPanel(0);
			}
#endif
			RF_FixOfs();
			StartMusic(gamestate.mapon);
			if (!showscorebox && scoreobj->sprite)
			{
				RF_RemoveSprite(&scoreobj->sprite);
			}
			if (showscorebox)
			{
#if 1
				//original code:
				*((Sint32 *)&scoreobj->temp1) = -1;	//force score to update (probably unsafe code)
				scoreobj->temp3 = -1;	//force grenade number & icon to update
				scoreobj->temp4 = -1;	//force lives number to update
				//BUG: this does not force all parts of the score box to update (the sprite may have been re-loaded after exiting from the control panel)
#else
				//safe code/bugfix:
				ResetScoreObj();
#endif
			}
			IN_ClearKeysDown();
			if (restartgame)
			{
				playstate = ex_resetgame;
			}
			else if (!loadedgame)
			{
				RF_ForceRefresh();
			}
			if (abortgame)
			{
				abortgame = false;
				playstate = ex_abort;
			}
			if (loadedgame)
			{
				playstate = ex_loadedgame;
			}
			lasttimecount = SD_GetTimeCount();
		}

//
// F9 boss key
//
		if (LastScan == sc_F9)
		{
			// REFKEEN - Alternative controllers support
			BE_ST_AltControlScheme_Push();
			BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

			VW_Shutdown();
			SD_MusicOff();
			BE_ST_cputs("C:>");
			IN_ClearKeysDown();
			while (LastScan != sc_Escape)
				BE_ST_ShortSleep();
			VW_SetScreenMode(GRMODE);
			VW_ColorBorder(bordercolor);
			RF_ForceRefresh();
			IN_ClearKeysDown();
			lasttimecount = SD_GetTimeCount();
			SD_MusicOn();
			// REFKEEN - Alternative controllers support
			BE_ST_AltControlScheme_Pop();
		}
	}

//
// C-A-T cheat code
//
	if (Keyboard[sc_C] && Keyboard[sc_A] && Keyboard[sc_T])
	{
		UserCheat();
	}
	
#if (defined VERSION_1_1_FREEWARE) || (defined BETA)
//
// C-O-D code check for debug mode
//
	if (Keyboard[sc_C] && Keyboard[sc_O] && Keyboard[sc_D])
	{
		VW_FixRefreshBuffer();
		US_CenterWindow(17, 3);
		PrintY += 6;
		strcpy(buffer, "  CODE: ");
		//_fstrcpy(buffer, "  CODE: ");
		US_Print(buffer);
		VW_UpdateScreen();
		esc = !US_LineInput(px, py, buffer, NULL, true, 10, 0);
		if (!esc)
		{
			if (!strcmp(buffer, "91827"))
			{
#if 1
				//original code:
				debugUnlocked = true;
#else
				//HACK: because some keyboards can't handle [J]+[I]+[M] + [SPACE]
				DebugOk = debugUnlocked = true;
#endif
			}
		}
		RF_ForceRefresh();
	}
#endif

//
// F10-? debug keys
//
	if (DebugOk && Keyboard[sc_F10])
	{
		if (DebugKeys())
		{
			RF_ForceRefresh();
			lasttimecount = SD_GetTimeCount();
		}
	}

//
// Ctrl-S toggles sound (only in storedemo mode)
//
	if (storedemo && Keyboard[sc_Control] && LastScan == sc_S)
	{
		if (SoundMode != sdm_Off)
		{
			SD_SetSoundMode(sdm_Off);
			SD_SetMusicMode(smm_Off);
		}
		else
		{
			if (AdLibPresent)
			{
				SD_SetSoundMode(sdm_AdLib);
				QuietFX = false;
				SD_SetMusicMode(smm_AdLib);
			}
			else
			{
				SD_SetSoundMode(sdm_PC);
				SD_SetMusicMode(smm_Off);
			}
			CA_LoadAllSounds();
		}
	}

//
// Ctrl-Q quick quit
//
	if (Keyboard[sc_Control] && LastScan == sc_Q)
	{
		IN_ClearKeysDown();
		Quit(NULL);
	}
}

//==========================================================================

/*
==================
=
= StartDemoRecord
=
==================
*/

void StartDemoRecord(void)
{
	Sint16 level;
	boolean esc;

	VW_FixRefreshBuffer();
	US_CenterWindow(30, 3);
	PrintY += 6;
#ifdef BETA
	US_Print("  Record a demo from level(0-18):");
#else
	US_Print("  Record a demo from level(0-25):");
#endif
	VW_UpdateScreen();
	esc = !US_LineInput(px, py, str, NULL, true, 2, 0);
	if (!esc)
	{
		level = atoi(str);
#ifdef BETA
		if (level >= 0 && level <= 18)
#else
		if (level >= 0 && level <= 25)
#endif
		{
			gamestate.mapon = level;
			playstate = ex_warped;
			IN_StartDemoRecord(0x1000);
		}
	}
}

/*
==================
=
= EndDemoRecord
=
==================
*/

void EndDemoRecord(void)
{
	BE_FILE_T handle;
	boolean esc;
	id0_char_t filename[] = "DEMO?." EXTENSION;

	IN_StopDemo();
	VW_FixRefreshBuffer();
	US_CenterWindow(22, 3);
	PrintY += 6;
	US_Print("  Save as demo #(0-9):");
	VW_UpdateScreen();
	esc = !US_LineInput(px, py, str, NULL, true, 2, 0);
	if (!esc && str[0] >= '0' && str[0] <= '9')
	{
		filename[4] = str[0];
		handle = BE_Cross_open_rewritable_for_overwriting(filename);
//		handle = open(filename, O_BINARY|O_WRONLY|O_CREAT, S_IFREG|S_IREAD|S_IWRITE);
		if (!BE_Cross_IsFileValid(handle))
		{
			Quit("EndDemoRecord:  Cannot write demo file!");
		}
		BE_Cross_writeInt16LE(handle, &mapon);
		BE_Cross_writeInt16LE(handle, &DemoOffset);
		CA_FarWrite(handle, DemoBuffer, DemoOffset);
		BE_Cross_close(handle);
	}
	IN_FreeDemoBuffer();
}

REFKEEN_NS_E
