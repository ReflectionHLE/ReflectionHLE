/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2026 K1n9_Duk3
 * Copyright (C) 2025-2026 NY00123
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

boolean debugUnlocked = false;
#ifndef BETA
boolean textdrawscores = false;
//Uint16 __unused__ = 0; // REFKEEN: Unused
Uint16 oldmapon = 0;
Sint16 practiceTimeLeft = -1;
Uint16 respawnx = 0;
Uint16 respawny = 0;
Sint16 bosshealth = -1;
Sint16 oldhealthbarlength = -1;
Sint16 timeleft = -1;
boolean pirated = false;
#endif

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

Uint16 tedlevelnum;
id0_char_t str[80];
id0_char_t str2[20];
boolean storedemo;
boolean tedlevel;
#ifndef BETA
Sint16 bossdiv;
Uint16 currentmusic;
#endif

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
=====================
=
= SizeText
=
= Calculates width and height of a string that contains line breaks
= (Note: only the height is ever used, width is NOT calculated correctly)
=
=====================
*/

void SizeText(const id0_char_t *text, Uint16 *width, Uint16 *height)
{
	/*register*/ id0_char_t *ptr;
	id0_char_t c;
	Uint16 w, h;
	id0_char_t strbuf[80];

	*width = *height = 0;
	ptr = &strbuf[0];
	while ((c=*(text++)) != '\0')
	{
		*(ptr++) = c;
		if (c == '\n' || !*text)
		{
			USL_MeasureString(strbuf, NULL, &w, &h);	// BUG: strbuf may not have a terminating '\0' at the end!
			*height += h;
			if (*width < w)
			{
				*width = w;
			}
			ptr = &strbuf[0];
		}
	}
}

//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId(void)
{
	US_Shutdown();
	SD_Shutdown();
	IN_Shutdown();
	RF_Shutdown();
	VW_Shutdown();
	CA_Shutdown();
	MM_Shutdown();
}

//===========================================================================

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

static void InitGame(void)
{
	void MML_UseSpace (Uint16 segstart, Uint16 seglength);
	Uint16 segstart,seglength;

	US_TextScreen();
#ifndef BETA
	BE_ST_ToggleTextCursor(false);
	//_setcursortype(_NOCURSOR);
#endif
	MM_Startup();
	VW_Startup();
	RF_Startup();
	IN_Startup();
	SD_Startup();
	US_Startup();
	US_UpdateTextScreen();
	CA_Startup();
	US_Setup();
	US_SetLoadSaveHooks(LoadTheGame, SaveTheGame, ResetGame);
//
// load in and lock down some basic chunks
//
	CA_ClearMarks();
	CA_MarkGrChunk(STARTFONT);
	CA_MarkGrChunk(STARTTILE8);
	CA_MarkGrChunk(STARTTILE8M);
	CA_CacheMarks(NULL);
	MM_SetLock(&grsegs[STARTFONT], true);
	MM_SetLock(&grsegs[STARTTILE8], true);
	MM_SetLock(&grsegs[STARTTILE8M], true);
	fontcolor = WHITE;
	US_FinishTextScreen();
//
// reclaim the memory from the linked in text screen
//
	// REFKEEN DIFFERENCE (FIXME: Should we "fix" this at all?)
	// - Don't handle this, a bit more complicated with our setup and the
	// difference is (probably) insignificant with well-defined behaviors
	// anyway...
#if 0
	segstart = FP_SEG(&introscn);
	seglength = 4000/16;
	if (FP_OFF(&introscn))
	{
		segstart++;
		seglength--;
	}
	MML_UseSpace (segstart,seglength);
#endif
	VW_SetScreenMode(GRMODE);
	VW_ColorBorder(BLACK);
	VW_ClearVideo(BLACK);
}

//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/

