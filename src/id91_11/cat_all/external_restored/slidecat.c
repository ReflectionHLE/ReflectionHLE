/* Catacomb Apocalypse Source Code
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

/* Reconstructed from the DEMOCAT.EXE file bundled with The Catacomb
 * Abyss Shareware v1.13, using available source files; Then modified to
 * optionally behave like any of the HINTCAT.EXE files from the last retail
 * release (Abyss v1.24, Armageddon v1.02, Apocalypse v1.01).
 */

//#include <conio.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#include "ext_heads.h"
#include "ext_gelib.h"

REFKEEN_NS_B

// File names
// REFKEEN - Dynamically set SCRIPT_FILENAME by version, and let END_SCREEN always be defined
//#ifdef GAMEVER_SHAREWARE
#define END_SCREEN "LAST.CAT"
//#define SCRIPT_FILENAME "SCRIPT.CAT"
//#else
//#define SCRIPT_FILENAME "SCRIPT.HNT"
//#endif
static const id0_char_t *SCRIPT_FILENAME;

// Misc. text definitions
// REFKEEN - Also define these macros as strings we dynamically set before entering main function
static const id0_char_t *FRAMETOP_STR, *TITLE_STR, *AUTHOR_STR, *COPYRIGHT_STR, *FRAMEBOT_STR;
static const id0_char_t *VERSION_TITLE_STR, *VERSION_REV_STR;

// REFKEEN - Re-use functions from intro.c
#if 0
void SetScreenMode (id0_int_t mode);
void SetLineWidth (id0_int_t width);
static id0_boolean_t IsKeyPressed (void);
void WaitForKeyRelease (void);
#endif
static void Beep (void);
// REFKEEN - Add this prototype
static id0_int_t CheckParm(const id0_char_t *parm,const id0_char_t **strings);

id0_int_t screenmode;
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
id0_long_t pg_t_end, pg_t_str;
#endif
id0_int_t pg_curr;

id0_byte_t id0_far *pg_scr[30];

static id0_boolean_t havebeep = true;
static id0_int_t pg_last = 0;
#ifdef GAMEVER_CATABYSS // REFKEEN - Define for CATABYSS only
static id0_boolean_t returntointro = false;
static id0_boolean_t returntostart = false;
#endif

// REFKEEN - HACK - Make this static so there's no conflict with variable from id_vw.c
static cardtype videocard;

struct Shape page_shapes[30];

#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
static const id0_char_t *ParmStrings_Shar[] = {"auto","ver","s",NULL};
#endif
//#else
static const id0_char_t *ParmStrings_Reg[] = {"ver","s",NULL};
//#endif
static const id0_char_t **ParmStrings; // REFKEEN - Fill this based on version

#ifdef GAMEVER_CATABYSS
// REFKEEN - New function passed to BE_Cross_Bexecv
static void slidecat_exe_main_reverter(void)
{
	// Skip "constant" version-specific vars like refkeen_compat_intro_frametop_str
	screenmode = 0;
	pg_t_end = pg_t_str = 0;
	pg_curr = 0;
	pg_last = 0;

	havebeep = true;
	returntointro = false;
	returntostart = false;

	videocard = (cardtype)0;
	// REFKEEN - No need to free shapes, their fields get refilled,
	// and, if memory was allocated by e.g., BE_Cross_Bfarmalloc,
	// it gets deallocated from BE_Cross_Bexecv automatically.
	//
	// Similarly there's no need to manually free pg_scr.
}
#endif

