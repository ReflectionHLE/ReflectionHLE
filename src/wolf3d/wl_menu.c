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

////////////////////////////////////////////////////////////////////
//
// WL_MENU.C
// by John Romero (C) 1992 Id Software, Inc.
//
////////////////////////////////////////////////////////////////////
#include "wl_def.h"
//#pragma hdrstop

#define REFKEEN_ENABLE_FILE_SEARCH 0 // TODO (REFKEEN): Implement

//
// PRIVATE PROTOTYPES
//
CP_routineret CP_ReadThis(CP_routineargs);
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
CP_routineret CP_BackToDemo(CP_routineargs);
#endif

#ifdef SPEAR
#define STARTITEM	newgame

#else
// *** ALPHA RESTORATION ***
#if (defined GOODTIMES) || (GAMEVER_WOLFREV <= GV_WR_WL920312)
//#ifdef GOODTIMES
#define STARTITEM	newgame

#else
#define STARTITEM	readthis
#endif
#endif

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_char_t GAMEVER_COND_FARPTR endStrings[9][80]=
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	{"Are you sure you're\nready to quit?.\nPress Y to Quit."},
	{"What? Quitting already?\nPress Y to Quit."},
	{"Are a few hungry animals\ntoo tough for you?\nPress Y to Quit."},
	{"Giving up so soon?\nPress Y to Quit."},
	{"Be sure to take your\numbrella, it's wet out there.\nPress Y to Quit."},
	{"You're going to let\nthose animals run free?\nPress Y to Quit."},
	{"Did the animals wear\nyou out?\nPress Y to Quit."},
	{"Are you monkeying around or\ndo you really want to quit?\nPress Y to Quit."},
	{"The animals might get\nhungry if you're gone\ntoo long!\nPress Y to Quit."}
#elif (!defined SPEAR)
//#ifndef SPEAR
	{"Dost thou wish to\nleave with such hasty\nabandon?"},
	{"Chickening out...\nalready?"},
	{"Press N for more carnage.\nPress Y to be a weenie."},
	{"So, you think you can\nquit this easily, huh?"},
	{"Press N to save the world.\nPress Y to abandon it in\nits hour of need."},
	{"Press N if you are brave.\nPress Y to cower in shame."},
	{"Heroes, press N.\nWimps, press Y."},
	{"You are at an intersection.\nA sign says, 'Press Y to quit.'\n>"},
	{"For guns and glory, press N.\nFor work and worry, press Y."}
#else
	ENDSTR1,
	ENDSTR2,
	ENDSTR3,
	ENDSTR4,
	ENDSTR5,
	ENDSTR6,
	ENDSTR7,
	ENDSTR8,
	ENDSTR9
#endif
};
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

// *** ALPHA RESTORATION ***
// Looks like these were originally defined here, with a few changes.
// The yet-unused pickquick variable is defined for the purpose
// of recreating the original EXE, albeit in a different location.
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
id0_int_t pickquick;
id0_int_t SaveGamesAvail[10],StartGame,SoundStatus=1;
id0_char_t SaveGameNames[10][32],SaveName[13]="SAVEGAM?.WL1";
#endif

CP_iteminfo
	MainItems={MENU_X,MENU_Y,10,STARTITEM,24},
	SndItems={SM_X,SM_Y1,12,0,52},
	LSItems={LSM_X,LSM_Y,10,0,24},
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	CtlItems={CTL_X,CTL_Y,5,-1,56},
	CusItems={8,CST_Y+13*2,9,-1,0},
	NewItems={NM_X,NM_Y,3,1,24};
#else
	CtlItems={CTL_X,CTL_Y,6,-1,56},
	CusItems={8,CST_Y+13*2,9,-1,0},
	NewEitems={NE_X,NE_Y,11,0,88},
	NewItems={NM_X,NM_Y,4,2,24};
#endif

// *** S3DNA RESTORATION ***
// A few convenience macros
#ifdef GAMEVER_NOAH3D
#define GAMEVER_N3D_SS_OFFSET 1
#define GAMEVER_MOUSELBACKY 192
#else
#define GAMEVER_N3D_SS_OFFSET 0
#define GAMEVER_MOUSELBACKY 184
#endif

//#pragma warn -sus
CP_itemtype GAMEVER_COND_FARPTR
MainMenu[]=
{
#ifdef JAPAN
	{1,"",CP_NewGame},
	{1,"",CP_Sound},
	{1,"",CP_Control},
	{1,"",CP_LoadGame},
	{0,"",CP_SaveGame},
	{1,"",CP_ChangeView},
	{2,"",CP_ReadThis},
	{1,"",CP_ViewScores},
	{1,"",0},
	{1,"",0}
#else

	{1,STR_NG,CP_NewGame},
	{1,STR_SD,CP_Sound},
	{1,STR_CL,CP_Control},
	{1,STR_LG,CP_LoadGame},
	{0,STR_SG,CP_SaveGame},
	{1,STR_CV,CP_ChangeView},

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	{2,"Read This!",HelpScreens},
	{1,"Ordering Info",OrderingInfo},
#else

#ifndef GOODTIMES
#ifndef SPEAR

	#ifdef SPANISH
	{2,"Ve esto!",CP_ReadThis},
	#else
	{2,"Read This!",CP_ReadThis},
	#endif

#endif
#endif

	{1,STR_VS,CP_ViewScores},
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	{1,STR_BD,CP_BackToDemo},
#else
	{1,STR_BD,0},
#endif
	{1,STR_QT,0}
#endif
},

GAMEVER_COND_FARPTR SndMenu[]=
{
#ifdef JAPAN
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{1,"",0},
#else
	{1,STR_NONE,0},
	{1,STR_PC,0},
	{1,STR_ALSB,0},
	{0,"",0},
	{0,"",0},
	{1,STR_NONE,0},
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	{1,STR_DISNEY,0},
#endif
	{1,STR_SB,0},
	{0,"",0},
	{0,"",0},
	{1,STR_NONE,0},
	{1,STR_ALSB,0}
#endif
},

GAMEVER_COND_FARPTR CtlMenu[]=
{
#ifdef JAPAN
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",MouseSensitivity},
	{1,"",CustomControls}
#else
	{0,STR_MOUSEEN,0},
	{0,STR_JOYEN,0},
	{0,STR_PORT2,0},
	{0,STR_GAMEPAD,0},
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	{0,STR_SENS,MouseSensitivity},
#endif
	{1,STR_CUSTOM,CustomControls}
#endif
},

//#pragma warn +sus

// *** ALPHA RESTORATION ***/
#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
GAMEVER_COND_FARPTR NewEmenu[]=
{
#ifdef JAPAN
#ifdef JAPDEMO
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
#else
	{1,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0}
#endif
#else
	// *** S3DNA RESTORATION ***
	#ifdef GAMEVER_NOAH3D
	{1,"Carl the Camel\n"
	   "Start at level 1-1",0},
	{0,"",0},
	{3,"Ginny the Giraffe\n"
	   "Start at level 2-1",0},
	{0,"",0},
	{3,"Melvin the Monkey\n"
	   "Start at level 3-1",0},
	{0,"",0},
	{3,"Kerry the Kangaroo\n"
	   "Start at level 4-1",0},
	{0,"",0},
	{3,"Ernie the Elephant\n"
	   "Start at level 5-1",0},
	{0,"",0},
	{3,"Burt the Bear\n"
	   "Start at level 6-1",0},
	#elif (defined SPANISH)
	//#ifdef SPANISH
	{1,"Episodio 1\n"
	   "Fuga desde Wolfenstein",0},
	{0,"",0},
	{3,"Episodio 2\n"
		   "Operacion Eisenfaust",0},
	{0,"",0},
	{3,"Episodio 3\n"
		   "Muere, Fuhrer, Muere!",0},
	{0,"",0},
	{3,"Episodio 4\n"
		  "Un Negro Secreto",0},
	{0,"",0},
	{3,"Episodio 5\n"
		  "Huellas del Loco",0},
	{0,"",0},
	{3,"Episodio 6\n"
		  "Confrontacion",0}
	#else
	{1,"Episode 1\n"
	   "Escape from Wolfenstein",0},
	{0,"",0},
	{3,"Episode 2\n"
		   "Operation: Eisenfaust",0},
	{0,"",0},
	{3,"Episode 3\n"
		   "Die, Fuhrer, Die!",0},
	{0,"",0},
	{3,"Episode 4\n"
		  "A Dark Secret",0},
	{0,"",0},
	{3,"Episode 5\n"
		  "Trail of the Madman",0},
	{0,"",0},
	{3,"Episode 6\n"
		  "Confrontation",0}
	#endif
#endif
},
#endif


GAMEVER_COND_FARPTR NewMenu[]=
{
#ifdef JAPAN
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0}
#else
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	{1,STR_DADDY,0},
#endif
	{1,STR_HURTME,0},
	{1,STR_BRINGEM,0},
	{1,STR_DEATH,0}
#endif
},

GAMEVER_COND_FARPTR LSMenu[]=
{
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0}
},

GAMEVER_COND_FARPTR CusMenu[]=
{
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{1,"",0}
}
;


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_int_t color_hlite[]={
   DEACTIVE,
   HIGHLIGHT,
   READHCOLOR,
   0x67
   },

   color_norml[]={
   DEACTIVE,
   TEXTCOLOR,
   READCOLOR,
   0x6b
   };

id0_int_t EpisodeSelect[6]={1};
#endif


// *** ALPHA RESTORATION ***
// Looks like these were originally defined above, with a few changes
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_int_t SaveGamesAvail[10],StartGame,SoundStatus=1,pickquick;
id0_char_t SaveGameNames[10][32],SaveName[13]="SAVEGAM?.";
#endif


////////////////////////////////////////////////////////////////////
//
// INPUT MANAGER SCANCODE TABLES
//
////////////////////////////////////////////////////////////////////

// REFKEEN - Originally the type was simply "byte" (unsigned 8-bit int), but at
// least for strings, if we want this to build as C++ (and reduce C warnings)
// then we should use "const char". Conversions to e.g., ints are done later.
static const char
					*ScanNames[] =		// Scan code names with single chars
					{
	"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
	"Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
	"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
	"B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
	"\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
					},	// DEBUG - consolidate these
					id0_far ExtScanCodes[] =	// Scan codes with >1 char names
					{
	1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,
	0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
	0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,
	0x50,0x4b,0x4d,0x00
					},
					*ExtScanNames[] =	// Names corresponding to ExtScanCodes
					{
	"Esc","BkSp","Tab","Ctrl","LShft","Space","CapsLk","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","F11","F12","ScrlLk","Enter","RShft",
	"PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up",
	"Down","Left","Right",""
					};


////////////////////////////////////////////////////////////////////
//
// Wolfenstein Control Panel!  Ta Da!
//
////////////////////////////////////////////////////////////////////
void US_ControlPanel(id0_byte_t scancode)
{
	id0_int_t which,i,start;


	// *** ALPHA RESTORATION ***
	// Looks like an almost-perfect replica of the code handling
	// sc_F10 in CP_CheckQuick, as well as CP_Quit.
	// Makes use of the QUITSUR string, *not* used in v1.0 or later.
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (scancode == sc_F9)
	{
		CA_CacheGrChunk(STARTFONT+1);
		WindowH=160;
		if (Confirm(QUITSUR))
		{
			VW_UpdateScreen();
			SD_MusicOff();
			SD_StopSound();
			MenuFadeOut();

			//
			// SHUT-UP THE ADLIB
			//
			for (i=1;i<=0xf5;i++)
				alOut(i,0);
			Quit(NULL);
		}

		WindowH=200;
		UNCACHEGRCHUNK(STARTFONT+1);
		fontnumber=0;
		return;
	}
#else // GAMEVER_WOLFREV > GV_WR_WL920312
	if (ingame)
		if (CP_CheckQuick(scancode))
			return;
#endif

	StartCPMusic(MENUSONG);
	SetupControlPanel();

	//
	// F-KEYS FROM WITHIN GAME
	//
	switch(scancode)
	{
		case sc_F1:
			#ifdef SPEAR
			BossKey();
			#else
			#ifdef GOODTIMES
			BossKey();
			#else
			HelpScreens();
			#endif
			#endif
			goto finishup;

		case sc_F2:
			// ALPHA RESTORATION
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			CP_SaveGame();
#else
			CP_SaveGame(0);
#endif
			goto finishup;

		case sc_F3:
			// ALPHA RESTORATION
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			CP_LoadGame();
#else
			CP_LoadGame(0);
#endif
			goto finishup;

		case sc_F4:
			CP_Sound(CP_itemvalue);
			goto finishup;

		case sc_F5:
			CP_ChangeView(CP_itemvalue);
			goto finishup;

		case sc_F6:
			CP_Control(CP_itemvalue);
			goto finishup;

		finishup:
			CleanupControlPanel();
			#ifdef SPEAR
			UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
			#endif
			return;
	}

#ifdef SPEAR
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif

	DrawMainMenu();
	MenuFadeIn();
	StartGame=0;

	//
	// MAIN MENU LOOP
	//
	do
	{
		which=HandleMenu(&MainItems,&MainMenu[0],NULL);

		#ifdef SPEAR
		#ifndef SPEARDEMO
		//
		// EASTER EGG FOR SPEAR OF DESTINY!
		//
		if (Keyboard[sc_I] && Keyboard[sc_D])
		{
			VW_FadeOut();
			StartCPMusic (XJAZNAZI_MUS);
			UnCacheLump(OPTIONS_LUMP_START,OPTIONS_LUMP_END);
			UnCacheLump(BACKDROP_LUMP_START,BACKDROP_LUMP_END);
			MM_SortMem ();
			ClearMemory ();


			CA_CacheGrChunk (IDGUYS1PIC);
			VWB_DrawPic(0,0,IDGUYS1PIC);
			UNCACHEGRCHUNK(IDGUYS1PIC);

			CA_CacheGrChunk (IDGUYS2PIC);
			VWB_DrawPic(0,80,IDGUYS2PIC);
			UNCACHEGRCHUNK(IDGUYS2PIC);

			VW_UpdateScreen();

			CA_CacheGrChunk (IDGUYSPALETTE);
			VL_FadeIn(0,255,(id0_byte_t *)grsegs[IDGUYSPALETTE],30);
			UNCACHEGRCHUNK(IDGUYSPALETTE);

			while (Keyboard[sc_I] || Keyboard[sc_D])
				BE_ST_ShortSleep();
			IN_ClearKeysDown();
			IN_Ack();

			VW_FadeOut();

			CacheLump(BACKDROP_LUMP_START,BACKDROP_LUMP_END);
			CacheLump(OPTIONS_LUMP_START,OPTIONS_LUMP_END);
			DrawMainMenu();
			StartCPMusic (MENUSONG);
			MenuFadeIn();
		}
		#endif
		#endif

		switch(which)
		{
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			case viewscores:
				if (MainMenu[viewscores].routine == NULL)
					if (CP_EndGame())
						StartGame=1;

				DrawMainMenu();
				MenuFadeIn();
				break;
#endif

			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			case backtodemo:
				#ifdef SPEAR
				if (!ingame)
				{
					//
					// DEALLOCATE ALL SOUNDS!
					//
					switch (SoundMode)
					{
						case sdm_PC:
							start = STARTPCSOUNDS;
							break;
						case sdm_AdLib:
							start = STARTADLIBSOUNDS;
							break;
					}

					if (SoundMode != sdm_Off)
						for (i=0;i<NUMSOUNDS;i++,start++)
							if (audiosegs[start])
								MM_SetPurge ((memptr *)&audiosegs[start],3);		// make purgable
				}
				#endif

				MM_SortMem();
				StartGame=1;
				if (!ingame)
					StartCPMusic(INTROSONG);
				VL_FadeOut(0,255,0,0,0,10);
				break;
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11

			case -1:
			case quit:
				CP_Quit();
				break;

			default:
				if (!StartGame)
				{
					DrawMainMenu();
					MenuFadeIn();
				}
		}

	//
	// "EXIT OPTIONS" OR "NEW GAME" EXITS
	//
	} while(!StartGame);

	//
	// DEALLOCATE EVERYTHING
	//
	CleanupControlPanel();

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	//
	// CHANGE MAINMENU ITEM
	//

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	if (startgame)
#else
	if (startgame || loadedgame)
#endif
	{
		#pragma warn -sus
		MainMenu[viewscores].routine = NULL;
		#ifndef JAPAN
		GAMEVER_COND_FSTRCPY(MainMenu[viewscores].string,STR_EG);
		#endif
		#pragma warn +sus
	}

	// RETURN/START GAME EXECUTION

#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	MM_SortMem ();
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
}