void Quit(const id0_char_t *error)
{
	void *finscreen;
	//Uint16 finscreen;
	if (!error)
	{
		CA_SetAllPurge();
#ifndef BETA
		CA_CacheGrChunk(ORDERSCREEN);
		finscreen = grsegs[ORDERSCREEN];
#endif
	}
	// BUG: VW_ClearVideo may brick the system if screenseg is 0
	// (i.e., if VW_SetScreenMode has not been executed yet) - this
	// may happen if the code runs into an error during InitGame
	// (EMS/XMS errors, files not found etc.)
	VW_ClearVideo(BLACK);
	VW_SetLineWidth(40);
	ShutdownId();
	if (error && *error)
	{
		BE_ST_puts(error);
		if (tedlevel)
		{
			BE_ST_BiosScanCode(0);
//			getch();
			// REFKEEN - DISABLED
			BE_ST_ExitWithErrorMsg("Sorry, but TED5.EXE cannot be launched from game in this source port.");
//			execlp("TED5.EXE", "TED5.EXE", "/LAUNCH", NULL);
		}
		BE_ST_HandleExit(1);
	}
	if (!NoWait)
	{
#ifndef BETA
		memcpy(BE_ST_GetTextModeMemoryPtr(), finscreen, 3780);
//		movedata(finscreen, 0, 0xB800, 0, 3780);
		BE_ST_SetScreenMode(3 | 128);
//		textmode(C80);
		BE_ST_textcolor(BE_CGA_WHITE);
		BE_ST_textbackground(BE_CGA_BLACK);
#endif
		BE_ST_gotoxy(1, 24);
	}
	BE_ST_HandleExit(0);
}

//===========================================================================

/*
==================
=
= TEDDeath
=
==================
*/

void TEDDeath(void)
{
	ShutdownId();
	// REFKEEN - DISABLED
	BE_ST_ExitWithErrorMsg("Sorry, but TED5.EXE cannot be launched from game in this source port.");
	//execlp("TED5.EXE", "TED5.EXE", "/LAUNCH", NULL);
	// BUG: should call exit(1); here in case starting TED5 fails
}

//===========================================================================

/*
==================
=
= CheckMemory
=
==================
*/

#if (defined BETA)
#define MINMEMORY 310000l
#elif (defined VERSION_1_0)
#define MINMEMORY 360000l
#else
#define MINMEMORY 250000l
#endif

/*
Note:

Neither of these values is the minimum requirement to actually play the game
without encountering "out of memory" crashes. See MEMORY.TXT for details.
*/

static void CheckMemory(void)
{
	Uint8 *finscreen;
//	Uint16 finscreen;

	if (mminfo.nearheap+mminfo.farheap+mminfo.EMSmem+mminfo.XMSmem >= MINMEMORY)
	{
		return;
	}

#ifdef BETA
	ShutdownId();
	BE_ST_puts("Not enough memory to run BioHazard!");
#else
	CA_CacheGrChunk (OUTOFMEM);
	finscreen = (Uint8 *)grsegs[OUTOFMEM];
//	finscreen = (Uint16)grsegs[OUTOFMEM];
	ShutdownId();
	memcpy(BE_ST_GetTextModeMemoryPtr(),finscreen+7,3780);
//	movedata (finscreen,7,0xb800,0,3780);
	BE_ST_SetScreenMode(3 | 128);
//	textmode(C80);
	BE_ST_textcolor(BE_CGA_WHITE);
	BE_ST_textbackground(BE_CGA_BLACK);
#endif
	BE_ST_gotoxy (1,24);
	BE_ST_HandleExit(1);
}

//===========================================================================

/*
=====================
=
= DemoLoop
=
=====================
*/

static const id0_char_t *ParmStrings[] = {"easy", "normal", "hard", ""};