void slidecat_exe_main(void)
{
	id0_word_t last_key;
	id0_byte_t id0_far *script_file_ptr = NULL, id0_far *script_ptr = NULL;
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
	id0_boolean_t leave_loop = false, page_changed = false, auto_mode = false;
#else
	id0_boolean_t leave_loop = false, page_changed = false;
#endif
	id0_int_t i = 0, j, pages_left;

	if (!BE_Cross_strcasecmp(id0_argv[1], "/?"))
	{
		BE_ST_clrscr();
		BE_ST_textcolor(15);
		BE_ST_textbackground(1);
		BE_ST_cprintf(FRAMETOP_STR);
		BE_ST_cprintf(TITLE_STR);
		BE_ST_cprintf(AUTHOR_STR);
		BE_ST_cprintf(COPYRIGHT_STR);
		BE_ST_cprintf(FRAMEBOT_STR);
		BE_ST_printf("\n");
		BE_ST_printf("/VER  - version number\n");
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
	if (refkeen_current_gamever == BE_GAMEVER_CATABYSS113)
		BE_ST_printf("/AUTO - auto mode\n");
#endif
		BE_ST_printf("/S    - turn sound mode off\n");
		BE_ST_printf("/?    - this help\n");
		BE_ST_HandleExit(0);
	}

	// REFKEEN - PARAM_OFFSET should be a variable (at least in catabyss).
	// Thus, we need to use subtract PARAM_OFFSET from CheckParm's returned value,
	// rather than trying to force usage in the switch statement (this is an error).
#ifdef GAMEVER_CATABYSS
	const int PARAM_OFFSET = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? 1 : 0;
#else
#define PARAM_OFFSET 0
#endif

	for (j=1;j<id0_argc;j++)
		switch (CheckParm(id0_argv[j], ParmStrings)-PARAM_OFFSET)
		//switch (CheckParm(id0_argv[j], ParmStrings))
		{
#if 0
#ifdef GAMEVER_SHAREWARE
#define PARAM_OFFSET 1
#else
#define PARAM_OFFSET 0
#endif
#endif
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
		case -1/*0*/:
			if (PARAM_OFFSET)
				auto_mode = true;
			break;
#endif
		case 0/*PARAM_OFFSET*/:
			BE_ST_clrscr();
			BE_ST_textcolor(15);
			BE_ST_textbackground(1);
			BE_ST_cprintf(FRAMETOP_STR);
			BE_ST_cprintf(TITLE_STR);
			BE_ST_cprintf(AUTHOR_STR);
			BE_ST_cprintf(COPYRIGHT_STR);
			BE_ST_cprintf(FRAMEBOT_STR);
			BE_ST_printf("\n");
			BE_ST_printf("%s %s\n", VERSION_TITLE_STR, VERSION_REV_STR);
			BE_ST_printf("\n");
			BE_ST_printf("This program requires an EGA monitor or better,\n");
			BE_ST_printf("                640K, and MS-DOS 3.0 or better.\n");
			BE_ST_HandleExit(0);
			break;
		case 1/*PARAM_OFFSET+1*/:
			havebeep = false;
			break;
		}

#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
if ((refkeen_current_gamever == BE_GAMEVER_CATABYSS113))
{
	if (!BE_Cross_strcasecmp(id0_argv[1], "#*-NM"))
		returntostart = true;
	else if (!BE_Cross_strcasecmp(id0_argv[1], "^(a@&r`"))
		returntointro = true;
}
#endif
	videocard = VideoID();
	if ((videocard != 3) && (videocard != 5))
		TrashProg("You need an EGA or VGA monitor to run this program.");
	screenmode = 1;
	SetScreenMode(screenmode);
	BE_ST_ToggleTextCursor(false);
	//_setcursortype(_NOCURSOR);

	ShowTextScreen(0);

	for (i=0;i<30;i++)
	{
		pg_scr[i] = (id0_byte_t *)BE_Cross_Bfarmalloc(14);
	}

	if (!ext_BLoad(SCRIPT_FILENAME, (memptr *)&script_file_ptr))
		TrashProg("Can't load Script File - Possibly corrupt file!");

	i = 0;
	// REFKEEN - Somewhat necessary vanilla bug fix
	id0_byte_t id0_far *script_file_start = script_file_ptr;
	while (*script_file_ptr != 0x40)
	{
		for (;*script_file_ptr!=0xD;script_file_ptr++,i++)
		{
			*(pg_scr[pg_last] + i) = *script_file_ptr;
		}
		*(pg_scr[pg_last] + i) = 0;
		i = 0;
		script_file_ptr++;
		script_file_ptr++;
		if (*script_file_ptr != 0x40)
			pg_last++;
	}
	// REFKEEN - As stated above, more-or-less required fix for vanilla bug
	BE_Cross_Bfarfree(script_file_start);
	//BE_Cross_Bfarfree(script_file_ptr);

	// REFKEEN - Alternative controllers support
	extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_slidecat_init;
	extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_slidecat;
	extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback;
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_slidecat_init);

	pages_left = pg_last + 1;
	for (i=0;i<=pg_last;i++)
	{
		BE_ST_ShortSleep(); // REFKEEN - Ensure screen is updated (and add a bit of slowdown)

		if (IsKeyPressed())
			last_key = TryGetScanCode();
		// VERSION RESTORATION - Yeah, there's possibly an access to an uninitialized var here!
		if (last_key == 0x1/*0x11B*/)
		{
			pg_last = pg_last - pages_left;
			TrashProg(NULL);
		}
		script_ptr = pg_scr[i];
		if (*script_ptr++ == 0x5E)
		{
			id0_int_t k;
			id0_char_t numstr[3], filename[13];
			BE_Cross_itoa_dec(pages_left--,numstr);
			//itoa(pages_left--,numstr,10);
			BE_ST_textbackground(4);
			WriteColoredTextAt(14,56,6,"   ");
			WriteColoredTextAt(206,56,6,numstr);
			BE_ST_textbackground(0);
			switch (*script_ptr++)
			{
			case 0x56:
				TrashProg("VGA not available at the moment!\n");
				break;
			case 0x45:
				memset(filename,' ',13);
				for (k=0;k<12;k++)
					filename[k] = *script_ptr++;
				filename[12] = 0;
				if (ext_LoadShape(filename, &page_shapes[i]))
					TrashProg( GAMEVER_SLIDECAT_ERR_STR " : Can't load image.");
				break;
			case 0x54:
				memset(filename,' ',13);
				for (k=0;k<12;k++)
					filename[k] = *script_ptr++;
				filename[12] = 0;
				if (!ext_BLoad(filename, &page_shapes[i].Data))
					TrashProg("Can't load Compressed Text - Possibly corrupt file!");
				break;
			}
		}
		else
			TrashProg( GAMEVER_SLIDECAT_ERR_STR " : Bad script code.\n\n");
	}

	pg_curr = 0;
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
if ((refkeen_current_gamever == BE_GAMEVER_CATABYSS113))
{
	pg_t_str = time(NULL);
	pg_t_end = pg_t_str;
}
#endif
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_slidecat);

	while (!leave_loop)
	{
		script_ptr = pg_scr[pg_curr];
		if (*script_ptr == 0x5E)
		{
			script_ptr++;
			switch (*script_ptr++)
			{
			case 0x56:
				TrashProg("VGA function not available yet!\n");
				break;
			case 0x45:
				if (screenmode != 3)
				{
					screenmode = 3;
					SetScreenMode(screenmode);
					SetScreen(0,0);
				}
				displayofs = 0;
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&page_shapes[pg_curr], 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				Beep();
				break;
			case 0x54:
				if (screenmode != 1)
				{
					screenmode = 1;
					SetScreenMode(screenmode);
					BE_ST_ToggleTextCursor(false);
					//_setcursortype(_NOCURSOR);
				}
				memcpy(BE_ST_GetTextModeMemoryPtr(), (id0_byte_t id0_far *)(page_shapes[pg_curr].Data)+7, 4000);
				BE_ST_MarkGfxForUpdate();
				//_fmemcpy(MK_FP(0xB800,0), (byte far *)(page_shapes[pg_curr].Data)+7, 4000);
				Beep();
				break;
			}
		}
		else
			TrashProg( GAMEVER_SLIDECAT_ERR_STR " : Bad script code\n");

#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
		if ((refkeen_current_gamever == BE_GAMEVER_CATABYSS113) && auto_mode)
		//if (auto_mode)
		{
			id0_byte_t s;
			for (s=0;s<=50;s++)
			{
				BE_ST_Delay(100);
				last_key = TryGetScanCode();
				if (last_key)
				{
					if (last_key == 0x1/*0x11B*/)
						TrashProg(NULL);
					else
					{
						auto_mode = false;
						page_changed = false;
						goto check_keys;
					}
				}
			}
			if (++pg_curr > pg_last)
				pg_curr = 0;
		}
		else
#endif
		{
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
			id0_boolean_t halt_wait_for_key;
#endif
			page_changed = false;
			WaitForKeyRelease();
			while (!page_changed)
			{
				last_key = 0;
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
if (refkeen_current_gamever == BE_GAMEVER_CATABYSS113)
{
				halt_wait_for_key = false;
				while (!halt_wait_for_key)
				{
					last_key = TryGetScanCode();
					if (last_key)
						halt_wait_for_key = true;

					BE_ST_ShortSleep();

					if (difftime(pg_t_end, pg_t_str) > 90.0f)
					{
						last_key = 0;
						halt_wait_for_key = true;
						page_changed = true;
						auto_mode = true;
						if (++pg_curr > pg_last)
							pg_curr = 0;
						break;
					}
					pg_t_end = time(NULL);
				}

				if (auto_mode)
					break;
check_keys:
				pg_t_str = pg_t_end = time(NULL);
}
else
#endif
{
//#else
				while (!last_key)
				{
					last_key = TryGetScanCode();
					BE_ST_ShortSleep();
				}
}
//#endif
				switch (last_key)
				{
				case 0x4D:
				case 0x51:
				case 0x50:
				/*case 0x4D36:
				case 0x4D00:
				case 0x5100:
				case 0x5000:
				case 0x5032:
				case 0x5133:*/
					if (++pg_curr > pg_last)
						pg_curr = 0;
					page_changed = true;
					break;
				case 0x4B:
				case 0x49:
				case 0x48:
				/*case 0x4B00:
				case 0x4900:
				case 0x4800:
				case 0x4838:
				case 0x4939:
				case 0x4B34:*/
					if (!pg_curr)
						pg_curr = pg_last;
					else
						pg_curr--;
					page_changed = true;
					break;
				case 0x47:
				/*case 0x4737:
				case 0x4700:*/
					if (pg_curr)
					{
						pg_curr = 0;
						page_changed = true;
					}
					break;
				case 0x4F:
				/*case 0x4F31:
				case 0x4F00:*/
					if (pg_curr < pg_last)
					{
						pg_curr = pg_last;
						page_changed = true;
					}
					break;
				case 0x1/*0x11B*/:
					TrashProg(NULL);
					break;
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
				case 0x19:
				/*case 0x1970:
				case 0x1950:*/
					if (refkeen_current_gamever == BE_GAMEVER_CATABYSS113)
					{
						PrinterDialog();
						break;
					}
					// Fall-through
#endif
				default:
					// REFKEEN - Alternative controllers support
					BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

					if (screenmode != 1)
					{
						SetScreenMode(1);
						BE_ST_ToggleTextCursor(false);
						//_setcursortype(_NOCURSOR);
					}
					ShowTextScreen(1);
					WaitForKeyRelease();
					while (!(last_key = TryGetScanCode()))
						BE_ST_ShortSleep();
					last_key = 0;
					switch (screenmode)
					{
					case 1:
						memcpy(BE_ST_GetTextModeMemoryPtr(), (id0_byte_t id0_far *)(page_shapes[pg_curr].Data)+7, 4000);
						BE_ST_MarkGfxForUpdate();
						//_fmemcpy(MK_FP(0xB800,0), (byte far *)(page_shapes[pg_curr].Data)+7, 4000);
						break;
					case 3:
						SetScreenMode(screenmode);
						displayofs = 0;
						ext_MoveGfxDst(0, 200);
						UnpackEGAShapeToScreen(&page_shapes[pg_curr], 0, 0);
						ScreenToScreen(8000, 0, 40, 200);
						break;
					}
					// REFKEEN - Alternative controllers support
					BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_slidecat);
					break;
				}
			}
		}
	}
	TrashProg("End of loop reached???????\n");
}