////////////////////////
//
// DRAW MAIN MENU SCREEN
//
void DrawMainMenu(void)
{
#ifdef JAPAN
	CA_CacheScreen(S_OPTIONSPIC);
#else
	ClearMScreen();

	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);
	// *** ALPHA RESTORATION ***
	// Using color from ID_VH.H here
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VWB_Bar(0,10,320,22,BLACK);
	VWB_Hlin(0,319,33,0);
#else
	DrawStripes(10);
#endif
	VWB_DrawPic(84,0,C_OPTIONSPIC);

	#ifdef SPANISH
	DrawWindow(MENU_X-8,MENU_Y-3,MENU_W+8,MENU_H,BKGDCOLOR);
	#else
	DrawWindow(MENU_X-8,MENU_Y-3,MENU_W,MENU_H,BKGDCOLOR);
	#endif
#endif

	//
	// CHANGE "GAME" AND "DEMO"
	//
	if (ingame)
	{
		#ifndef JAPAN

		#ifdef SPANISH
		GAMEVER_COND_FSTRCPY(&MainMenu[backtodemo].string,STR_GAME);
		#else
		GAMEVER_COND_FSTRCPY(&MainMenu[backtodemo].string[8],STR_GAME);
		#endif

		#else
		CA_CacheGrChunk(C_MRETGAMEPIC);
		VWB_DrawPic(12*8,20*8,C_MRETGAMEPIC);
		UNCACHEGRCHUNK(C_MRETGAMEPIC);
		CA_CacheGrChunk(C_MENDGAMEPIC);
		VWB_DrawPic(12*8,18*8,C_MENDGAMEPIC);
		UNCACHEGRCHUNK(C_MENDGAMEPIC);
		#endif
		MainMenu[backtodemo].active=2;
	}
	else
	{
		#ifndef JAPAN
		#ifdef SPANISH
		GAMEVER_COND_FSTRCPY(&MainMenu[backtodemo].string,STR_BD);
		#else
		GAMEVER_COND_FSTRCPY(&MainMenu[backtodemo].string[8],STR_DEMO);
		#endif
		#else
		CA_CacheGrChunk(C_MRETDEMOPIC);
		VWB_DrawPic(12*8,20*8,C_MRETDEMOPIC);
		UNCACHEGRCHUNK(C_MRETDEMOPIC);
		CA_CacheGrChunk(C_MSCORESPIC);
		VWB_DrawPic(12*8,18*8,C_MSCORESPIC);
		UNCACHEGRCHUNK(C_MSCORESPIC);
		#endif
		MainMenu[backtodemo].active=1;
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (LastScan == sc_F9)
		return;
#endif
	DrawMenu(&MainItems,&MainMenu[0]);
	VW_UpdateScreen();
}

// *** ALPHA RESTORATION ***
// Various functions were added after the alpha

#if (GAMEVER_WOLFREV > GV_WR_WL920312)

#ifndef GOODTIMES
#ifndef SPEAR
////////////////////////////////////////////////////////////////////
//
// READ THIS!
//
////////////////////////////////////////////////////////////////////
CP_routineret CP_ReadThis(CP_routineargs)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	StartCPMusic(NOAH01_MUS);
	HelpScreens();
	StartCPMusic(MENUSONG);
#else
	StartCPMusic(CORNER_MUS);
	HelpScreens();
	StartCPMusic(MENUSONG);
#endif
	return CP_itemvalue;
}
#endif
#endif

/*** SOD VERSIONS RESTORATION - Should be able to call BossKey... ***/
#if (defined GOODTIMES) || (defined SPEAR)
//#ifndef SPEAR
//#ifndef GOODTIMES
//#else
////////////////////////////////////////////////////////////////////
//
// BOSS KEY
//
////////////////////////////////////////////////////////////////////
void BossKey(void)
{
	SD_MusicOff();
	BE_ST_SetScreenMode(3);
	BE_ST_printf("C>");
	while (!Keyboard[sc_Escape])
	{
		IN_ClearKeysDown();
		BE_ST_ShortSleep();
	}

	SD_MusicOn();
	VL_SetVGAPlaneMode ();
	VL_TestPaletteSet ();
	VL_SetPalette (gamepal);
	LoadLatchMem();
}
//#endif
#endif

////////////////////////////////////////////////////////////////////
//
// CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
//
////////////////////////////////////////////////////////////////////
id0_int_t CP_CheckQuick(id0_unsigned_t scancode)
{
	switch(scancode)
	{
		//
		// END GAME
		//
		case sc_F7:
			CA_CacheGrChunk(STARTFONT+1);

			WindowH=160;
			#ifdef JAPAN
			if (GetYorN(7,8,C_JAPQUITPIC))
			#else
			if (Confirm(ENDGAMESTR))
			#endif
			{
				playstate = ex_died;
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				gamestate.lives = 0;
#else
				pickquick = gamestate.lives = 0;
#endif
			}

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			DrawAllPlayBorder();
#endif
			WindowH=200;
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			UNCACHEGRCHUNK(STARTFONT+1);
#endif
			fontnumber=0;
			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			MainMenu[savegame].active = 0;
#endif
			return 1;

		//
		// QUICKSAVE
		//
		case sc_F8:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				CA_CacheGrChunk(STARTFONT+1);
				// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
				fontnumber = 1;
#endif
				Message(STR_SAVING"...");
				CP_SaveGame(1);
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				UNCACHEGRCHUNK(STARTFONT+1);
#endif
				fontnumber=0;
			}
			else
			{
				#ifndef SPEAR
				CA_CacheGrChunk(STARTFONT+1);
				CA_CacheGrChunk(C_CURSOR1PIC);
				CA_CacheGrChunk(C_CURSOR2PIC);
				CA_CacheGrChunk(C_DISKLOADING1PIC);
				CA_CacheGrChunk(C_DISKLOADING2PIC);
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				CA_CacheGrChunk(C_DISKLOADING3PIC);
				CA_CacheGrChunk(C_DISKLOADING4PIC);
#endif
				CA_CacheGrChunk(C_SAVEGAMEPIC);
				CA_CacheGrChunk(C_MOUSELBACKPIC);
				#else
				CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
				CA_CacheGrChunk(C_CURSOR1PIC);
				#endif

				VW_FadeOut ();
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				ResetSplitScreen ();
#endif

				StartCPMusic(MENUSONG);
				pickquick=CP_SaveGame(0);

				SETFONTCOLOR(0,15);
				IN_ClearKeysDown();
				DrawPlayScreen ();

				if (!startgame && !loadedgame)
				{
					VW_FadeIn ();
					StartMusic ();
				}

				if (loadedgame)
					playstate = ex_abort;
				lasttimecount = SD_GetTimeCount();

				if (MousePresent)
					BE_ST_GetEmuAccuMouseMotion(NULL, NULL); // Clear accumulated mouse movement
//					Mouse(MDelta);	// Clear accumulated mouse movement

				PM_CheckMainMem ();

				#ifndef SPEAR
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				UNCACHEGRCHUNK(STARTFONT+1);
#endif
				UNCACHEGRCHUNK(C_CURSOR1PIC);
				UNCACHEGRCHUNK(C_CURSOR2PIC);
				UNCACHEGRCHUNK(C_DISKLOADING1PIC);
				UNCACHEGRCHUNK(C_DISKLOADING2PIC);
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				UNCACHEGRCHUNK(C_DISKLOADING3PIC);
				UNCACHEGRCHUNK(C_DISKLOADING4PIC);
#endif
				UNCACHEGRCHUNK(C_SAVEGAMEPIC);
				UNCACHEGRCHUNK(C_MOUSELBACKPIC);
				#else
				UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
				#endif
			}
			return 1;

		//
		// QUICKLOAD
		//
		case sc_F9:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				id0_char_t string[100]=STR_LGC;


				CA_CacheGrChunk(STARTFONT+1);
				// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
				fontnumber = 1;
#endif

				strcat(string,SaveGameNames[LSItems.curpos]);
				strcat(string,"\"?");

				if (Confirm(string))
					CP_LoadGame(1);

				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
				DrawAllPlayBorder();
#endif
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				UNCACHEGRCHUNK(STARTFONT+1);
#endif
				fontnumber=0;
			}
			else
			{
				#ifndef SPEAR
				CA_CacheGrChunk(STARTFONT+1);
				CA_CacheGrChunk(C_CURSOR1PIC);
				CA_CacheGrChunk(C_CURSOR2PIC);
				CA_CacheGrChunk(C_DISKLOADING1PIC);
				CA_CacheGrChunk(C_DISKLOADING2PIC);
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				CA_CacheGrChunk(C_DISKLOADING3PIC);
				CA_CacheGrChunk(C_DISKLOADING4PIC);
#endif
				CA_CacheGrChunk(C_LOADGAMEPIC);
				CA_CacheGrChunk(C_MOUSELBACKPIC);
				#else
				CA_CacheGrChunk(C_CURSOR1PIC);
				CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
				#endif

				VW_FadeOut ();
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				ResetSplitScreen ();
#endif

				StartCPMusic(MENUSONG);
				pickquick=CP_LoadGame(0);

				SETFONTCOLOR(0,15);
				IN_ClearKeysDown();
				DrawPlayScreen ();

				if (!startgame && !loadedgame)
				{
					VW_FadeIn ();
					StartMusic ();
				}

				if (loadedgame)
					playstate = ex_abort;

				lasttimecount = SD_GetTimeCount();

				if (MousePresent)
					BE_ST_GetEmuAccuMouseMotion(NULL, NULL); // Clear accumulated mouse movement
//					Mouse(MDelta);	// Clear accumulated mouse movement
				PM_CheckMainMem ();

				#ifndef SPEAR
				// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
				UNCACHEGRCHUNK(STARTFONT+1);
#endif
				UNCACHEGRCHUNK(C_CURSOR1PIC);
				UNCACHEGRCHUNK(C_CURSOR2PIC);
				UNCACHEGRCHUNK(C_DISKLOADING1PIC);
				UNCACHEGRCHUNK(C_DISKLOADING2PIC);
				// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
				UNCACHEGRCHUNK(C_DISKLOADING3PIC);
				UNCACHEGRCHUNK(C_DISKLOADING4PIC);
#endif
				UNCACHEGRCHUNK(C_LOADGAMEPIC);
				UNCACHEGRCHUNK(C_MOUSELBACKPIC);
				#else
				UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
				#endif
			}
			return 1;

		//
		// QUIT
		//
		case sc_F10:
			CA_CacheGrChunk(STARTFONT+1);

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			WindowX=WindowY=0;
			WindowW=320;
#endif
			WindowH=160;
			#ifdef JAPAN
			if (GetYorN(7,8,C_QUITMSGPIC))
			#else
				#ifdef SPANISH
			if (Confirm(ENDGAMESTR))
				#else
			if (Confirm(endStrings[US_RndT()&0x7+(US_RndT()&1)]))
				#endif
			#endif
			{
				id0_int_t i;


				VW_UpdateScreen();
				SD_MusicOff();
				SD_StopSound();
				MenuFadeOut();

				//
				// SHUT-UP THE ADLIB
				//
				for (i=1;i<=0xf5;i++)
					alOut(i,0);
				Quit(NULL);
			}

			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
			DrawAllPlayBorder();
#endif
			WindowH=200;
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			UNCACHEGRCHUNK(STARTFONT+1);
#endif
			fontnumber=0;
			return 1;
		}

	return 0;
}


////////////////////////////////////////////////////////////////////
//
// END THE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
id0_int_t CP_EndGame(void)
{
#ifdef JAPAN
	if (!GetYorN(7,8,C_JAPQUITPIC))
#else
	if (!Confirm(ENDGAMESTR))
#endif
		return 0;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	gamestate.lives = 0;
#else
	pickquick = gamestate.lives = 0;
#endif
	playstate = ex_died;

	#pragma warn -sus
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	MainMenu[savegame].active = 0;
#endif
	MainMenu[viewscores].routine=CP_ViewScores;
	#ifndef JAPAN
	GAMEVER_COND_FSTRCPY(MainMenu[viewscores].string,STR_VS);
	#endif
	#pragma warn +sus

	return 1;
}


