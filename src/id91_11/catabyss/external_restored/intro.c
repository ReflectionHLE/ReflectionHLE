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

/* Reconstructed from the INTRO.EXE file bundled with The Catacomb Abyss
 * Shareware v1.13, using available source files
 */

//#include <conio.h>
#include <stdarg.h>

#include "ext_heads.h"
#include "ext_gelib.h"

static void SetScreenMode (id0_int_t mode);
static void SetLineWidth (id0_int_t width);
static id0_boolean_t IsKeyPressed (void);
static void WaitForKeyRelease (void);
static void FreeAllShapes();

static id0_int_t screenmode;
static cardtype videocard;

static id0_boolean_t havebeep = false;

static struct Shape app_start_shape,
                    page_1_shape, page_2_shape, page_3_shape, page_4_shape, page_5_shape,  
                    before_skill_choice_shape, after_loadgame_choice_shape,
                    novice_choice_shape, warrior_choice_shape, before_gamelaunch_shape;
#if 0
// Looks like there were some unused shapes...
static struct Shape unused_1_shape, unused_2_shape, unused_3_shape, unused_4_shape,
                    unused_5_shape, unused_6_shape, unused_7_shape, unused_8_shape,
                    unused_9_shape, unused_10_shape, unused_11_shape, unused_12_shape,
                    unused_13_shape;
#endif

static void Beep(void)
{
	if (havebeep)
	{
		BE_SDL_BSound(800);
		BE_SDL_Delay(170);
		BE_SDL_BNoSound();
	}
}