void slidecat_TrashProg (const id0_char_t *OutMsg, ...)
{
	//va_list ap;

#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
	memptr endscreen = NULL;
#endif
	id0_int_t i;

	for (i=0;i<30;i++)
		BE_Cross_Bfarfree(pg_scr[i]);
	for (i=0;i<=pg_last;i++)
		if (page_shapes[i].Data)
			ext_FreeShape(&page_shapes[i]);

	WaitForKeyRelease();
	SetScreenMode(1);

	// REFKEEN - Vanilla bug fix (theoretically preserves the original behaviors)
	if (!OutMsg || !(*OutMsg))
	//if (!(*OutMsg))
	{
#ifdef GAMEVER_CATABYSS
//#ifdef GAMEVER_SHAREWARE
if (refkeen_current_gamever == BE_GAMEVER_CATABYSS113)
{
		if (returntostart)
		{
			// REFKEEN - This is unsupported!
			BE_ST_ExitWithErrorMsg("Can't execute START.EXE - More precisely, unsupported in this port.");
#if 0
			if (execv("START.EXE", id0_argv) == -1)
			{
				printf("ERROR : Can't find executable.\n");
				exit(0);
			}
#endif
		}
		else if (returntointro)
		{
			// REFKEEN difference from vanilla Catacomb Abyss (Shareware release):
			// Role of ^(a@&r` for INTRO.EXE has been flipped. No need to pass it
			// (or use start), but if ^(a@&r` is added then you get this message.
			//
			// However, originally, DEMOCAT.EXE needed it, so it knows to go back
			// to INTRO.EXE. We still do so here, so let's change it back for
			// the intro.
			id0_argv[1] = "z";
			BE_Cross_Bexecv(intro_exe_main, id0_argv, slidecat_exe_main_reverter, false);
#if 0
			if (execv("INTRO.EXE", id0_argv) == -1)
			{
				printf("ERROR : Can't find executable.\n");
				exit(0);
			}
#endif
		}
		else
		{
			if (!ext_BLoad(END_SCREEN, &endscreen))
			{
				// VERSION RESTORATION - The way printf is called here
				// is the reason we use the macro END_SCREEN, rather
				// than a direct mention of the string
				BE_ST_printf("ERROR : Can't load end screen, %s\n", END_SCREEN);
				BE_ST_HandleExit(0);
			}
			memcpy(BE_ST_GetTextModeMemoryPtr(), (id0_byte_t id0_far *)endscreen+7, 4000);
			BE_ST_MarkGfxForUpdate();
			//_fmemcpy(MK_FP(0xB800,0), (byte far *)endscreen+7, 4000);
			BE_Cross_Bfarfree(endscreen);
			BE_ST_MoveTextCursorTo(0, 23); //gotoxy(1,24);
		}
}
else
{
//#elif (defined GAMEVER_CATABYSS)
		BE_ST_HandleExit(0);
}
#endif
	}
	else
		BE_ST_printf(OutMsg);

	WaitForKeyRelease();
	BE_ST_HandleExit(0);
}