////////////////////////////////////////////////////////////////////
//
// VIEW THE HIGH SCORES
//
////////////////////////////////////////////////////////////////////
CP_routineret CP_ViewScores(CP_routineargs)
{
	fontnumber=0;

#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	StartCPMusic (XAWARD_MUS);
	// *** S3DNA RESTORATION ***
#elif defined GAMEVER_NOAH3D
	StartCPMusic (NOAH11_MUS);
#else
	StartCPMusic (ROSTER_MUS);
#endif

	DrawHighScores ();
	VW_UpdateScreen ();
	MenuFadeIn();
	fontnumber=1;

	IN_Ack();

	StartCPMusic(MENUSONG);
	MenuFadeOut();

#ifdef SPEAR
	CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif
	return CP_itemvalue;
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


////////////////////////////////////////////////////////////////////
//
// START A NEW GAME
//
////////////////////////////////////////////////////////////////////
CP_routineret CP_NewGame(CP_routineargs)
{
	id0_int_t which,episode;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)

#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif


#ifndef SPEAR
firstpart:

	DrawNewEpisode();
	do
	{
		which=HandleMenu(&NewEitems,&NewEmenu[0],NULL);
		switch(which)
		{
			case -1:
				MenuFadeOut();
				return CP_itemvalue;

			default:
				if (!EpisodeSelect[which/2])
				{
					SD_PlaySound (NOWAYSND);
					Message("Please select \"Read This!\"\n"
							"from the Options menu to\n"
							"find out how to order this\n"
							"episode from Apogee.");
					IN_ClearKeysDown();
					IN_Ack();
					DrawNewEpisode();
					which = 0;
				}
				else
				{
					// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
					switch (which/2)
					{
						case 1: episode = 3;
							break;
						case 2: episode = 7;
							break;
						case 3: episode = 12;
							break;
						case 4: episode = 17;
							break;
						case 5: episode = 23;
							break;
						default: episode = 0;
					}
#else
					episode = which/2;
#endif
					which = 1;
				}
				break;
		}

	} while (!which);

	ShootSnd();

	//
	// ALREADY IN A GAME?
	//
	if (ingame)
		#ifdef JAPAN
		if (!GetYorN(7,8,C_JAPNEWGAMEPIC))
		#else
		if (!Confirm(CURGAME))
		#endif
		{
			MenuFadeOut();
			return CP_itemvalue;
		}

	MenuFadeOut();

#else
	episode = 0;

	//
	// ALREADY IN A GAME?
	//
	CacheLump (NEWGAME_LUMP_START,NEWGAME_LUMP_END);
	DrawNewGame();
	if (ingame)
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			UnCacheLump (NEWGAME_LUMP_START,NEWGAME_LUMP_END);
			CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
			return CP_itemvalue;
		}

#endif

#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	DrawNewGame();
	which=HandleMenu(&NewItems,&NewMenu[0],DrawNewGameDiff);
	if (which<0)
	{
		MenuFadeOut();
		// *** ALPHA RESTORATION ***
		#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		return CP_itemvalue;
		#elif (!defined SPEAR)
		//#ifndef SPEAR
		goto firstpart;
		#else
		UnCacheLump (NEWGAME_LUMP_START,NEWGAME_LUMP_END);
		CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
		return CP_itemvalue;
		#endif
	}

	// *** ALPHA RESTORATION ***
	// Some code is similar to what's found above (e.g., for SPEAR)
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	//
	// ALREADY IN A GAME?
	//
	if (ingame)
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			return;
		}
#endif
	ShootSnd();

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	NewGame(which);
#else
	NewGame(which,episode);
#endif
	StartGame=1;
	MenuFadeOut();

	//
	// CHANGE "READ THIS!" TO NORMAL COLOR
	//
	#ifndef SPEAR
	#ifndef GOODTIMES
	MainMenu[readthis].active=1;
	#endif
	#endif

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	pickquick = 0;
#endif

#ifdef SPEAR
	UnCacheLump (NEWGAME_LUMP_START,NEWGAME_LUMP_END);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif
}


// *** ALPHA RESTORATION ***
#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef SPEAR
/////////////////////
//
// DRAW NEW EPISODE MENU
//
void DrawNewEpisode(void)
{
	id0_int_t i;

#ifdef JAPAN
	CA_CacheScreen(S_EPISODEPIC);
#else
	ClearMScreen();
	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);

	DrawWindow(NE_X-4,NE_Y-4,NE_W+8,NE_H+8,BKGDCOLOR);
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintY=2;
	WindowX=0;
	#ifdef SPANISH
	US_CPrint("Cual episodio jugar?");
	#else
	US_CPrint("Which episode to play?");
	#endif
#endif

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	DrawMenu(&NewEitems,&NewEmenu[0]);

	for (i=0;i<6;i++)
		VWB_DrawPic(NE_X+32,NE_Y+i*26,C_EPISODE1PIC+i);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}
#endif

/////////////////////
//
// DRAW NEW GAME MENU
//
void DrawNewGame(void)
{
#ifdef JAPAN
	CA_CacheScreen(S_SKILLPIC);
#else
	ClearMScreen();
	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	SETFONTCOLOR(HIGHLIGHT,BKGDCOLOR);
	DrawWindow(NM_X+15,NM_Y-50,NM_W-40,NM_H-31,BKGDCOLOR);
	PrintX=NM_X+20;
	PrintY=NM_Y-45;
#else
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintX=NM_X+20;
	PrintY=NM_Y-32;
#endif

#ifndef SPEAR
	#ifdef SPANISH
	US_Print("Eres macho?");
	#else
	US_Print("How tough are you?");
	#endif
#else
	VWB_DrawPic (PrintX,PrintY,C_HOWTOUGHPIC);
#endif

	DrawWindow(NM_X-5,NM_Y-10,NM_W,NM_H,BKGDCOLOR);
#endif

	DrawMenu(&NewItems,&NewMenu[0]);
	DrawNewGameDiff(NewItems.curpos);
	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}


////////////////////////
//
// DRAW NEW GAME GRAPHIC
//
void DrawNewGameDiff(id0_int_t w)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VWB_DrawPic(NM_X+165,NM_Y-7,w+C_EASYPIC);
#else
	VWB_DrawPic(NM_X+185,NM_Y+7,w+C_BABYMODEPIC);
#endif
}


////////////////////////////////////////////////////////////////////
//
// HANDLE SOUND MENU
//
////////////////////////////////////////////////////////////////////
CP_routineret CP_Sound(CP_routineargs)
{
	id0_int_t which,i;


#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	CacheLump (SOUND_LUMP_START,SOUND_LUMP_END);
#endif

	DrawSoundMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&SndItems,&SndMenu[0],NULL);
		//
		// HANDLE MENU CHOICES
		//
		switch(which)
		{
			//
			// SOUND EFFECTS
			//
			case 0:
				if (SoundMode!=sdm_Off)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_Off);
					DrawSoundMenu();
				}
				break;
			case 1:
				if (SoundMode!=sdm_PC)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_PC);
					CA_LoadAllSounds();
					DrawSoundMenu();
					ShootSnd();
				}
				break;
			case 2:
				if (SoundMode!=sdm_AdLib)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_AdLib);
					CA_LoadAllSounds();
					DrawSoundMenu();
					ShootSnd();
				}
				break;

			//
			// DIGITIZED SOUND
			//
			case 5:
				if (DigiMode!=sds_Off)
				{
					SD_SetDigiDevice(sds_Off);
					DrawSoundMenu();
				}
				break;
			// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
			case 6:
				if (DigiMode!=sds_SoundSource)
				{
					SD_SetDigiDevice(sds_SoundSource);
					DrawSoundMenu();
					ShootSnd();
				}
				break;
#endif
			case 7-GAMEVER_N3D_SS_OFFSET:
				if (DigiMode!=sds_SoundBlaster)
				{
					SD_SetDigiDevice(sds_SoundBlaster);
					DrawSoundMenu();
					ShootSnd();
				}
				break;

			//
			// MUSIC
			//

			case 10-GAMEVER_N3D_SS_OFFSET:
				if (MusicMode!=smm_Off)
				{
					SD_SetMusicMode(smm_Off);
					DrawSoundMenu();
					ShootSnd();
				}
				break;
			case 11-GAMEVER_N3D_SS_OFFSET:
				if (MusicMode!=smm_AdLib)
				{
					SD_SetMusicMode(smm_AdLib);
					DrawSoundMenu();
					ShootSnd();
					StartCPMusic(MENUSONG);
				}
				break;
		}
		BE_ST_ShortSleep();
	} while(which>=0);

	MenuFadeOut();

#ifdef SPEAR
	UnCacheLump (SOUND_LUMP_START,SOUND_LUMP_END);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif
	return CP_itemvalue;
}


//////////////////////
//
// DRAW THE SOUND MENU
//
void DrawSoundMenu(void)
{
	id0_int_t i,on;


#ifdef JAPAN
	CA_CacheScreen(S_SOUNDPIC);
#else
	//
	// DRAW SOUND MENU
	//
	ClearMScreen();
	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);

	DrawWindow(SM_X-8,SM_Y1-3,SM_W,SM_H1,BKGDCOLOR);
	DrawWindow(SM_X-8,SM_Y2-3,SM_W,SM_H2,BKGDCOLOR);
	DrawWindow(SM_X-8,SM_Y3-3,SM_W,SM_H3,BKGDCOLOR);
#endif

	//
	// IF NO ADLIB, NON-CHOOSENESS!
	//
	if (!AdLibPresent && !SoundBlasterPresent)
	{
		SndMenu[2].active=SndMenu[10-GAMEVER_N3D_SS_OFFSET].active=SndMenu[11-GAMEVER_N3D_SS_OFFSET].active=0;
	}

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	if (!SoundSourcePresent)
		SndMenu[6].active=0;
#endif

	if (!SoundBlasterPresent)
		SndMenu[7-GAMEVER_N3D_SS_OFFSET].active=0;

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!SoundBlasterPresent)
#else
	if (!SoundSourcePresent && !SoundBlasterPresent)
#endif
		SndMenu[5].active=0;

	DrawMenu(&SndItems,&SndMenu[0]);
#ifndef JAPAN
	VWB_DrawPic(100,SM_Y1-20,C_FXTITLEPIC);
	VWB_DrawPic(100,SM_Y2-20,C_DIGITITLEPIC);
	VWB_DrawPic(100,SM_Y3-20,C_MUSICTITLEPIC);
#endif

	for (i=0;i<SndItems.amount;i++)
#ifdef JAPAN
		if (i!=3 && i!=4 && i!=8 && i!=9)
#else
		if (SndMenu[i].string[0])
#endif
		{
			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//
			on=0;
			switch(i)
			{
				//
				// SOUND EFFECTS
				//
				case 0: if (SoundMode==sdm_Off) on=1; break;
				case 1: if (SoundMode==sdm_PC) on=1; break;
				case 2: if (SoundMode==sdm_AdLib) on=1; break;

				//
				// DIGITIZED SOUND
				//
				case 5: if (DigiMode==sds_Off) on=1; break;
				// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
				case 6: if (DigiMode==sds_SoundSource) on=1; break;
#endif
				case 7-GAMEVER_N3D_SS_OFFSET: if (DigiMode==sds_SoundBlaster) on=1; break;

				//
				// MUSIC
				//
				case 10-GAMEVER_N3D_SS_OFFSET: if (MusicMode==smm_Off) on=1; break;
				case 11-GAMEVER_N3D_SS_OFFSET: if (MusicMode==smm_AdLib) on=1; break;
			}

			if (on)
				VWB_DrawPic(SM_X+24,SM_Y1+i*13+2,C_SELECTEDPIC);
			else
				VWB_DrawPic(SM_X+24,SM_Y1+i*13+2,C_NOTSELECTEDPIC);
		}

	DrawMenuGun(&SndItems);
	VW_UpdateScreen();
}


//
// DRAW LOAD/SAVE IN PROGRESS
//
void DrawLSAction(id0_int_t which)
{
	#define LSA_X	96
	#define LSA_Y	80
	#define LSA_W	130
	#define LSA_H	42

	DrawWindow(LSA_X,LSA_Y,LSA_W,LSA_H,TEXTCOLOR);
	DrawOutline(LSA_X,LSA_Y,LSA_W,LSA_H,0,HIGHLIGHT);
	VWB_DrawPic(LSA_X+8,LSA_Y+5,C_DISKLOADING1PIC);

	fontnumber=1;
	SETFONTCOLOR(0,TEXTCOLOR);
	PrintX=LSA_X+46;
	PrintY=LSA_Y+13;

	if (!which)
		US_Print(STR_LOADING"...");
	else
		US_Print(STR_SAVING"...");

	VW_UpdateScreen();
}


////////////////////////////////////////////////////////////////////
//
// LOAD SAVED GAMES
//
////////////////////////////////////////////////////////////////////
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void CP_LoadGame(void)
#else
id0_int_t CP_LoadGame(id0_int_t quick)
#endif
{
	BE_FILE_T handle;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_int_t which;
#else
	id0_int_t which,exit=0;
#endif
	id0_char_t name[13];


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	strcpy(name,SaveName);

	//
	// QUICKLOAD?
	//
	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			name[7]=which+'0';
			handle=BE_Cross_open_rewritable_for_reading(name);
//			handle=open(name,O_BINARY);
			BE_Cross_seek(handle,32,SEEK_SET);
			loadedgame=true;
			LoadTheGame(handle,0,0);
			loadedgame=false;
			BE_Cross_close(handle);

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
			return 1;
		}
	}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	CacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
#endif

	DrawLoadSaveScreen(0);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0 && SaveGamesAvail[which])
		{
			ShootSnd();
			// *** ALPHA RESTORATION -- ***
			// Looks like this got relocated later
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			strcpy(name,SaveName);
#endif
			name[7]=which+'0';

			handle=BE_Cross_open_rewritable_for_reading(name);
//			handle=open(name,O_BINARY);
			BE_Cross_seek(handle,32,SEEK_SET);

			DrawLSAction(0);
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			loadedgame=true;
#endif

			LoadTheGame(handle,LSA_X+8,LSA_Y+5);
			BE_Cross_close(handle);

			StartGame=1;
			// *** ALPHA RESTORATION ***
			// Relocated as well
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			loadedgame=true;
#endif
			ShootSnd();
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			//
			// CHANGE "READ THIS!" TO NORMAL COLOR
			//

			#ifndef SPEAR
			#ifndef GOODTIMES
			MainMenu[readthis].active=1;
			#endif
			#endif

			exit=1;
#endif
			break;
		}
		BE_ST_ShortSleep();

	} while(which>=0);

	MenuFadeOut();