static void DemoLoop(void)
{
	/*register*/ Sint16 i, state;
	Sint16 level;
#ifndef BETA
	Sint16 var4 = 1;	//is initialized but never used
#if (defined VERSION_1_1)
	boolean techhelpdone = false;
#endif
#endif

//
// check for launch from ted
//
	if (tedlevel)
	{
		NewGame();
		CA_LoadAllSounds();
		gamestate.mapon = tedlevelnum;
		restartgame = gd_Normal;
		for (i = 1;i < id0_argc;i++)
		{
			if ( (level = US_CheckParm(id0_argv[i],ParmStrings)) == -1)
				continue;

			restartgame = (GameDiff)(gd_Easy+level);
			break;
		}
		GameLoop();
		TEDDeath();
	}
	
//
// demo loop
//
	state = 0;
	playstate = ex_stillplaying;
	while (1)
	{
#ifdef BETA
		StartMusic(MUS_NO_MUSIC);	// stop music
#endif
		switch (state++)
		{
#ifdef BETA
		case 0:
			TitleScreen();
			break;
		case 1:
			MiscScreen();
			break;
		case 2:
			ShowHighScores();
			state = 0;
			break;
#else
		case 0:
			RunDemo(DEMO_APOGEE);
			IN_ClearKeysDown();
			LastScan = 0;
			// For some strange reason, the tech help screen is not shown in the
			// registered version 1.1 (the one without debug cheats), even though
			// the image is present in the EGAGRAPH file.
#if ((EPISODE == 1) && (defined VERSION_1_1) && ((defined VERSION_1_1_FREEWARE) || (SHAREWARE) || (defined GMS_VERSION)))
			if (!techhelpdone)
			{
				StartMusic(MUS_NO_MUSIC);
				techhelpdone = true;
				MiscScreen(3);
			}
#endif
#if (defined GMS_VERSION)
			StartMusic(MUS_NO_MUSIC);
			MiscScreen(4);
#elif (!SHAREWARE)
			StartMusic(MUS_NO_MUSIC);
			MiscScreen(2);
#endif
			break;
		case 1:
			IN_ClearKeysDown();
			LastScan = 0;
			StartMusic(MUS_LEVEL12);
			TitleScreen(40);
			break;
		case 2:
			if (currentmusic != MUS_LEVEL12)
			{
				StartMusic(MUS_LEVEL12);
			}
			MiscScreen(1);
			break;
#if (EPISODE == 1)
		case 3:
			if (currentmusic != MUS_LEVEL12)
			{
				StartMusic(MUS_LEVEL12);
			}
			MiscScreen(0);
			break;
		case 4:
			ShowHighScores();
			break;
		case 5:
			RunDemo(DEMO_FIRST);
			break;
		case 6:
			RunDemo(DEMO_SECOND);
			state = 0;
			break;
#else
		case 4:
			ShowHighScores();
			break;
		case 5:
			state = 0;
			break;
#endif
#endif	// ifdef BETA ... else ...
		}
		CheckLastScan();
		while (playstate == ex_resetgame || playstate == ex_loadedgame)
		{
#ifdef BETA
			StartMusic(MUS_NO_MUSIC);	// stop music
#endif
			GameLoop();
			///////////////
			// this is completely useless:
			if (playstate == ex_resetgame || playstate == ex_loadedgame)
			{
				continue;
			}
			if (playstate == ex_resetgame || playstate == ex_loadedgame)
			{
				continue;
			}
			///////////////
		}
	}
}

//===========================================================================

/*
=====================
=
= CheckCutFile
=
=====================
*/

#define FILE_GR1 GREXT "1." EXTENSION
#define FILE_GR2 GREXT "2." EXTENSION
#define FILE_GRAPH GREXT "GRAPH." EXTENSION

static void CheckCutFile(void)
{
#if 0 // REFKEEN TODO: Implement?
	register Sint16 ohandle, ihandle;
	Sint16 handle;
	Sint32 size;
	void id0_far *buffer;

	if ( (handle = open(FILE_GRAPH, O_BINARY|O_RDONLY)) != -1)
	{
		close(handle);
		return;
	}
	BE_ST_puts("Combining " FILE_GR1 " and " FILE_GR2 " into " FILE_GRAPH "...");
	if (rename(FILE_GR1, FILE_GRAPH) == -1)
	{
		BE_ST_puts("Can't rename " FILE_GR1 "!");
		BE_ST_HandleExit(1);
	}
	if ( (ohandle = open(FILE_GRAPH, O_BINARY|O_APPEND|O_WRONLY)) == -1)
	{
		BE_ST_puts("Can't open " FILE_GRAPH "!");
		BE_ST_HandleExit(1);
	}
	lseek(ohandle, 0, SEEK_END);
	if ( (ihandle = open(FILE_GR2, O_BINARY|O_RDONLY)) == -1)
	{
		BE_ST_puts("Can't find " FILE_GR2 "!");
		BE_ST_HandleExit(1);
	}
	size = filelength(ihandle);
	buffer = farmalloc(32000);
	while (size)
	{
		if (size > 32000)
		{
			CA_FarRead(ihandle, buffer, 32000);
			CA_FarWrite(ohandle, buffer, 32000);
			size -= 32000;
		}
		else
		{
			CA_FarRead(ihandle, buffer, size);
			CA_FarWrite(ohandle, buffer, size);
			size = 0;
		}
	}
	farfree(buffer);
	close(ohandle);
	close(ihandle);
	unlink(FILE_GR2);
#endif
}

//===========================================================================

/*
=====================
=
= CheckDIZ
=
=====================
*/