// REFKEEN - Re-use functions from intro.c
#if 0
void SetScreenMode (id0_int_t mode)
{
	switch (mode)
	{
	case 1:
		BE_ST_SetScreenMode(3);
		SetLineWidth(80);
		break;
	case 3:
		BE_ST_SetScreenMode(0xd);
		SetLineWidth(40);
		break;
	}
}

//===========================================================================

/*
====================
=
= VW_SetLineWidth
=
= Must be an even number of bytes
=
====================
*/

void SetLineWidth (id0_int_t width)
{
  id0_int_t i,offset;

#if GRMODE == EGAGR
//
// set wide virtual screen
//
	BE_ST_EGASetLineWidth(width); // Ported from ASM
#endif

//
// set up lookup tables
//
  //linewidth = width;

  offset = 0;

  for (i=0;i<VIRTUALHEIGHT;i++)
  {
	ylookup[i]=offset;
	offset += width;
  }
}


//===========================================================================

static id0_boolean_t IsKeyPressed (void)
{
	return BE_ST_KbHit();
#if 0
asm	mov ah, 1
asm	int 0x16
asm	jnz is_pressed

	return false;
	
is_pressed:
	return true;
#endif
}

void WaitForKeyRelease (void)
{
	if (IsKeyPressed())
		while (IsKeyPressed())
		{
			BE_ST_BiosScanCode(0);
			//getch();
		}
}
#endif // REFKEEN - Re-use functions