#ifdef SPEAR
	UnCacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	return exit;
#endif
}


///////////////////////////////////
//
// HIGHLIGHT CURRENT SELECTED ENTRY
//
void TrackWhichGame(id0_int_t w)
{
	static id0_int_t lastgameon=0;

	PrintLSEntry(lastgameon,TEXTCOLOR);
	PrintLSEntry(w,HIGHLIGHT);

	lastgameon=w;
}


////////////////////////////
//
// DRAW THE LOAD/SAVE SCREEN
//
void DrawLoadSaveScreen(id0_int_t loadsave)
{
	#define DISKX	100
	#define DISKY	0

	id0_int_t i;


	ClearMScreen();
	fontnumber=1;
	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);
	DrawWindow(LSM_X-10,LSM_Y-5,LSM_W,LSM_H,BKGDCOLOR);
	// *** ALPHA RESTORATION ***
	// Now we have some usage for DISKX and DISKY
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VWB_DrawPic(DISKX,DISKY,C_LOADSAVEDISKPIC);
	PrintX=LSM_X+50;
	PrintY=LSM_Y-30;
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);

	if (!loadsave)
		US_Print(STR_LG);
	else
		US_Print(STR_SG);
#else
	DrawStripes(10);

	if (!loadsave)
		VWB_DrawPic(60,0,C_LOADGAMEPIC);
	else
		VWB_DrawPic(60,0,C_SAVEGAMEPIC);
#endif

	for (i=0;i<10;i++)
		PrintLSEntry(i,TEXTCOLOR);

	DrawMenu(&LSItems,&LSMenu[0]);
	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}


///////////////////////////////////////////
//
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
//
void PrintLSEntry(id0_int_t w,id0_int_t color)
{
	SETFONTCOLOR(color,BKGDCOLOR);
	DrawOutline(LSM_X+LSItems.indent,LSM_Y+w*13,LSM_W-LSItems.indent-15,11,color,color);
	PrintX=LSM_X+LSItems.indent+2;
	PrintY=LSM_Y+w*13+1;
	fontnumber=0;

	if (SaveGamesAvail[w])
		US_Print(SaveGameNames[w]);
	else
		US_Print("      - "STR_EMPTY" -");

	fontnumber=1;
}


////////////////////////////////////////////////////////////////////
//
// SAVE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void CP_SaveGame(void)
#else
id0_int_t CP_SaveGame(id0_int_t quick)
#endif
{
	BE_FILE_T handle;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_int_t which;
#else
	id0_int_t which,exit=0;
	id0_unsigned_t nwritten;
#endif
	id0_char_t name[13],input[32];


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	strcpy(name,SaveName);

	//
	// QUICKSAVE?
	//
	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			name[7]=which+'0';
			BE_Cross_unlink_rewritable(name);
			handle=BE_Cross_open_rewritable_for_overwriting(name);
//			handle=creat(name,S_IREAD|S_IWRITE);

			strcpy(input,&SaveGameNames[which][0]);

			BE_Cross_writeInt8LEBuffer(handle,(void id0_far *)input,32);
			//_dos_write(handle,(void id0_far *)input,32,&nwritten);
			BE_Cross_seek(handle,32,SEEK_SET);
			SaveTheGame(handle,0,0);
			BE_Cross_close(handle);

			return 1;
		}
	}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	CacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
#endif

	DrawLoadSaveScreen(1);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0)
		{
			//
			// OVERWRITE EXISTING SAVEGAME?
			//
			if (SaveGamesAvail[which])
				#ifdef JAPAN
				if (!GetYorN(7,8,C_JAPSAVEOVERPIC))
				#else
				if (!Confirm(GAMESVD))
				#endif
				{
					DrawLoadSaveScreen(1);
					continue;
				}
				else
				{
					DrawLoadSaveScreen(1);
					PrintLSEntry(which,HIGHLIGHT);
					VW_UpdateScreen();
				}

			ShootSnd();
			// *** ALPHA RESTORATION ***
			// Looks like this got relocated later
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			strcpy(name,SaveName);
#endif

			strcpy(input,&SaveGameNames[which][0]);
			name[7]=which+'0';

			fontnumber=0;
			if (!SaveGamesAvail[which])
				VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*13+1,LSM_W-LSItems.indent-16,10,BKGDCOLOR);
			VW_UpdateScreen();

			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			if (US_LineInput(LSM_X+LSItems.indent+2,LSM_Y+which*13+1,input,input,true,31,LSM_W-LSItems.indent-20))
#else
			if (US_LineInput(LSM_X+LSItems.indent+2,LSM_Y+which*13+1,input,input,true,31,LSM_W-LSItems.indent-30))
#endif
			{
				SaveGamesAvail[which]=1;
				strcpy(&SaveGameNames[which][0],input);

				BE_Cross_unlink_rewritable(name);
				handle=BE_Cross_open_rewritable_for_overwriting(name);
//				handle=creat(name,S_IREAD|S_IWRITE);
				// *** ALPHA RESTORATION ***
				BE_Cross_writeInt8LEBuffer(handle,input,32);
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
				//write(handle,input,32);
#else
				//_dos_write(handle,(void id0_far *)input,32,&nwritten);
				BE_Cross_seek(handle,32,SEEK_SET);
#endif

				DrawLSAction(1);
				SaveTheGame(handle,LSA_X+8,LSA_Y+5);

				BE_Cross_close(handle);

				ShootSnd();
				// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
				exit=1;
#endif
			}
			else
			{
				VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*13+1,LSM_W-LSItems.indent-16,10,BKGDCOLOR);
				PrintLSEntry(which,HIGHLIGHT);
				VW_UpdateScreen();
				SD_PlaySound(ESCPRESSEDSND);
				continue;
			}

			fontnumber=1;
			break;
		}
		BE_ST_ShortSleep();

	} while(which>=0);

	MenuFadeOut();

#ifdef SPEAR
	UnCacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	StartGame = 1;	
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	return exit;
#endif
}

// *** PRE-V1.4 APOGEE RESTORATION *** - CalibrateJoystick was added to v1.4,
// while v1.2 seems to have an unused handler assigned to "Back to demo"
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
CP_routineret CP_BackToDemo(CP_routineargs)
{
	StartGame = 1;
	return CP_itemvalue;
}
#else
////////////////////////////////////////////////////////////////////
//
// CALIBRATE JOYSTICK
//
////////////////////////////////////////////////////////////////////
id0_int_t CalibrateJoystick(void)
{
	#define CALX	85
	#define CALY	40
	#define CALW	158
	#define CALH	140

	id0_unsigned_t xmin,ymin,xmax,ymax,jb;



	#ifdef JAPAN
	VWB_DrawPic(CALX,CALY,C_JOY0PIC);
	#else
	DrawWindow(CALX-5,CALY-5,CALW,CALH,TEXTCOLOR);
	DrawOutline(CALX-5,CALY-5,CALW,CALH,0,HIGHLIGHT);
	SETFONTCOLOR(0,TEXTCOLOR);

	WindowX = PrintX = CALX;
	WindowW = CALW;
	WindowH = CALH;
	WindowY = PrintY = CALY;
	US_Print("    "STR_CALIB"\n    "STR_JOYST"\n");
	VWB_DrawPic(CALX+40,CALY+30,C_JOY1PIC);
	PrintY = CALY+80;
	US_Print(STR_MOVEJOY);
	SETFONTCOLOR(BKGDCOLOR,TEXTCOLOR);
	US_Print("   "STR_ESCEXIT);
	#endif
	VW_UpdateScreen();

	do
	{
		jb=IN_JoyButtons();
		if (Keyboard[sc_Escape])
			return 0;
		// *** SHAREWARE/REGISTERED APOGEE + S3DNA RESTORATION ***
		// This is also skipped in the Apogee and S3DNA EXEs
		#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A) && (!defined GAMEVER_NOAH3D)
		//#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			PicturePause();
		#endif
		BE_ST_ShortSleep();

	} while(!(jb&1));

	SD_PlaySound(SHOOTSND);
	IN_GetJoyAbs(joystickport,&xmin,&ymin);


	#ifdef JAPAN
	VWB_DrawPic(CALX,CALY,C_JOY1PIC);
	#else
	DrawWindow(CALX-5,CALY-5,CALW,CALH,TEXTCOLOR);
	DrawOutline(CALX-5,CALY-5,CALW,CALH,0,HIGHLIGHT);
	SETFONTCOLOR(0,TEXTCOLOR);

	PrintX = CALX;
	PrintY = CALY;
	US_Print("    "STR_CALIB"\n    "STR_JOYST"\n");
	VWB_DrawPic(CALX+40,CALY+30,C_JOY2PIC);
	PrintY = CALY+80;
	US_Print(STR_MOVEJOY2);
	SETFONTCOLOR(BKGDCOLOR,TEXTCOLOR);
	US_Print("   "STR_ESCEXIT);
	#endif
	VW_UpdateScreen();

	do
	{
		jb=IN_JoyButtons();
		if (Keyboard[sc_Escape])
			return 0;
		// *** SHAREWARE/REGISTERED APOGEE + S3DNA RESTORATION ***
		// And again
		#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A) && (!defined GAMEVER_NOAH3D)
		//#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			PicturePause();
		#endif
		BE_ST_ShortSleep();
	} while(!(jb&2));

	IN_GetJoyAbs(joystickport,&xmax,&ymax);
	SD_PlaySound(SHOOTSND);

	while (IN_JoyButtons())
		BE_ST_ShortSleep();

	//
	// ASSIGN ACTUAL VALUES HERE
	//
	if ((xmin != xmax) && (ymin != ymax))
		IN_SetupJoy(joystickport,xmin,xmax,ymin,ymax);
	else
		return 0;

	return 1;
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL6AP11


////////////////////////////////////////////////////////////////////
//
// DEFINE CONTROLS
//
////////////////////////////////////////////////////////////////////
CP_routineret CP_Control(CP_routineargs)
{
	#define CTL_SPC	70
	enum {MOUSEENABLE,JOYENABLE,USEPORT2,PADENABLE,MOUSESENS,CUSTOMIZE};
	id0_int_t i,which;


#ifdef SPEAR
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	CacheLump (CONTROL_LUMP_START,CONTROL_LUMP_END);
#endif

	DrawCtlScreen();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&CtlItems,&CtlMenu[0],NULL);
		switch(which)
		{
			case MOUSEENABLE:
				mouseenabled^=1;
				BE_ST_SetEmuMousePos(CENTER,CENTER);
//				_CX=_DX=CENTER;
//				Mouse(4);
				DrawCtlScreen();
				CusItems.curpos=-1;
				ShootSnd();
				break;

			case JOYENABLE:
				joystickenabled^=1;
				// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
				if (joystickenabled)
					if (!CalibrateJoystick())
						joystickenabled = 0;
#endif
				DrawCtlScreen();
				CusItems.curpos=-1;
				ShootSnd();
				break;

			case USEPORT2:
				joystickport^=1;
				DrawCtlScreen();
				ShootSnd();
				break;

			case PADENABLE:
				joypadenabled^=1;
				DrawCtlScreen();
				ShootSnd();
				break;

			case MOUSESENS:
			case CUSTOMIZE:
				DrawCtlScreen();
				MenuFadeIn();
				WaitKeyUp();
				break;
		}
		BE_ST_ShortSleep();
	} while(which>=0);

	MenuFadeOut();

#ifdef SPEAR
	UnCacheLump (CONTROL_LUMP_START,CONTROL_LUMP_END);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
#endif
	return CP_itemvalue;
}


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
////////////////////////////////
//
// DRAW MOUSE SENSITIVITY SCREEN
//
void DrawMouseSens(void)
{
#ifdef JAPAN
	CA_CacheScreen(S_MOUSESENSPIC);
#else
	ClearMScreen();
	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);
	#ifdef SPANISH
	DrawWindow(10,80,300,43,BKGDCOLOR);
	#else
	DrawWindow(10,80,300,30,BKGDCOLOR);
	#endif

	WindowX=0;
	WindowW=320;
	PrintY=82;
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	US_CPrint(STR_MOUSEADJ);

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	#ifdef SPANISH
	PrintX=14;
	PrintY=95+13;
	US_Print(STR_SLOW);
	PrintX=252;
	US_Print(STR_FAST);
	#else
	PrintX=14;
	PrintY=95;
	US_Print(STR_SLOW);
	PrintX=269;
	US_Print(STR_FAST);
	#endif
#endif

	VWB_Bar(60,97,200,10,TEXTCOLOR);
	DrawOutline(60,97,200,10,0,HIGHLIGHT);
	DrawOutline(60+20*mouseadjustment,97,20,10,0,READCOLOR);
	VWB_Bar(61+20*mouseadjustment,98,19,9,READHCOLOR);

	VW_UpdateScreen();
	MenuFadeIn();
}


///////////////////////////
//
// ADJUST MOUSE SENSITIVITY
//
CP_routineret MouseSensitivity(CP_routineargs)
{
	ControlInfo ci;
	id0_int_t exit=0,oldMA;


	oldMA=mouseadjustment;
	DrawMouseSens();
	do
	{
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			case dir_North:
			case dir_West:
				if (mouseadjustment)
				{
					mouseadjustment--;
					VWB_Bar(60,97,200,10,TEXTCOLOR);
					DrawOutline(60,97,200,10,0,HIGHLIGHT);
					DrawOutline(60+20*mouseadjustment,97,20,10,0,READCOLOR);
					VWB_Bar(61+20*mouseadjustment,98,19,9,READHCOLOR);
					VW_UpdateScreen();
					SD_PlaySound(MOVEGUN1SND);
					while(Keyboard[sc_LeftArrow])
						BE_ST_ShortSleep();
					WaitKeyUp();
				}
				break;

			case dir_South:
			case dir_East:
				if (mouseadjustment<9)
				{
					mouseadjustment++;
					VWB_Bar(60,97,200,10,TEXTCOLOR);
					DrawOutline(60,97,200,10,0,HIGHLIGHT);
					DrawOutline(60+20*mouseadjustment,97,20,10,0,READCOLOR);
					VWB_Bar(61+20*mouseadjustment,98,19,9,READHCOLOR);
					VW_UpdateScreen();
					SD_PlaySound(MOVEGUN1SND);
					while(Keyboard[sc_RightArrow])
						BE_ST_ShortSleep();
					WaitKeyUp();
				}
				break;
		}

		// *** SHAREWARE/REGISTERED V1.4 APOGEE+SOD (DEMO) V1.0 RESTORATION ***
		// And again - kind of
		#if (GAMEVER_WOLFREV > GV_WR_SODFG14A)
		#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		#else
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm("debugmode"))
		#endif
			PicturePause();
		#endif

		if (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
			exit=2;

		BE_ST_ShortSleep();
	} while(!exit);

	if (exit==2)
	{
		mouseadjustment=oldMA;
		SD_PlaySound(ESCPRESSEDSND);
	}
	else
		SD_PlaySound(SHOOTSND);

	WaitKeyUp();
	MenuFadeOut();
	return CP_itemvalue;
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