void id0_intro_exe_main(void)
{
	id0_boolean_t leave_init_sequence = false, leave_pre_skill_selection = false, leave_skill_selection = false, select_saved_game = false;
	id0_int_t general_loop_var, init_sequence_iters_upperbound;
	id0_int_t current_page = 0, last_key;
	if (!BE_Cross_strcasecmp(id0_argv[1], "/?"))
	{
		BE_SDL_clrscr();
		BE_SDL_textcolor(15);
		BE_SDL_textbackground(1);
		BE_Cross_Simplified_cprintf("\xD5\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB8\r\n");
		BE_Cross_Simplified_cprintf("\xB3 GAMER'S EDGE Catacomb Abyss Introduction Program \xB3\r\n");
		BE_Cross_Simplified_cprintf("\xB3 by Nolan Martin                                  \xB3\r\n");
		BE_Cross_Simplified_cprintf("\xB3 Copyright 1992 - Softdisk Publishing             \xB3\r\n");
		BE_Cross_Simplified_cprintf("\xD4\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBE\r\n");
		BE_Cross_Simplified_printf("\n");
		BE_Cross_Simplified_printf("/VER  - version number\n");
		BE_Cross_Simplified_printf("/?    - this help\n");
		BE_SDL_HandleExit(0);
	}
	if (!BE_Cross_strcasecmp(id0_argv[1], "/VER"))
	{
		BE_SDL_clrscr();
		BE_SDL_textcolor(15);
		BE_SDL_textbackground(1);
		BE_Cross_Simplified_cprintf("\xD5\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xB8\r\n");
		BE_Cross_Simplified_cprintf("\xB3 GAMER'S EDGE Catacomb Abyss Introduction Program \xB3\r\n");
		BE_Cross_Simplified_cprintf("\xB3 by Nolan Martin                                  \xB3\r\n");
		BE_Cross_Simplified_cprintf("\xB3 Copyright 1992 - Softdisk Publishing             \xB3\r\n");
		BE_Cross_Simplified_cprintf("\xD4\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBE\r\n");
		BE_Cross_Simplified_printf("\n");
		// TODO (REFKEEN) No need but...
		BE_Cross_Simplified_printf("THE CATACOMB ABYSS INTRODUCTION PROGRAM  VERSION 1.01   QA[0]\n");
		//BE_Cross_Simplified_printf("%s %s\n", "THE CATACOMB ABYSS INTRODUCTION PROGRAM", " VERSION 1.01   QA[0]");
		BE_Cross_Simplified_printf("\n");
		BE_Cross_Simplified_printf("This program requires an EGA monitor or better,\n");
		BE_Cross_Simplified_printf("                640K, and MS-DOS 3.0 or better.\n");
		BE_SDL_HandleExit(0);
	}
	if (!BE_Cross_strcasecmp(id0_argv[1], "/S") || !BE_Cross_strcasecmp(id0_argv[2], "/S") || !BE_Cross_strcasecmp(id0_argv[3], "/S") || !BE_Cross_strcasecmp(id0_argv[4], "/S"))
		havebeep = false;
	// REFKEEN difference from vanilla Catacomb Abyss (Shareware releases):
	// Role of ^(a@&r1 for INTRO.EXE has been filled. No need to pass it
	// (or use start), but if ^(a@&r1 is added then you get this message.
	if (!BE_Cross_strcasecmp(id0_argv[1], "^(a@&r`"))
	{
		TrashProg("You must type START [ENTER] to run CATACOMB APOCALYPSE!\n");
	}
	videocard = VideoID();
	if ((videocard != 3) && (videocard != 5))
		TrashProg("You need an EGA or VGA monitor to run this program.\n", "Please read the \"INSTRUCTIONS SECTION\" of the START program.\n");
	screenmode = 3;
	SetScreenMode(screenmode);
	SetScreen(0,0);

	// Prepare to enter the CATACOMB...
	if (ext_LoadShape("SHP05.ABS", &app_start_shape))
		TrashProg("ERROR : Can't load image.");
	ext_MoveGfxDst(0, 200);
	UnpackEGAShapeToScreen(&app_start_shape, (320-app_start_shape.bmHdr.w)/2, (200-app_start_shape.bmHdr.h)/2);
	ScreenToScreen(8000, 0, 40, 200);
	ext_FreeShape(&app_start_shape);
	// (REFKEEN) Add an artificial delay so we can actually see loading message...
	BE_SDL_Delay(1000);
	WaitForKeyRelease();

	if (ext_LoadShape("SHP01.ABS", &page_2_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP02.ABS", &page_3_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP03.ABS", &page_4_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP04.ABS", &app_start_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP06.ABS", &before_gamelaunch_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP07.ABS", &before_skill_choice_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP08.ABS", &novice_choice_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP09.ABS", &warrior_choice_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP10.ABS", &after_loadgame_choice_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP11.ABS", &page_5_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();
	if (ext_LoadShape("SHP12.ABS", &page_1_shape))
		TrashProg("ERROR : Can't load image.");
	WaitForKeyRelease();

	current_page = 0;
	if (id0_argc < 3)
	{
		for (general_loop_var = id0_argc; general_loop_var <= 3; ++general_loop_var)
		{
			id0_argv[general_loop_var] = "0";
		}
		id0_argc = 4;
		id0_argv[id0_argc] = NULL;
	}
	do
	{
		for (leave_init_sequence = false; !leave_init_sequence;)
		{
			switch (current_page)
			{
			case 0: // The mad gamers of Gamer's Edge present
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&page_1_shape, 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				++current_page;
				Beep();
				break;
			case 1: // A SoftDisk Publishing production
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&page_2_shape, 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				++current_page;
				Beep();
				break;
			case 2: // The Catacomb Abyss 3D
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&page_3_shape, 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				++current_page;
				Beep();
				break;
			case 3: // Credits
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&page_4_shape, 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				++current_page;
				Beep();
				break;
			case 4: // The Catacomb Adventure Series
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&page_5_shape, 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				current_page = 0;
				Beep();
				break;
			}
			if (current_page == 2)
				init_sequence_iters_upperbound = 150;
			else
				init_sequence_iters_upperbound = 100;
			for (general_loop_var = 0; general_loop_var <= init_sequence_iters_upperbound; ++general_loop_var)
			{
				BE_SDL_Delay(50);
				last_key = TryGetScanCode();
				if (last_key == 0x1/*0x11B*/) // ESC
				{
					FreeAllShapes();
					SetScreenMode(1);
					id0_argc = 2;
					id0_argv[1] = "LAST.SHL";
					id0_argv[2] = "ENDSCN.SCN";
					id0_argv[3] = NULL;

					id0_argv[0] = "LOADSCN.EXE";
					id0_loadscn_exe_main();
#if 0
					if (execv("LOADSCN.EXE", id0_argv) == -1)
					{
						SetScreenMode(1);
						BE_Cross_Simplified_puts("Couldn't find executable LOADSCN.EXE.\n");
						BE_SDL_HandleExit(1);
					}
#endif
				}
				else if (last_key == 0x1C/*0x1C0D*/) // Enter
				{
					leave_init_sequence = true;
					general_loop_var = 600;
				}
				else if (last_key == 0x3E/*0x3E00*/) // F4
				{
					leave_init_sequence = true;
					select_saved_game = true;
					general_loop_var = 600;
				}
				else if (last_key == 0x44/*0x4400*/) // F10 (Demo)
				{
					SetScreenMode(1);
					// REFKEEN: This is currently unsupported
					BE_Cross_Simplified_printf("ERROR : Can't find executable.\nOr rather, the \"Demo\" feature is unsupported in this source port.\n");
					BE_SDL_BiosScanCode(0);
					SetScreenMode(3);
					general_loop_var = 600;
#if 0
					if (execv("DEMOCAT.EXE", id0_argv) == -1)
					{
						BE_Cross_Simplified_printf("ERROR : Can't find executable.\n");
						BE_SDL_HandleExit(0);
					}
#endif
				}
			}
		}
		for (leave_pre_skill_selection = false; !leave_pre_skill_selection; )
		{
			if (select_saved_game) // Launch CATABYSS.EXE and select saved game
			{
				if (!id0_argv[3])
				{
					id0_argv[id0_argc++] = "1";
				}
				else
				{
					id0_argv[id0_argc++] = id0_argv[3];
					id0_argv[3] = "1";
				}
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&before_gamelaunch_shape, 0, 0);
				UnpackEGAShapeToScreen(&after_loadgame_choice_shape, (320-after_loadgame_choice_shape.bmHdr.w)/2, 192);
				ScreenToScreen(8000, 0, 40, 200);
				Beep();
				TrashProg(NULL);
			}
			else
			{
				if (!id0_argv[3])
				{
					id0_argv[id0_argc++] = "0";
				}
				else
				{
					id0_argv[id0_argc++] = id0_argv[3];
					id0_argv[3] = "0";
				}
				ext_MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&before_skill_choice_shape, 0, 0);
				ScreenToScreen(8000, 0, 40, 200);
				Beep();
				for (leave_skill_selection = false; !leave_skill_selection; )
				{
					last_key = TryGetScanCode();
					if ((last_key == 0x11/*0x1157*/)/* || (last_key == 0x1177)*/) // W or w (Warrior)
					{
						if (!id0_argv[2])
						{
							id0_argv[id0_argc++] = "0";
						}
						else
						{
							id0_argv[id0_argc++] = id0_argv[2];
							id0_argv[2] = "0";
						}
						ext_MoveGfxDst(0, 200);
						UnpackEGAShapeToScreen(&before_gamelaunch_shape, 0, 0);
						UnpackEGAShapeToScreen(&warrior_choice_shape, (320-warrior_choice_shape.bmHdr.w)/2, 192);
						ScreenToScreen(8000, 0, 40, 200);
						Beep();
						WaitForKeyRelease();
						while (!(last_key = TryGetScanCode()))
							BE_SDL_ShortSleep();
						if (last_key == 0x1/*0x11B*/) // ESC
							leave_skill_selection = true;
						else
						{
							ext_MoveGfxDst(0, 200);
							UnpackEGAShapeToScreen(&app_start_shape, 0, 0);
							ScreenToScreen(8000, 0, 40, 200);
							Beep();
							BE_SDL_BiosScanCode(0);
							TrashProg(NULL);
						}
					}
					else if ((last_key == 0x31/*0x314E*/)/* || (last_key == 0x316E)*/) // N or n (Novice)
					{
						if (!id0_argv[2])
						{
							id0_argv[id0_argc++] = "1";
						}
						else
						{
							id0_argv[id0_argc++] = id0_argv[2];
							id0_argv[2] = "1";
						}
						ext_MoveGfxDst(0, 200);
						UnpackEGAShapeToScreen(&before_gamelaunch_shape, 0, 0);
						UnpackEGAShapeToScreen(&novice_choice_shape, (320-novice_choice_shape.bmHdr.w)/2, 192);
						ScreenToScreen(8000, 0, 40, 200);
						Beep();
						WaitForKeyRelease();
						while (!(last_key = TryGetScanCode()))
							BE_SDL_ShortSleep();
						if (last_key == 0x1/*0x11B*/)
							leave_skill_selection = true;
						else
						{
							ext_MoveGfxDst(0, 200);
							UnpackEGAShapeToScreen(&app_start_shape, 0, 0);
							ScreenToScreen(8000, 0, 40, 200);
							Beep();
							BE_SDL_BiosScanCode(0);
							TrashProg(NULL);
						}
					}
					else if (last_key == 0x1/*0x11B*/) // ESC
					{
						leave_pre_skill_selection = true;
						leave_skill_selection = true;
						current_page = 1;
					}
					else
					{
						BE_SDL_ShortSleep();
					}
				}
			}
		}
	} while (1);
}