static void Beep(void)
{
	if (havebeep)
	{
		BE_ST_BSound(100);
		BE_ST_Delay(1);
		BE_ST_BNoSound();
	}
}


///////////////////////////////////////////////////////////////////////////
//
//      US_CheckParm() - checks to see if a string matches one of a set of
//              strings. The check is case insensitive. The routine returns the
//              index of the string that matched, or -1 if no matches were found
//
///////////////////////////////////////////////////////////////////////////
static id0_int_t
CheckParm(const id0_char_t *parm,const id0_char_t **strings)
{
	id0_char_t    cp,cs;
	const id0_char_t *p,*s;
	id0_int_t             i;

	// (REFKEEN) The test has been modified to prevent a buffer overflow.
	// TODO: Any need to emulate behaviors of the original EXEs?
	while ((*parm) && !isalpha(*parm)) // Skip non-alphas
	//while (!isalpha(*parm)) // Skip non-alphas
		parm++;

	for (i = 0;*strings && **strings;i++)
	{
		for (s = *strings++,p = parm,cs = cp = 0;cs == cp;)
		{
			cs = *s++;
			if (!cs)
				return(i);
			cp = *p++;

			if (isupper(cs))
				cs = tolower(cs);
			if (isupper(cp))
				cp = tolower(cp);
		}
	}
	return(-1);
}