///////////////////////////
//
// DRAW CONTROL MENU SCREEN
//
void DrawCtlScreen(void)
{
 id0_int_t i,x,y;


#ifdef JAPAN
	CA_CacheScreen(S_CONTROLPIC);
#else
 ClearMScreen();
 // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
 DrawStripes(10);
 VWB_DrawPic(80,0,C_CONTROLPIC);
#endif
 VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);
 DrawWindow(CTL_X-8,CTL_Y-5,CTL_W,CTL_H,BKGDCOLOR);
#endif
 WindowX=0;
 WindowW=320;
 // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
 SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
 PrintY=5;
 US_CPrint("Control Menu");
#endif
 SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);

 if (JoysPresent[0])
   // *** ALPHA RESTORATION ***
   CtlMenu[1].active=
   CtlMenu[2].active=
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
   CtlMenu[3].active=
   NewMenu[0].active=1;
#else
   CtlMenu[3].active=1;
#endif

 // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
 CtlMenu[2].active=CtlMenu[3].active=joystickenabled;
#endif

 if (MousePresent)
 {
  // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
  CtlMenu[4].active=
#endif
  CtlMenu[0].active=1;
 }

 // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
 CtlMenu[4].active=mouseenabled;
#endif


 DrawMenu(&CtlItems,&CtlMenu[0]);


 x=CTL_X+CtlItems.indent-24;
 y=CTL_Y+3;
 if (mouseenabled)
   VWB_DrawPic(x,y,C_SELECTEDPIC);
 else
   VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

 y=CTL_Y+16;
 if (joystickenabled)
   VWB_DrawPic(x,y,C_SELECTEDPIC);
 else
   VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

 y=CTL_Y+29;
 if (joystickport)
   VWB_DrawPic(x,y,C_SELECTEDPIC);
 else
   VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

 y=CTL_Y+42;
 if (joypadenabled)
   VWB_DrawPic(x,y,C_SELECTEDPIC);
 else
   VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

 //
 // PICK FIRST AVAILABLE SPOT
 //

 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
 if (CtlItems.curpos==-1)
   for (i=0;i<5;i++)
#else
 if (CtlItems.curpos<0 || !CtlMenu[CtlItems.curpos].active)
   for (i=0;i<6;i++)
#endif
	 if (CtlMenu[i].active)
	 {
	  CtlItems.curpos=i;
	  break;
	 }

 DrawMenuGun(&CtlItems);
 VW_UpdateScreen();
}


////////////////////////////////////////////////////////////////////
//
// CUSTOMIZE CONTROLS
//
////////////////////////////////////////////////////////////////////
enum {FIRE,STRAFE,RUN,OPEN};
id0_char_t mbarray[4][3]={"b0","b1","b2","b3"},
	   order[4]={RUN,OPEN,FIRE,STRAFE};


CP_routineret CustomControls(CP_routineargs)
{
 id0_int_t which;


 DrawCustomScreen();
 do
 {
  which=HandleMenu(&CusItems,&CusMenu[0],FixupCustom);
  switch(which)
  {
   case 0:
	 DefineMouseBtns();
	 DrawCustMouse(1);
	 break;
   case 3:
	 DefineJoyBtns();
	 DrawCustJoy(0);
	 break;
   case 6:
	 DefineKeyBtns();
	 DrawCustKeybd(0);
	 break;
   case 8:
	 DefineKeyMove();
	 DrawCustKeys(0);
  }
  BE_ST_ShortSleep();
 } while(which>=0);



 MenuFadeOut();
 return CP_itemvalue;
}


////////////////////////
//
// DEFINE THE MOUSE BUTTONS
//
void DefineMouseBtns(void)
{
 CustomCtrls mouseallowed={0,1,1,1};
 EnterCtrlData(2,&mouseallowed,DrawCustMouse,PrintCustMouse,MOUSE);
}


////////////////////////
//
// DEFINE THE JOYSTICK BUTTONS
//
void DefineJoyBtns(void)
{
 CustomCtrls joyallowed={1,1,1,1};
 EnterCtrlData(5,&joyallowed,DrawCustJoy,PrintCustJoy,JOYSTICK);
}


////////////////////////
//
// DEFINE THE KEYBOARD BUTTONS
//
void DefineKeyBtns(void)
{
 CustomCtrls keyallowed={1,1,1,1};
 EnterCtrlData(8,&keyallowed,DrawCustKeybd,PrintCustKeybd,KEYBOARDBTNS);
}


////////////////////////
//
// DEFINE THE KEYBOARD BUTTONS
//
void DefineKeyMove(void)
{
	CustomCtrls keyallowed={1,1,1,1};
	EnterCtrlData(10,&keyallowed,DrawCustKeys,PrintCustKeys,KEYBOARDMOVE);
}


////////////////////////
//
// ENTER CONTROL DATA FOR ANY TYPE OF CONTROL
//
enum {FWRD,RIGHT,BKWD,LEFT};
id0_int_t moveorder[4]={LEFT,RIGHT,FWRD,BKWD};

void EnterCtrlData(id0_int_t index,CustomCtrls *cust,void (*DrawRtn)(id0_int_t),void (*PrintRtn)(id0_int_t),id0_int_t type)
{
 id0_int_t j,exit,tick,redraw,which,x,picked;
 ControlInfo ci;


 ShootSnd();
 PrintY=CST_Y+13*index;
 IN_ClearKeysDown();
 exit=0;
 redraw=1;
 //
 // FIND FIRST SPOT IN ALLOWED ARRAY
 //
 for (j=0;j<4;j++)
   if (cust->allowed[j])
   {
	which=j;
	break;
   }

 do
 {
  if (redraw)
  {
   x=CST_START+CST_SPC*which;
   DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);

   DrawRtn(1);
   DrawWindow(x-2,PrintY,CST_SPC,11,TEXTCOLOR);
   DrawOutline(x-2,PrintY,CST_SPC,11,0,HIGHLIGHT);
   SETFONTCOLOR(0,TEXTCOLOR);
   PrintRtn(which);
   PrintX=x;
   SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
   VW_UpdateScreen();
   WaitKeyUp();
   redraw=0;
  }

  ReadAnyControl(&ci);

  if (type==MOUSE || type==JOYSTICK)
	if (IN_KeyDown(sc_Enter)||IN_KeyDown(sc_Control)||IN_KeyDown(sc_Alt))
	{
	 IN_ClearKeysDown();
	 ci.button0=ci.button1=false;
	}

  //
  // CHANGE BUTTON VALUE?
  //
  if ((ci.button0|ci.button1|ci.button2|ci.button3)||
	  ((type==KEYBOARDBTNS||type==KEYBOARDMOVE) && LastScan==sc_Enter))
  {
   tick=picked=0;
   SD_AddToTimeCount(-SD_GetTimeCount());
//   tick=TimeCount=picked=0;
   SETFONTCOLOR(0,TEXTCOLOR);

   do
   {
	id0_int_t button,result=0;


	if (type==KEYBOARDBTNS||type==KEYBOARDMOVE)
	  IN_ClearKeysDown();
	BE_ST_ShortSleep(); // REFKEEN: Need to call this after clearing keys

	//
	// FLASH CURSOR
	//
	if (SD_GetTimeCount()>10)
	{
	 switch(tick)
	 {
	  case 0:
	VWB_Bar(x,PrintY+1,CST_SPC-2,10,TEXTCOLOR);
	break;
	  case 1:
	PrintX=x;
	US_Print("?");
	SD_PlaySound(HITWALLSND);
	 }
	 tick^=1;
	 SD_AddToTimeCount(-SD_GetTimeCount());
	 //TimeCount=0;
	 VW_UpdateScreen();
	}

	//
	// WHICH TYPE OF INPUT DO WE PROCESS?
	//
	switch(type)
	{
	 case MOUSE:
	   button=BE_ST_GetEmuMouseButtons();
//	   Mouse(3);
//	   button=_BX;
	   switch(button)
	   {
	case 1: result=1; break;
	case 2: result=2; break;
	case 4: result=3; break;
	   }

	   if (result)
	   {
	id0_int_t z;


	for (z=0;z<4;z++)
	  if (order[which]==buttonmouse[z])
	  {
	   buttonmouse[z]=bt_nobutton;
	   break;
	  }

	buttonmouse[result-1]=order[which];
	picked=1;
	SD_PlaySound(SHOOTDOORSND);
	   }
	   break;

	 case JOYSTICK:
	   if (ci.button0) result=1;
	   else
	   if (ci.button1) result=2;
	   else
	   if (ci.button2) result=3;
	   else
	   if (ci.button3) result=4;

	   if (result)
	   {
	id0_int_t z;


	for (z=0;z<4;z++)
	  if (order[which]==buttonjoy[z])
	  {
	   buttonjoy[z]=bt_nobutton;
	   break;
	  }

	buttonjoy[result-1]=order[which];
	picked=1;
	SD_PlaySound(SHOOTDOORSND);
	   }
	   break;

	 case KEYBOARDBTNS:
	   if (LastScan)
	   {
	buttonscan[order[which]]=LastScan;
	picked=1;
	ShootSnd();
	IN_ClearKeysDown();
	   }
	   break;

	 case KEYBOARDMOVE:
	   if (LastScan)
	   {
	dirscan[moveorder[which]]=LastScan;
	picked=1;
	ShootSnd();
	IN_ClearKeysDown();
	   }
	   break;
	}

	//
	// EXIT INPUT?
	//
	if (IN_KeyDown(sc_Escape))
	{
	 picked=1;
	 continue;
	}

   } while(!picked);

   SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
   redraw=1;
   WaitKeyUp();
   continue;
  }

  if (ci.button1 || IN_KeyDown(sc_Escape))
	exit=1;

  //
  // MOVE TO ANOTHER SPOT?
  //
  switch(ci.dir)
  {
   case dir_West:
	 do
	 {
	  which--;
	  if (which<0)
	which=3;
	 } while(!cust->allowed[which]);
	 redraw=1;
	 SD_PlaySound(MOVEGUN1SND);
	 while(ReadAnyControl(&ci),ci.dir!=dir_None)
	   BE_ST_ShortSleep();
	 IN_ClearKeysDown();
	 break;

   case dir_East:
	 do
	 {
	  which++;
	  if (which>3)
	which=0;
	 } while(!cust->allowed[which]);
	 redraw=1;
	 SD_PlaySound(MOVEGUN1SND);
	 while(ReadAnyControl(&ci),ci.dir!=dir_None)
	   BE_ST_ShortSleep();
	 IN_ClearKeysDown();
	 break;
   case dir_North:
   case dir_South:
	 exit=1;
  }
  BE_ST_ShortSleep();
 } while(!exit);

 SD_PlaySound(ESCPRESSEDSND);
 WaitKeyUp();
 DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
}


////////////////////////
//
// FIXUP GUN CURSOR OVERDRAW SHIT
//
void FixupCustom(id0_int_t w)
{
	static id0_int_t lastwhich=-1;
	id0_int_t y=CST_Y+26+w*13;


	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	VWB_Hlin(7,32,y-2,BORDCOLOR);
	VWB_Hlin(7,32,y-1,DEACTIVE);
	VWB_Hlin(7,32,y+12,BORD2COLOR);
	VWB_Hlin(7,32,y+13,BORDCOLOR);
#else
	VWB_Hlin(7,32,y-1,DEACTIVE);
	VWB_Hlin(7,32,y+12,BORD2COLOR);
#ifndef SPEAR
	VWB_Hlin(7,32,y-2,BORDCOLOR);
	VWB_Hlin(7,32,y+13,BORDCOLOR);
#else
	VWB_Hlin(7,32,y-2,BORD2COLOR);
	VWB_Hlin(7,32,y+13,BORD2COLOR);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL6AP11

	switch(w)
	{
		case 0: DrawCustMouse(1); break;
		case 3: DrawCustJoy(1); break;
		case 6: DrawCustKeybd(1); break;
		case 8: DrawCustKeys(1);
	}


	if (lastwhich>=0)
	{
		y=CST_Y+26+lastwhich*13;
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		VWB_Hlin(7,32,y-2,BORDCOLOR);
		VWB_Hlin(7,32,y-1,DEACTIVE);
		VWB_Hlin(7,32,y+12,BORD2COLOR);
		VWB_Hlin(7,32,y+13,BORDCOLOR);
#else
		VWB_Hlin(7,32,y-1,DEACTIVE);
		VWB_Hlin(7,32,y+12,BORD2COLOR);
#ifndef SPEAR
		VWB_Hlin(7,32,y-2,BORDCOLOR);
		VWB_Hlin(7,32,y+13,BORDCOLOR);
#else
		VWB_Hlin(7,32,y-2,BORD2COLOR);
		VWB_Hlin(7,32,y+13,BORD2COLOR);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL6AP11

		if (lastwhich!=w)
			switch(lastwhich)
			{
				case 0: DrawCustMouse(0); break;
				case 3: DrawCustJoy(0); break;
				case 6: DrawCustKeybd(0); break;
				case 8: DrawCustKeys(0);
			}
	}

	lastwhich=w;
}