void intro_TrashProg (id0_char_t *OutMsg, ...)
{
	va_list ap;

	FreeAllShapes();
	BE_SDL_ToggleTextCursor(true);
	//_setcursortype(_NORMALCURSOR);

	if (!OutMsg)
	{
		if (!id0_argv[1])
		{
			id0_argv[id0_argc++] = "^(a@&r`";
		}
		else
		{
			id0_argv[id0_argc++] = id0_argv[1];
			id0_argv[1] = "^(a@&r`";
		}
		id0_argv[id0_argc++] = "/NODR";
		id0_argv[id0_argc] = NULL;

		id0_main();
#if 0
		if (execv("CATABYSS.EXE", id0_argv) == -1)
		{
			SetScreenMode(1);
			printf("ERROR : Can't find executable.\n");
			BE_SDL_HandleExit(0);
		}
#endif
	}
	else
	{
		if (screenmode != 1)
			SetScreenMode(1);

		va_start(ap, OutMsg);

		if (OutMsg && *OutMsg)
		// TODO (REFKEEN) PROPERLY IMPLEMENT!
			BE_Cross_Simplified_printf(OutMsg);
			//vprintf(OutMsg,ap);

		va_end(ap);
	}
	BE_SDL_HandleExit(0);
}

static void SetScreenMode (id0_int_t mode)
{
	switch (mode)
	{
	case 1:
		BE_SDL_SetScreenMode(3);
		SetLineWidth(80);
		break;
	case 3:
		BE_SDL_SetScreenMode(0xd);
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

static void SetLineWidth (id0_int_t width)
{
  id0_int_t i,offset;

#if GRMODE == EGAGR
//
// set wide virtual screen
//
	BE_SDL_EGASetLineWidth(width); // Ported from ASM
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
	return BE_SDL_KbHit();
#if 0
asm	mov ah, 1
asm	int 0x16
asm	jnz is_pressed

	return false;
	
is_pressed:
	return true;
#endif
}

static void WaitForKeyRelease (void)
{
	if (IsKeyPressed())
		while (IsKeyPressed())
		{
			BE_SDL_BiosScanCode(0);
			//getch();
		}
}

static void FreeAllShapes()
{
	ext_FreeShape(&page_3_shape);
	ext_FreeShape(&page_2_shape);
	ext_FreeShape(&page_4_shape);
	ext_FreeShape(&app_start_shape);
	ext_FreeShape(&before_skill_choice_shape);
	ext_FreeShape(&before_gamelaunch_shape);
	ext_FreeShape(&novice_choice_shape);
	ext_FreeShape(&warrior_choice_shape);
	ext_FreeShape(&after_loadgame_choice_shape);
}