#ifndef BETA
static boolean CheckDIZ(void)
{
	BE_FILE_T handle;
	/*register*/ Sint16 i;
	id0_char_t c1, c2;
	const id0_char_t diztext[443] = 
		"\xdb\xdb\xdb\xdb\xb2\xb1\xb0"
		       "  THIS IS PIRATED SOFTWARE!  "
		               "\xb0\xb1\xb2\xdb\xdb\xdb\xdb  "
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb  "
		"THIS FILE IS NOT SHAREWARE -- it is ILLEGAL  "
		"and forbidden to upload this copyrighted  "
		"software on bulletin boards.  If you see  "
		"this message on a BBS, please request that  "
		"the Sysop remove this software IMMEDIATELY.  "
		"\xdb\xb2\xb1\xb0"
		    " OR CONTACT APOGEE: (214) 278-5655 "
		                           "\xb0\xb1\xb2\xdb  "
		"IT IS ILLEGAL TO GET THIS FILE FROM A BBS!  "
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb"
		"\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb  ";
	id0_char_t buffer[443];

	if (!BE_Cross_IsFileValid(handle = BE_Cross_open_readonly_for_reading("file_id.diz")))
	{
		BE_Cross_close(handle);
		return true;	// file is missing, game was pirated
	}
	if (BE_Cross_FileLengthFromHandle(handle) != 442)
	{
		BE_Cross_close(handle);
		return true;	// wrong file size, game was pirated
	}
	if (BE_Cross_readInt8LEBuffer(handle, buffer, 442) < 442)
//	if (read(handle, buffer, 442) == -1)
	{
		BE_Cross_close(handle);
		return true;	// hardware error, game was pirated (seriously?)
	}
	else
	{
		BE_Cross_close(handle);
	}
	for (i=0; i<442; i++)
	{
		c1 = diztext[i];
		c2 = buffer[i];
		if (c2 != '\r' && c2 != '\n' && c1 != c2)
		{
			return true;	// not a match, game was pirated
		}
		// Note: A FILE_ID.DIZ that is 442 bytes long and consists ENTIRELY of
		// line breaks would be accepted as a valid file!
	}
	return false;	// not pirated
}
#endif

//===========================================================================


/*
==========================
=
= main
=
==========================
*/

#ifdef BETA
const id0_char_t *betaparm[] = {"slammer",""};
#elif 0 // REFKEEN: Disable
id0_char_t *betaparm[] = {"sewerman",""};	// not used in the final game
#endif

// REFKEEN: Skip that
//#include "_CTCHECK.C"	// compile-time checks for portability

void bmenace_exe_main(void)
{
// REFKEEN: Also skip that
//#include "_RTCHECK.C"	// run-time checks for portability

#ifdef BETA
	if (!g_refKeenCfg.bmenace.betaFixes) // REFKEEN: Rather check this instead.
//	if (!tedlevel)	// we haven't even run any code that could have set tedlevel to true!
	{
		int year, month, day;
//		struct date today;
		
		BE_Cross_GetLocalDate_UNSAFE(&year, &month, &day);
//		getdate(&today);
		// Note: This date check doesn't really work the way it was supposed to.
		// The da_year value returned by getdate() would have been 1992, not 92,
		// so the last part of the condition would always be true. And using an
		// AND condition like this means the date will be considered too late
		// on August 15 to August 31, but not on September 1 to September 14.
		if (day >= 15 && month >= 8 && year >= 92)
//		if (today.da_day >= 15 && today.da_mon >= 8 && today.da_year >= 92)
		{
			BE_ST_printf("This beta version has expired. Thanks for your help.");
			BE_ST_HandleExit(0);
		}
	}
#elif (!SHAREWARE)
	pirated = CheckDIZ();
#endif
	CheckCutFile();
	storedemo = false;	//Note: this is never set to any other value
#ifdef BETA
	if (!g_refKeenCfg.bmenace.betaFixes) // REFKEEN: Again, check this instead.
//	if (!tedlevel)	// we still haven't run any code that could have set tedlevel to true!
	{
		/*register*/ int i;
		
		if (id0_argc <= 1)	// need at least 1 real parameter
		{
			BE_ST_printf("You are not authorized to play this game!\n");
			BE_ST_HandleExit(0);
		}
		for (i=1; i<id0_argc; i++)
		{
			// ALL parameters passed to the game must match betaparm[0]
			if (US_CheckParm(id0_argv[i], betaparm) != 0)
			{
				BE_ST_printf("You are not authorized to play this game!\n");
				BE_ST_HandleExit(0);
			}
		}
	}
#endif
	// REFKEEN - Alternative controllers support
	extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback;
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	InitGame();
	CheckMemory();
#ifndef BETA
	gamestate._zero = 0;	//Note: this field is not used anywhere else
	BE_ST_ToggleTextCursor(true);
	//_setcursortype(_NORMALCURSOR);
#endif
	DemoLoop();
	Quit("Demo loop exited???");
}

REFKEEN_NS_E