////////////////////////
//
// DRAW CUSTOMIZE SCREEN
//
void DrawCustomScreen(void)
{
	id0_int_t i;


#ifdef JAPAN
	CA_CacheScreen(S_CUSTOMPIC);
	fontnumber=1;

	PrintX=CST_START;
	PrintY = CST_Y+26;
	DrawCustMouse(0);

	PrintX=CST_START;
	US_Print("\n\n\n");
	DrawCustJoy(0);

	PrintX=CST_START;
	US_Print("\n\n\n");
	DrawCustKeybd(0);

	PrintX=CST_START;
	US_Print("\n\n\n");
	DrawCustKeys(0);
#else
	ClearMScreen();
	WindowX=0;
	WindowW=320;
	VWB_DrawPic(112,GAMEVER_MOUSELBACKY,C_MOUSELBACKPIC);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintY = 5;
	US_CPrint("Customize Controls");
#else
	DrawStripes(10);
	VWB_DrawPic(80,0,C_CUSTOMIZEPIC);
#endif

	//
	// MOUSE
	//
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	WindowX=0;
	WindowW=320;

#ifndef SPEAR
	PrintY=CST_Y;
	US_CPrint("Mouse\n");
#else
	PrintY = CST_Y+13;
	VWB_DrawPic (128,48,C_MOUSEPIC);
#endif

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	#ifdef SPANISH
	PrintX=CST_START-16;
	US_Print(STR_CRUN);
	PrintX=CST_START-16+CST_SPC*1;
	US_Print(STR_COPEN);
	PrintX=CST_START-16+CST_SPC*2;
	US_Print(STR_CFIRE);
	PrintX=CST_START-16+CST_SPC*3;
	US_Print(STR_CSTRAFE"\n");
	#else
	PrintX=CST_START;
	US_Print(STR_CRUN);
	PrintX=CST_START+CST_SPC*1;
	US_Print(STR_COPEN);
	PrintX=CST_START+CST_SPC*2;
	US_Print(STR_CFIRE);
	PrintX=CST_START+CST_SPC*3;
	US_Print(STR_CSTRAFE"\n");
	#endif

	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustMouse(0);
	US_Print("\n");


	//
	// JOYSTICK/PAD
	//
#ifndef SPEAR
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	US_CPrint("Joystick/PC GamePad\n");
#else
	US_CPrint("Joystick/Gravis GamePad\n");
#endif
#else
	PrintY += 13;
	VWB_DrawPic (40,88,C_JOYSTICKPIC);
#endif

#ifdef SPEAR
	VWB_DrawPic (112,120,C_KEYBOARDPIC);
#endif

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	#ifdef SPANISH
	PrintX=CST_START-16;
	US_Print(STR_CRUN);
	PrintX=CST_START-16+CST_SPC*1;
	US_Print(STR_COPEN);
	PrintX=CST_START-16+CST_SPC*2;
	US_Print(STR_CFIRE);
	PrintX=CST_START-16+CST_SPC*3;
	US_Print(STR_CSTRAFE"\n");
	#else
	PrintX=CST_START;
	US_Print(STR_CRUN);
	PrintX=CST_START+CST_SPC*1;
	US_Print(STR_COPEN);
	PrintX=CST_START+CST_SPC*2;
	US_Print(STR_CFIRE);
	PrintX=CST_START+CST_SPC*3;
	US_Print(STR_CSTRAFE"\n");
	#endif
	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustJoy(0);
	US_Print("\n");


	//
	// KEYBOARD
	//
#ifndef SPEAR
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	US_CPrint("Keyboard\n");
#else
	PrintY += 13;
#endif
	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	#ifdef SPANISH
	PrintX=CST_START-16;
	US_Print(STR_CRUN);
	PrintX=CST_START-16+CST_SPC*1;
	US_Print(STR_COPEN);
	PrintX=CST_START-16+CST_SPC*2;
	US_Print(STR_CFIRE);
	PrintX=CST_START-16+CST_SPC*3;
	US_Print(STR_CSTRAFE"\n");
	#else
	PrintX=CST_START;
	US_Print(STR_CRUN);
	PrintX=CST_START+CST_SPC*1;
	US_Print(STR_COPEN);
	PrintX=CST_START+CST_SPC*2;
	US_Print(STR_CFIRE);
	PrintX=CST_START+CST_SPC*3;
	US_Print(STR_CSTRAFE"\n");
	#endif
	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustKeybd(0);
	US_Print("\n");


	//
	// KEYBOARD MOVE KEYS
	//
	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	#ifdef SPANISH
	PrintX=4;
	US_Print(STR_LEFT);
	US_Print("/");
	US_Print(STR_RIGHT);
	US_Print("/");
	US_Print(STR_FRWD);
	US_Print("/");
	US_Print(STR_BKWD"\n");
	#else
	PrintX=CST_START;
	US_Print(STR_LEFT);
	PrintX=CST_START+CST_SPC*1;
	US_Print(STR_RIGHT);
	PrintX=CST_START+CST_SPC*2;
	US_Print(STR_FRWD);
	PrintX=CST_START+CST_SPC*3;
	US_Print(STR_BKWD"\n");
	#endif
	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustKeys(0);
#endif
	//
	// PICK STARTING POINT IN MENU
	//
	if (CusItems.curpos<0)
		for (i=0;i<CusItems.amount;i++)
			if (CusMenu[i].active)
			{
				CusItems.curpos=i;
				break;
			}


	VW_UpdateScreen();
	MenuFadeIn();
}


void PrintCustMouse(id0_int_t i)
{
	id0_int_t j;

	for (j=0;j<4;j++)
		if (order[i]==buttonmouse[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}

void DrawCustMouse(id0_int_t hilight)
{
	id0_int_t i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	if (!mouseenabled)
	{
		SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
		CusMenu[0].active=0;
	}
	else
		CusMenu[0].active=1;

	PrintY=CST_Y+13*2;
	for (i=0;i<4;i++)
		PrintCustMouse(i);
}

void PrintCustJoy(id0_int_t i)
{
	id0_int_t j;

	for (j=0;j<4;j++)
		if (order[i]==buttonjoy[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}

void DrawCustJoy(id0_int_t hilight)
{
	id0_int_t i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	if (!joystickenabled)
	{
		SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
		CusMenu[3].active=0;
	}
	else
		CusMenu[3].active=1;

	PrintY=CST_Y+13*5;
	for (i=0;i<4;i++)
		PrintCustJoy(i);
}


void PrintCustKeybd(id0_int_t i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(buttonscan[order[i]]));
}

void DrawCustKeybd(id0_int_t hilight)
{
	id0_int_t i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	PrintY=CST_Y+13*8;
	for (i=0;i<4;i++)
		PrintCustKeybd(i);
}

void PrintCustKeys(id0_int_t i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(dirscan[moveorder[i]]));
}

void DrawCustKeys(id0_int_t hilight)
{
	id0_int_t i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	PrintY=CST_Y+13*10;
	for (i=0;i<4;i++)
		PrintCustKeys(i);
}


////////////////////////////////////////////////////////////////////
//
// CHANGE SCREEN VIEWING SIZE
//
////////////////////////////////////////////////////////////////////
CP_routineret CP_ChangeView(CP_routineargs)
{
	id0_int_t exit=0,oldview,newview;
	ControlInfo ci;


	WindowX=WindowY=0;
	WindowW=320;
	WindowH=200;
	newview=oldview=viewwidth/16;
	DrawChangeView(oldview);

	do
	{
		CheckPause();
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
		case dir_South:
		case dir_West:
			newview--;
			if (newview<4)
				newview=4;
			ShowViewSize(newview);
			VW_UpdateScreen();
			SD_PlaySound(HITWALLSND);
			TicDelay(10);
			break;

		case dir_North:
		case dir_East:
			newview++;
			if (newview>19)
				newview=19;
			ShowViewSize(newview);
			VW_UpdateScreen();
			SD_PlaySound(HITWALLSND);
			TicDelay(10);
			break;
		}

		// *** SHAREWARE/REGISTERED V1.4 APOGEE+SOD (DEMO) V1.0 RESTORATION ***
		// And again
		#if (GAMEVER_WOLFREV > GV_WR_SODFG14A)
		#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		#else
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm("debugmode"))
		#endif
			PicturePause();
		#endif

		if (ci.button0 || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
		{
			viewwidth=oldview*16;
			SD_PlaySound(ESCPRESSEDSND);
			MenuFadeOut();
			return CP_itemvalue;
		}
		BE_ST_ShortSleep();

	} while(!exit);


	if (oldview!=newview)
	{
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		SD_PlaySound (SHOOTSND);
		Message(STR_THINK"...");
#endif
		NewViewSize(newview);
	}

	ShootSnd();
	MenuFadeOut();
	return CP_itemvalue;
}


/////////////////////////////
//
// DRAW THE CHANGEVIEW SCREEN
//
void DrawChangeView(id0_int_t view)
{
#ifdef JAPAN
	CA_CacheScreen(S_CHANGEPIC);

	ShowViewSize(view);
#else
	// *** ALPHA RESTORATION ***
	// Note that the color is hardcoded here
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VWB_Bar(0,0,320,200,3);
#endif
	VWB_Bar(0,160,320,40,VIEWCOLOR);
	ShowViewSize(view);

	PrintY=161;
	WindowX=0;
	WindowY=320;
	SETFONTCOLOR(HIGHLIGHT,BKGDCOLOR);

	US_CPrint(STR_SIZE1"\n");
	US_CPrint(STR_SIZE2"\n");
	US_CPrint(STR_SIZE3);
#endif
	VW_UpdateScreen();

	MenuFadeIn();
}


////////////////////////////////////////////////////////////////////
//
// QUIT THIS INFERNAL GAME!
//
////////////////////////////////////////////////////////////////////
void CP_Quit(void)
{
	id0_int_t i;


	#ifdef JAPAN
	if (GetYorN(7,11,C_QUITMSGPIC))
	#else

	#ifdef SPANISH
	if (Confirm(ENDGAMESTR))
	// *** ALPHA RESTORATION ***
	// This also makes use of QUITSUR string (unused in v1.0+)
	#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (Confirm(QUITSUR))
	#else
	if (Confirm(endStrings[US_RndT()&0x7+(US_RndT()&1)]))
	#endif

	#endif
	{
		VW_UpdateScreen();
		SD_MusicOff();
		SD_StopSound();
		MenuFadeOut();
		//
		// SHUT-UP THE ADLIB
		//
		for (i=1;i<=0xf5;i++)
			alOut(i,0);
		Quit(NULL);
	}

	DrawMainMenu();
}


// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
////////////////////////////////////////////////////////////////////
//
// HANDLE INTRO SCREEN (SYSTEM CONFIG)
//
////////////////////////////////////////////////////////////////////
void IntroScreen(void)
{
#ifdef SPEAR

#define MAINCOLOR	0x4f
#define EMSCOLOR	0x4f
#define XMSCOLOR	0x4f

// *** ALPHA RESTORATION ***
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)

#define MAINCOLOR	0xa
#define EMSCOLOR	0xb
#define XMSCOLOR	0xc

#else

#define MAINCOLOR	0x6c
#define EMSCOLOR	0x6c
#define XMSCOLOR	0x6c

#endif
#define FILLCOLOR	14

	id0_long_t memory,emshere,xmshere;
	// *** ALPHA RESTORATION ***
	// Looks like num was used in the alpha, but forgotten later...
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_int_t i,num,ems[10]={256,512,1000,1500,2000,2500,3000,3500,4000},
		xms[10]={512,1000,2000,3000,4000,5000,6000,7000,8000};
#else
	id0_int_t i,num,ems[10]={100,200,300,400,500,600,700,800,900,1000},
		xms[10]={100,200,300,400,500,600,700,800,900,1000},
		main[10]={32,64,96,128,160,192,224,256,288,320};
#endif

	// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	StartCPMusic(SALUTE_MUS);
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	StartCPMusic(NAZI_NOR_MUS);
#endif
	///

	//
	// DRAW MAIN MEMORY
	//
	memory=(1023l+mminfo.nearheap+mminfo.farheap)/1024l;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	num=(63l+memory)/64l;
	for (i=0;i<num;i++)
		VWB_Bar(49,163-8*i,6,5,MAINCOLOR);
#else
	for (i=0;i<10;i++)
		if (memory>=main[i])
			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			VWB_Bar(49,163-8*i,6,5,MAINCOLOR-2*i);
#else
			VWB_Bar(49,163-8*i,6,5,MAINCOLOR-i);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312


	//
	// DRAW EMS MEMORY
	//
	if (EMSPresent)
	{
		emshere=4l*EMSPagesAvail;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		num=10;
		for (i=0;i<10;i++)
			if (emshere<ems[i])
			{
				num=i+1;
				break;
			}
		for (i=0;i<num;i++)
			VWB_Bar(89,163-8*i,6,5,EMSCOLOR);
#else
		for (i=0;i<10;i++)
			if (emshere>=ems[i])
			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
				VWB_Bar(89,163-8*i,6,5,EMSCOLOR-2*i);
#else
				VWB_Bar(89,163-8*i,6,5,EMSCOLOR-i);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
	}

	//
	// DRAW XMS MEMORY
	//
	if (XMSPresent)
	{
		xmshere=4l*XMSPagesAvail;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		num=10;
		for (i=0;i<10;i++)
			if (xmshere<xms[i])
			{
				num=i+1;
				break;
			}
		for (i=0;i<num;i++)
			VWB_Bar(129,163-8*i,6,5,XMSCOLOR);
#else
		for (i=0;i<10;i++)
			if (xmshere>=xms[i])
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
				VWB_Bar(129,163-8*i,6,5,XMSCOLOR-2*i);
#else
				VWB_Bar(129,163-8*i,6,5,XMSCOLOR-i);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
	}

	//
	// FILL BOXES
	//
	if (MousePresent)
		VWB_Bar(164,82,12,2,FILLCOLOR);

	if (JoysPresent[0] || JoysPresent[1])
		VWB_Bar(164,105,12,2,FILLCOLOR);

	if (AdLibPresent && !SoundBlasterPresent)
		VWB_Bar(164,128,12,2,FILLCOLOR);

	if (SoundBlasterPresent)
		VWB_Bar(164,151,12,2,FILLCOLOR);

	if (SoundSourcePresent)
		VWB_Bar(164,174,12,2,FILLCOLOR);
}
#endif


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
// SUPPORT ROUTINES
//
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//
// Clear Menu screens to dark red
//
////////////////////////////////////////////////////////////////////
void ClearMScreen(void)
{
#ifndef SPEAR
	VWB_Bar(0,0,320,200,BORDCOLOR);
#else
	VWB_DrawPic(0,0,C_BACKDROPPIC);
#endif
}


////////////////////////////////////////////////////////////////////
//
// Un/Cache a LUMP of graphics
//
////////////////////////////////////////////////////////////////////
void CacheLump(id0_int_t lumpstart,id0_int_t lumpend)
{
 id0_int_t i;

 for (i=lumpstart;i<=lumpend;i++)
   CA_CacheGrChunk(i);
}


void UnCacheLump(id0_int_t lumpstart,id0_int_t lumpend)
{
 id0_int_t i;

 for (i=lumpstart;i<=lumpend;i++)
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (grsegs[i])
#endif
		UNCACHEGRCHUNK(i);
}