// (REFKEEN) Used for patching version-specific stuff

void RefKeen_Patch_slidecat(void)
{
	switch (refkeen_current_gamever)
	{
#ifdef GAMEVER_CATABYSS
	case BE_GAMEVER_CATABYSS113:
		FRAMETOP_STR = "\xD5\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB8\r\n";

		TITLE_STR = "\xB3 GAMER'S EDGE Electronic Catalog      \xB3\r\n";
		AUTHOR_STR = "\xB3 by Nolan Martin                      \xB3\r\n";
		COPYRIGHT_STR = "\xB3 Copyright 1992 - Softdisk Publishing \xB3\r\n";

		FRAMEBOT_STR = "\xD4\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBE\r\n";

		VERSION_TITLE_STR = "ELECTRONIC CATALOG";
		VERSION_REV_STR = " VERSION 1.00   QA[0]";
		break;
	case BE_GAMEVER_CATABYSS124:
		FRAMETOP_STR = "\xD5\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB8\r\n";

		TITLE_STR = "\xB3 CATACOMB ABYSS 3-D HINT BOOK            \xB3\r\n";
		AUTHOR_STR = "\xB3 by Nolan Martin                         \xB3\r\n";
		COPYRIGHT_STR = "\xB3 Copyright 1992-93 - Softdisk Publishing \xB3\r\n";

		FRAMEBOT_STR = "\xD4\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBE\r\n";

		VERSION_TITLE_STR = "CATACOMB ABYSS 3-D HINT BOOK";
		VERSION_REV_STR = " VERSION 1.01 rev 1";
		break;
#endif
#ifdef GAMEVER_CATARM
	case BE_GAMEVER_CATARM102:
		FRAMETOP_STR = "\xD5\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB8\r\n";

		TITLE_STR = "\xB3 CATACOMB ARMAGEDDON 3-D HINT BOOK        \xB3\r\n";
		AUTHOR_STR = "\xB3 by Nolan Martin                          \xB3\r\n";
		COPYRIGHT_STR = "\xB3 Copyright 1992-93 - Softdisk Publishing  \xB3\r\n";

		FRAMEBOT_STR = "\xD4\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBE\r\n";

		VERSION_TITLE_STR = "CATACOMB ARMAGEDDON 3-D HINT BOOK";
		VERSION_REV_STR = " version 1.12  (Rev 1)";
		break;
#endif
#ifdef GAMEVER_CATAPOC
	case BE_GAMEVER_CATAPOC101:
		FRAMETOP_STR = "\xD5\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB8\r\n";

		TITLE_STR = "\xB3 CATACOMB APOCALYPSE 3-D HINT BOOK       \xB3\r\n";
		AUTHOR_STR = "\xB3 by Nolan Martin                         \xB3\r\n";
		COPYRIGHT_STR = "\xB3 Copyright 1992-93 - Softdisk Publishing \xB3\r\n";

		FRAMEBOT_STR = "\xD4\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBE\r\n";

		VERSION_TITLE_STR = "CATACOMB APOCALYPSE 3-D HINT BOOK";
		VERSION_REV_STR = " VERSION 1.13  (Rev 1)";
		break;
#endif
	}

#ifdef GAMEVER_CATABYSS
	SCRIPT_FILENAME = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? "SCRIPT.CAT" : "SCRIPT.HNT";
	ParmStrings = (refkeen_current_gamever == BE_GAMEVER_CATABYSS113) ? ParmStrings_Shar : ParmStrings_Reg;
#else
	SCRIPT_FILENAME = "SCRIPT.HNT";
	ParmStrings = ParmStrings_Reg;
#endif
}

// (REFKEEN) Used for loading data from DOS EXE (instead of hardcoding)
id0_byte_t id0_far *textscn;

void RefKeen_Load_Embedded_Resources_From_slidecat_exe(void)
{
	if (!(textscn = (id0_byte_t *)BE_Cross_BfarmallocFromEmbeddedData("TEXTSCN.SCN", NULL)))
		// Don't use quit, yet
		BE_ST_ExitWithErrorMsg("RefKeen_Load_Embedded_Resources_From_slidecat_exe - Failed to load TEXTSCN.SCN.");
}

REFKEEN_NS_E