////////////////////////////////////////////////////////////////////
//
// Draw a window for a menu
//
////////////////////////////////////////////////////////////////////
void DrawWindow(id0_int_t x,id0_int_t y,id0_int_t w,id0_int_t h,id0_int_t wcolor)
{
	VWB_Bar(x,y,w,h,wcolor);
	DrawOutline(x,y,w,h,BORD2COLOR,DEACTIVE);
}


void DrawOutline(id0_int_t x,id0_int_t y,id0_int_t w,id0_int_t h,id0_int_t color1,id0_int_t color2)
{
	VWB_Hlin(x,x+w,y,color2);
	VWB_Vlin(y,y+h,x,color2);
	VWB_Hlin(x,x+w,y+h,color1);
	VWB_Vlin(y,y+h,x+w,color1);
}


////////////////////////////////////////////////////////////////////
//
// Setup Control Panel stuff - graphics, etc.
//
////////////////////////////////////////////////////////////////////
void SetupControlPanel(void)
{
#if REFKEEN_ENABLE_FILE_SEARCH
	struct ffblk f;
#endif
	id0_char_t name[13];
	id0_int_t which,i;


	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// Some v1.0 specific function, guessing it's VW_InitDoubleBuffer
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	VW_InitDoubleBuffer();
#endif
	//
	// CACHE GRAPHICS & SOUNDS
	//
	CA_CacheGrChunk(STARTFONT+1);
#ifndef SPEAR
	CacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#else
	CacheLump(BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#endif

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	fontnumber=1;
	WindowH=200;

	if (!ingame)
		CA_LoadAllSounds();
	else
		MainMenu[savegame].active=1;

#if REFKEEN_ENABLE_FILE_SEARCH
	//
	// SEE WHICH SAVE GAME FILES ARE AVAILABLE & READ STRING IN
	//
	strcpy(name,SaveName);
	if (!findfirst(name,&f,0))
		do
		{
			which=f.ff_name[7]-'0';
			if (which<10)
			{
				BE_FILE_T handle;
				id0_char_t temp[32];

				SaveGamesAvail[which]=1;
				handle=BE_Cross_open_rewritable_for_reading(f.ff_name);
//				handle=open(f.ff_name,O_BINARY);
				BE_Cross_readInt8LEBuffer(handle,temp,32);
				BE_Cross_close(handle);
				strcpy(&SaveGameNames[which][0],temp);
			}
		} while(!findnext(&f));
#endif

	//
	// CENTER MOUSE
	//
	BE_ST_SetEmuMousePos(CENTER,CENTER);
//	_CX=_DX=CENTER;
//	Mouse(4);
}


////////////////////////////////////////////////////////////////////
//
// Clean up all the Control Panel stuff
//
////////////////////////////////////////////////////////////////////
void CleanupControlPanel(void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	UNCACHEGRCHUNK(STARTFONT+1);
#endif
#ifndef SPEAR
	UnCacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#else
	UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#endif

	fontnumber = 0;
}


////////////////////////////////////////////////////////////////////
//
// Handle moving gun around a menu
//
////////////////////////////////////////////////////////////////////
id0_int_t HandleMenu(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,void (*routine)(id0_int_t w))
{
	id0_char_t key;
	static id0_int_t redrawitem=1,lastitem=-1;
	id0_int_t i,x,y,basey,exit,which,shape,timer;
	ControlInfo ci;


	which=item_i->curpos;
	x=item_i->x&-8;
	basey=item_i->y-2;
	y=basey+which*13;

	VWB_DrawPic(x,y,C_CURSOR1PIC);
	SetTextColor(items+which,1);
	if (redrawitem)
	{
		PrintX=item_i->x+item_i->indent;
		PrintY=item_i->y+which*13;
		US_Print((items+which)->string);
	}
	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//
	if (routine)
		routine(which);
	VW_UpdateScreen();

	shape=C_CURSOR1PIC;
	timer=8;
	exit=0;
	SD_AddToTimeCount(-SD_GetTimeCount());
//	TimeCount=0;
	IN_ClearKeysDown();


	do
	{
		BE_ST_ShortSleep();
		//
		// CHANGE GUN SHAPE
		//
		if (SD_GetTimeCount()>timer)
		{
			SD_AddToTimeCount(-SD_GetTimeCount());
//			TimeCount=0;
			if (shape==C_CURSOR1PIC)
			{
				shape=C_CURSOR2PIC;
				timer=8;
			}
			else
			{
				shape=C_CURSOR1PIC;
				timer=70;
			}
			VWB_DrawPic(x,y,shape);
			if (routine)
				routine(which);
			VW_UpdateScreen();
		}

		CheckPause();

		//
		// SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
		//
		key=LastASCII;
		if (key)
		{
			id0_int_t ok=0;

			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			//
			// CHECK FOR SCREEN CAPTURE
			//
			#ifndef SPEAR
			if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			#else
			if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm("debugmode"))
			#endif
				PicturePause();
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11


			if (key>='a')
				key-='a'-'A';

			for (i=which+1;i<item_i->amount;i++)
				if ((items+i)->active && (items+i)->string[0]==key)
				{
					EraseGun(item_i,items,x,y,which);
					which=i;
					DrawGun(item_i,items,x,&y,which,basey,routine);
					ok=1;
					IN_ClearKeysDown();
					break;
				}

			//
			// DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
			//
			if (!ok)
			{
				for (i=0;i<which;i++)
					if ((items+i)->active && (items+i)->string[0]==key)
					{
						EraseGun(item_i,items,x,y,which);
						which=i;
						DrawGun(item_i,items,x,&y,which,basey,routine);
						IN_ClearKeysDown();
						break;
					}
			}
		}

		//
		// GET INPUT
		//
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			////////////////////////////////////////////////
			//
			// MOVE UP
			//
			case dir_North:

			EraseGun(item_i,items,x,y,which);

			//
			// ANIMATE HALF-STEP
			//
			if (which && (items+which-1)->active)
			{
				y-=6;
				DrawHalfStep(x,y);
			}

			//
			// MOVE TO NEXT AVAILABLE SPOT
			//
			do
			{
				if (!which)
					which=item_i->amount-1;
				else
					which--;
			} while(!(items+which)->active);

			DrawGun(item_i,items,x,&y,which,basey,routine);
			//
			// WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
			//
			TicDelay(20);
			break;

			////////////////////////////////////////////////
			//
			// MOVE DOWN
			//
			case dir_South:

			EraseGun(item_i,items,x,y,which);
			//
			// ANIMATE HALF-STEP
			//
			if (which!=item_i->amount-1 && (items+which+1)->active)
			{
				y+=6;
				DrawHalfStep(x,y);
			}

			do
			{
				if (which==item_i->amount-1)
					which=0;
				else
					which++;
			} while(!(items+which)->active);

			DrawGun(item_i,items,x,&y,which,basey,routine);

			//
			// WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
			//
			TicDelay(20);
			break;
		}

		if (ci.button0 ||
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			Keyboard[sc_Space] ||
#endif
			Keyboard[sc_Enter])
				exit=1;

		if (ci.button1 ||
			Keyboard[sc_Escape])
				exit=2;

	} while(!exit);


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	while (Keyboard[sc_Enter] || Keyboard[sc_Escape])
		BE_ST_ShortSleep();
#else
	IN_ClearKeysDown();
#endif

	//
	// ERASE EVERYTHING
	//
	if (lastitem!=which)
	{
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		VWB_Bar(x-1,y,24,16,BKGDCOLOR);
#else
		VWB_Bar(x-1,y,25,16,BKGDCOLOR);
#endif
		PrintX=item_i->x+item_i->indent;
		PrintY=item_i->y+which*13;
		US_Print((items+which)->string);
		redrawitem=1;
	}
	else
		redrawitem=0;

	if (routine)
		routine(which);
	VW_UpdateScreen();

	item_i->curpos=which;

	lastitem=which;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	WaitKeyUp();
#endif
	switch(exit)
	{
		case 1:
			//
			// CALL THE ROUTINE
			//
			if ((items+which)->routine!=NULL)
			{
				ShootSnd();
				MenuFadeOut();
				// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
				(items+which)->routine();
#else
				(items+which)->routine(0);
#endif
			}
			return which;

		case 2:
			SD_PlaySound(ESCPRESSEDSND);
			return -1;
	}

	return 0; // JUST TO SHUT UP THE ERROR MESSAGES!
}


//
// ERASE GUN & DE-HIGHLIGHT STRING
//
void EraseGun(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,id0_int_t x,id0_int_t y,id0_int_t which)
{
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	VWB_Bar(x-1,y,24,16,BKGDCOLOR);
#else
	VWB_Bar(x-1,y,25,16,BKGDCOLOR);
#endif
	SetTextColor(items+which,0);

	PrintX=item_i->x+item_i->indent;
	PrintY=item_i->y+which*13;
	US_Print((items+which)->string);
	VW_UpdateScreen();
}


//
// DRAW HALF STEP OF GUN TO NEXT POSITION
//
void DrawHalfStep(id0_int_t x,id0_int_t y)
{
	VWB_DrawPic(x,y,C_CURSOR1PIC);
	VW_UpdateScreen();
	SD_PlaySound(MOVEGUN1SND);
	SD_SetTimeCount(0);
	SD_TimeCountWaitForDest(8);
//	while(TimeCount<8);
}


//
// DRAW GUN AT NEW POSITION
//
void DrawGun(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,id0_int_t x,id0_int_t *y,id0_int_t which,id0_int_t basey,void (*routine)(id0_int_t w))
{
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	VWB_Bar(x-1,*y,24,16,BKGDCOLOR);
#else
	VWB_Bar(x-1,*y,25,16,BKGDCOLOR);
#endif
	*y=basey+which*13;
	VWB_DrawPic(x,*y,C_CURSOR1PIC);
	SetTextColor(items+which,1);

	PrintX=item_i->x+item_i->indent;
	PrintY=item_i->y+which*13;
	US_Print((items+which)->string);

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//
	if (routine)
		routine(which);
	VW_UpdateScreen();
	SD_PlaySound(MOVEGUN2SND);
}

////////////////////////////////////////////////////////////////////
//
// DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
//
////////////////////////////////////////////////////////////////////
void TicDelay(id0_int_t count)
{
	ControlInfo ci;


	SD_SetTimeCount(0);
	do
	{
		ReadAnyControl(&ci);
		BE_ST_ShortSleep();
	} while(SD_GetTimeCount()<count && ci.dir!=dir_None);
}


////////////////////////////////////////////////////////////////////
//
// Draw a menu
//
////////////////////////////////////////////////////////////////////
void DrawMenu(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items)
{
	id0_int_t i,which=item_i->curpos;


	WindowX=PrintX=item_i->x+item_i->indent;
	WindowY=PrintY=item_i->y;
	WindowW=320;
	WindowH=200;

	for (i=0;i<item_i->amount;i++)
	{
		SetTextColor(items+i,which==i);

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		PrintY=item_i->y+i*13;
#endif
		if ((items+i)->active)
			US_Print((items+i)->string);
		else
		{
			SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
			US_Print((items+i)->string);
			SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
		}

		US_Print("\n");
	}
}


////////////////////////////////////////////////////////////////////
//
// SET TEXT COLOR (HIGHLIGHT OR NO)
//
////////////////////////////////////////////////////////////////////
void SetTextColor(CP_itemtype GAMEVER_COND_FARPTR *items,id0_int_t hlight)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (items->active == 1)
	{
		if (hlight)
			{SETFONTCOLOR(HIGHLIGHT,BKGDCOLOR);}
		else
			{SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);}
	}
	else if (items->active == 2)
	{
		if (hlight)
			{SETFONTCOLOR(READHCOLOR,BKGDCOLOR);}
		else
			{SETFONTCOLOR(READCOLOR,BKGDCOLOR);}
	}
#else
	if (hlight)
		{SETFONTCOLOR(color_hlite[items->active],BKGDCOLOR);}
	else
		{SETFONTCOLOR(color_norml[items->active],BKGDCOLOR);}
#endif
}


////////////////////////////////////////////////////////////////////
//
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
//
////////////////////////////////////////////////////////////////////
void WaitKeyUp(void)
{
	ControlInfo ci;
	while(ReadAnyControl(&ci),	ci.button0|
								ci.button1|
								ci.button2|
								ci.button3|
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
								Keyboard[sc_Space]|
#endif
								Keyboard[sc_Enter]|
								Keyboard[sc_Escape])
		BE_ST_ShortSleep();
}


////////////////////////////////////////////////////////////////////
//
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
//
////////////////////////////////////////////////////////////////////
void ReadAnyControl(ControlInfo *ci)
{
	id0_int_t mouseactive=0;


	IN_ReadControl(0,ci);

	if (mouseenabled)
	{
		id0_int_t mousey,mousex;


		// READ MOUSE MOTION COUNTERS
		// RETURN DIRECTION
		// HOME MOUSE
		// CHECK MOUSE BUTTONS

		BE_ST_GetEmuMousePos(&mousex,&mousey);
//		Mouse(3);
//		mousex=_CX;
//		mousey=_DX;

		if (mousey<CENTER-SENSITIVE)
		{
			ci->dir=dir_North;
			BE_ST_SetEmuMousePos(CENTER,CENTER);
//			_CX=_DX=CENTER;
//			Mouse(4);
			mouseactive=1;
		}
		else
		if (mousey>CENTER+SENSITIVE)
		{
			ci->dir=dir_South;
			BE_ST_SetEmuMousePos(CENTER,CENTER);
//			_CX=_DX=CENTER;
//			Mouse(4);
			mouseactive=1;
		}

		if (mousex<CENTER-SENSITIVE)
		{
			ci->dir=dir_West;
			BE_ST_SetEmuMousePos(CENTER,CENTER);
//			_CX=_DX=CENTER;
//			Mouse(4);
			mouseactive=1;
		}
		else
		if (mousex>CENTER+SENSITIVE)
		{
			ci->dir=dir_East;
			BE_ST_SetEmuMousePos(CENTER,CENTER);
//			_CX=_DX=CENTER;
//			Mouse(4);
			mouseactive=1;
		}

		if (IN_MouseButtons())
		{
			ci->button0=IN_MouseButtons()&1;
			ci->button1=IN_MouseButtons()&2;
			ci->button2=IN_MouseButtons()&4;
			ci->button3=false;
			mouseactive=1;
		}
	}

	if (joystickenabled && !mouseactive)
	{
		id0_int_t jx,jy,jb;


		INL_GetJoyDelta(joystickport,&jx,&jy);
		if (jy<-SENSITIVE)
			ci->dir=dir_North;
		else
		if (jy>SENSITIVE)
			ci->dir=dir_South;

		if (jx<-SENSITIVE)
			ci->dir=dir_West;
		else
		if (jx>SENSITIVE)
			ci->dir=dir_East;

		jb=IN_JoyButtons();
		if (jb)
		{
			ci->button0=jb&1;
			ci->button1=jb&2;
			if (joypadenabled)
			{
				ci->button2=jb&4;
				ci->button3=jb&8;
			}
			else
				ci->button2=ci->button3=false;
		}
	}
}


////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
id0_int_t Confirm(id0_char_t GAMEVER_COND_FARPTR *string)
{
	soundnames whichsnd[2]={ESCPRESSEDSND,SHOOTSND}; // REFKEEN: For C++ build
	// *** ALPHA RESTORATION ***
	// Originally the code from the Message function was *hardcoded* here
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_int_t xit=0,h=0,w=0,mw=0,i,x,y,tick=0,time/*,whichsnd[2]={ESCPRESSEDSND,SHOOTSND}*/;
	fontstruct id0_seg *font;


	fontnumber=1;
	font=(fontstruct *)grsegs[STARTFONT+fontnumber];
	h=font->height;
	for (i=0;i<strlen(string);i++)
		if (string[i]=='\n')
		{
			if (w>mw)
				mw=w;
			w=0;
			h+=font->height;
		}
		else
			w+=font->width[string[i]];

	if (w+10>mw)
		mw=w+10;

	PrintY=(WindowH/2)-h/2;
	PrintX=WindowX=160-mw/2;

	DrawWindow(WindowX-5,PrintY-5,mw+10,h+10,TEXTCOLOR);
	DrawOutline(WindowX-5,PrintY-5,mw+10,h+10,0,HIGHLIGHT);
	SETFONTCOLOR(0,TEXTCOLOR);
	US_Print(string);
	VW_UpdateScreen();
#else // GAMEVER_WOLFREV > GV_WR_WL920312
	id0_int_t xit=0,i,x,y,tick=0,time/*,whichsnd[2]={ESCPRESSEDSND,SHOOTSND}*/;


	Message(string);
#endif
	IN_ClearKeysDown();

	//
	// BLINK CURSOR
	//
	x=PrintX;
	y=PrintY;
	SD_AddToTimeCount(-SD_GetTimeCount());
//	TimeCount=0;

	do
	{
		if (SD_GetTimeCount()>=10)
		{
			switch(tick)
			{
				case 0:
					VWB_Bar(x,y,8,13,TEXTCOLOR);
					break;
				case 1:
					PrintX=x;
					PrintY=y;
					US_Print("_");
			}
			VW_UpdateScreen();
			tick^=1;
			SD_AddToTimeCount(-SD_GetTimeCount());
//			TimeCount=0;
		}

		// *** SHAREWARE/REGISTERED APOGEE + S3DNA RESTORATION ***
		// And again
		#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A) && (!defined GAMEVER_NOAH3D)
		//#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			PicturePause();
		#endif

		BE_ST_ShortSleep();
	#ifdef SPANISH
	} while(!Keyboard[sc_S] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
	#else
	} while(!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
	#endif

	#ifdef SPANISH
	if (Keyboard[sc_S])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_S] || Keyboard[sc_N] || Keyboard[sc_Escape])
		BE_ST_ShortSleep();

	#else

	if (Keyboard[sc_Y])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape])
		BE_ST_ShortSleep();
	#endif

	IN_ClearKeysDown();
	SD_PlaySound(whichsnd[xit]);
	return xit;
}

#ifdef JAPAN
////////////////////////////////////////////////////////////////////
//
// DRAW MESSAGE & GET Y OR N
//
////////////////////////////////////////////////////////////////////
id0_int_t GetYorN(id0_int_t x,id0_int_t y,id0_int_t pic)
{
	soundnames whichsnd[2]={ESCPRESSEDSND,SHOOTSND}; // REFKEEN: For C++ build
	id0_int_t xit=0/*,whichsnd[2]={ESCPRESSEDSND,SHOOTSND}*/;


	CA_CacheGrChunk(pic);
	VWB_DrawPic(x * 8,y * 8,pic);
	UNCACHEGRCHUNK(pic);
	VW_UpdateScreen();
	IN_ClearKeysDown();

	do
	{
		BE_ST_ShortSleep();
		// *** SHAREWARE/REGISTERED APOGEE RESTORATION ***
		// And again
		#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A)
		//#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			PicturePause();
		#endif

	#ifdef SPANISH
	} while(!Keyboard[sc_S] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
	#else
	} while(!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
	#endif

	#ifdef SPANISH
	if (Keyboard[sc_S])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_S] || Keyboard[sc_N] || Keyboard[sc_Escape])
		BE_ST_ShortSleep();

	#else

	if (Keyboard[sc_Y])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape])
		BE_ST_ShortSleep();
	#endif

	IN_ClearKeysDown();
	SD_PlaySound(whichsnd[xit]);
	return xit;
}
#endif


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
////////////////////////////////////////////////////////////////////
//
// PRINT A MESSAGE IN A WINDOW
//
////////////////////////////////////////////////////////////////////
void Message(id0_char_t GAMEVER_COND_FARPTR *string)
{
	id0_int_t h=0,w=0,mw=0,i,x,y,time;
	fontstruct id0_seg *font;


	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	CA_CacheGrChunk (STARTFONT+1);
#endif
	fontnumber=1;
	font=(fontstruct *)grsegs[STARTFONT+fontnumber];
	h=font->height;
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if 1 // REFKEEN: Just use strlen
//#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	for (i=0;i<strlen(string);i++)
#else
	for (i=0;i<_fstrlen(string);i++)
#endif
		if (string[i]=='\n')
		{
			if (w>mw)
				mw=w;
			w=0;
			h+=font->height;
		}
		else
			w+=font->width[string[i]];

	if (w+10>mw)
		mw=w+10;

	PrintY=(WindowH/2)-h/2;
	PrintX=WindowX=160-mw/2;

	DrawWindow(WindowX-5,PrintY-5,mw+10,h+10,TEXTCOLOR);
	DrawOutline(WindowX-5,PrintY-5,mw+10,h+10,0,HIGHLIGHT);
	SETFONTCOLOR(0,TEXTCOLOR);
	US_Print(string);
	VW_UpdateScreen();
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
////////////////////////////////////////////////////////////////////
//
// THIS MAY BE FIXED A LITTLE LATER...
//
////////////////////////////////////////////////////////////////////
static	musicnames/*id0_int_t*/	lastmusic; // REFKEEN: For C++ build
#endif

void StartCPMusic(musicnames/*id0_int_t*/ song)
{
	musicnames	chunk;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	if (audiosegs[STARTMUSIC + lastmusic])	// JDC
		MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + lastmusic]);
	lastmusic = song;
#endif

	SD_MusicOff();
	chunk =	song;

	MM_BombOnError (false);
	CA_CacheAudioChunk(STARTMUSIC + chunk);
	MM_BombOnError (true);
	if (mmerror)
		mmerror = false;
	else
	{
		MM_SetLock((memptr *)&audiosegs[STARTMUSIC + chunk],true);
		SD_StartMusic((MusicGroup id0_far *)audiosegs[STARTMUSIC + chunk]);
	}
}

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void FreeMusic (void)
{
	if (audiosegs[STARTMUSIC + lastmusic])	// JDC
		MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + lastmusic]);
}
#endif


///////////////////////////////////////////////////////////////////////////
//
//	IN_GetScanName() - Returns a string containing the name of the
//		specified scan code
//
///////////////////////////////////////////////////////////////////////////

// REFKEEN - const char is used instead of 8-bit unsigned int type for
// ScanNames, ExtScanCodes and ExtScanNames now, so fixes are required
const char *
//id0_byte_t *
IN_GetScanName(ScanCode scan)
{
	const char **p;
	const char id0_far *s;
//	id0_byte_t		**p;
//	ScanCode	id0_far *s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}


///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
//
///////////////////////////////////////////////////////////////////////////
void CheckPause(void)
{
	if (Paused)
	{
		switch(SoundStatus)
		{
			case 0: SD_MusicOn(); break;
			case 1: SD_MusicOff(); break;
		}

		SoundStatus^=1;
		VW_WaitVBL(3);
		IN_ClearKeysDown();
		Paused=false;
 }
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
//
///////////////////////////////////////////////////////////////////////////
void DrawMenuGun(CP_iteminfo *iteminfo)
{
	id0_int_t x,y;


	x=iteminfo->x;
	y=iteminfo->y+iteminfo->curpos*13-2;
	VWB_DrawPic(x,y,C_CURSOR1PIC);
}


// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
///////////////////////////////////////////////////////////////////////////
//
// DRAW SCREEN TITLE STRIPES
//
///////////////////////////////////////////////////////////////////////////
void DrawStripes(id0_int_t y)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	return;
#else
#ifndef SPEAR
	VWB_Bar(0,y,320,24,0);
	VWB_Hlin(0,319,y+22,STRIPE);
#else
	VWB_Bar(0,y,320,22,0);
	VWB_Hlin(0,319,y+23,0);
#endif
#endif
}

void ShootSnd(void)
{
	SD_PlaySound(SHOOTSND);
}


///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR EPISODES
//
///////////////////////////////////////////////////////////////////////////
void CheckForEpisodes(void)
{
#if !REFKEEN_ENABLE_FILE_SEARCH // TODO: Assume it's WL1/SDM/SOD and files are found

#ifdef SPEAR
#ifndef SPEARDEMO
	strcpy(extension,"SOD");
#else
	strcpy(extension,"SDM");
#endif
#else
#ifndef UPLOAD
#ifdef GAMEVER_NOAH3D
	strcpy(extension,"N3D");
#else
	strcpy(extension,"WL6");
#endif
	NewEmenu[2].active =
	NewEmenu[4].active =
	NewEmenu[6].active =
	NewEmenu[8].active =
	NewEmenu[10].active =
	EpisodeSelect[1] =
	EpisodeSelect[2] =
	EpisodeSelect[3] =
	EpisodeSelect[4] =
	EpisodeSelect[5] = 1;
#else
	strcpy(extension,"WL1");
#endif
#endif
	strcat(configname,extension);
	strcat(SaveName,extension);
	strcat(PageFileName,extension);
	strcat(audioname,extension);
#if (!defined SPEAR) && (!defined GOODTIMES) && (!defined GAMEVER_NOAH3D) // REFKEEN: But not if unneeded
	strcat(helpfilename,extension);
#endif
	strcat(demoname,extension);
#else // REFKEEN_ENABLE_FILE_SEARCH
	struct ffblk f;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!findfirst("*.N3D",&f,FA_ARCH))
	{
		strcpy(extension,"N3D");
		NewEmenu[2].active =
		NewEmenu[4].active =
		NewEmenu[6].active =
		NewEmenu[8].active =
		NewEmenu[10].active =
		EpisodeSelect[1] =
		EpisodeSelect[2] =
		EpisodeSelect[3] =
		EpisodeSelect[4] =
		EpisodeSelect[5] = 1;

		strcat(configname,extension);
		strcat(SaveName,extension);
		strcat(PageFileName,extension);
		strcat(audioname,extension);
		strcat(demoname,extension);
		return;
	}
	else
		Quit("Unable to find data files!\n");

#else
//
// JAPANESE VERSION
//
#ifdef JAPAN
#ifdef JAPDEMO
	if (!findfirst("*.WJ1",&f,FA_ARCH))
	{
		strcpy(extension,"WJ1");
#else
	if (!findfirst("*.WJ6",&f,FA_ARCH))
	{
		strcpy(extension,"WJ6");
#endif
		strcat(configname,extension);
		strcat(SaveName,extension);
		strcat(PageFileName,extension);
		strcat(audioname,extension);
		strcat(demoname,extension);
		EpisodeSelect[1] =
		EpisodeSelect[2] =
		EpisodeSelect[3] =
		EpisodeSelect[4] =
		EpisodeSelect[5] = 1;
	}
	else
		Quit("NO JAPANESE WOLFENSTEIN 3-D DATA FILES to be found!");
#else


//
// ENGLISH
//
#ifndef UPLOAD
#ifndef SPEAR
	if (!findfirst("*.WL6",&f,FA_ARCH))
	{
		strcpy(extension,"WL6");
		NewEmenu[2].active =
		NewEmenu[4].active =
		NewEmenu[6].active =
		NewEmenu[8].active =
		NewEmenu[10].active =
		EpisodeSelect[1] =
		EpisodeSelect[2] =
		EpisodeSelect[3] =
		EpisodeSelect[4] =
		EpisodeSelect[5] = 1;
	}
	else
	if (!findfirst("*.WL3",&f,FA_ARCH))
	{
		strcpy(extension,"WL3");
		NewEmenu[2].active =
		NewEmenu[4].active =
		EpisodeSelect[1] =
		EpisodeSelect[2] = 1;
	}
	else
#endif
#endif



#ifdef SPEAR
#ifndef SPEARDEMO
	if (!findfirst("*.SOD",&f,FA_ARCH))
	{
		strcpy(extension,"SOD");
	}
	else
		Quit("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
#else
	if (!findfirst("*.SDM",&f,FA_ARCH))
	{
		strcpy(extension,"SDM");
	}
	else
		Quit("NO SPEAR OF DESTINY DEMO DATA FILES TO BE FOUND!");
#endif

#else
	if (!findfirst("*.WL1",&f,FA_ARCH))
	{
		strcpy(extension,"WL1");
	}
	else
		Quit("NO WOLFENSTEIN 3-D DATA FILES to be found!");
#endif

	strcat(configname,extension);
	strcat(SaveName,extension);
	strcat(PageFileName,extension);
	strcat(audioname,extension);
	// *** PRE-V1.4 APOGEE RESTORATION *** - Relocate demoname preparation down for earlier releases
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	strcat(demoname,extension);
#endif
#ifndef SPEAR
#ifndef GOODTIMES
	strcat(helpfilename,extension);
#endif
	strcat(endfilename,extension);
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	strcat(demoname,extension);
#endif
#endif

#endif // S3DNA RESTORATION
#endif // REFKEEN_ENABLE_FILE_SEARCH
}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
