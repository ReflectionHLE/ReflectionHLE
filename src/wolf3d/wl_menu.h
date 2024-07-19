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

//
// WL_MENU.H
//

REFKEEN_NS_B

#ifdef SPEAR

#define BORDCOLOR	0x99
#define BORD2COLOR	0x93
#define DEACTIVE	0x9b
#define BKGDCOLOR	0x9d
//#define STRIPE		0x9c

#define MenuFadeOut()	VL_FadeOut(0,255,0,0,51,10)

// *** S3DNA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)

#define BORDCOLOR	0xd9
#define BORD2COLOR	0xd7
#define DEACTIVE	0xdb
#define BKGDCOLOR	0xdd

#define MenuFadeOut()	VL_FadeOut(0,255,30,23,11,8)

#else

#define BORDCOLOR	0x29
#define BORD2COLOR	0x23
#define DEACTIVE	0x2b
#define BKGDCOLOR	0x2d
#define STRIPE		0x2c

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define MenuFadeOut()	VL_FadeOut(0,255,43,0,0,20)
#else
#define MenuFadeOut()	VL_FadeOut(0,255,43,0,0,10)
#endif

#endif

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D

#define READCOLOR	0x50
#define READHCOLOR	0x0f
#define VIEWCOLOR	0xda
#define TEXTCOLOR	0xcc
#define HIGHLIGHT	0xc5
#define MenuFadeIn()	VL_FadeIn(0,255,gamepal,8)

#else

#define READCOLOR	0x4a
#define READHCOLOR	0x47
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define VIEWCOLOR	0x01
#else
#define VIEWCOLOR	0x7f
#endif
#define TEXTCOLOR	0x17
#define HIGHLIGHT	0x13
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define MenuFadeIn()	VL_FadeIn(0,255,gamepal,20)
#else
#define MenuFadeIn()	VL_FadeIn(0,255,gamepal,10)
#endif

#endif


// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D

#define MENUSONG	NOAH01_MUS
#define INTROSONG	NOAH02_MUS

#else

#define MENUSONG	WONDERIN_MUS

#ifndef SPEAR
#define INTROSONG	NAZI_NOR_MUS
#else
#define INTROSONG	XTOWER2_MUS
#endif

#endif // S3DNA RESTORATION

#define SENSITIVE	60
#define CENTER		SENSITIVE*2

#define MENU_X	76
#define MENU_Y	55
#define MENU_W	178
#ifndef SPEAR
#define MENU_H	13*10+6
#else
#define MENU_H	13*9+6
#endif

#define SM_X	48
#define SM_W	250

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define SM_Y1	28
#else
#define SM_Y1	20
#endif
#define SM_H1	4*13-7
#define SM_Y2	SM_Y1+5*13
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define SM_H2	3*13-7
#define SM_Y3	SM_Y2+4*13
#else
#define SM_H2	4*13-7
#define SM_Y3	SM_Y2+5*13
#endif
#define SM_H3	3*13-7

#define CTL_X	24
#define CTL_Y	70
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define CTL_W	280
#define CTL_H	13*7-20
#else
#define CTL_W	284
#define CTL_H	13*7-7
#endif

#define LSM_X	85
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define LSM_Y	40
#else
#define LSM_Y	55
#endif
#define LSM_W	175
#define LSM_H	10*13+10

#define NM_X	50
#define NM_Y	100
#define NM_W	225
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define NM_H	13*4+2
#else
#define NM_H	13*4+15
#endif

#define NE_X	10
#define NE_Y	23
#define NE_W	320-NE_X*2
#define NE_H	200-NE_Y*2

#define CST_X		20
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define CST_Y		30
#else
#define CST_Y		48
#endif
#define CST_START	60
#define CST_SPC	60

//
// TYPEDEFS
//
typedef struct {
		id0_int_t x,y,amount,curpos,indent;
		} CP_iteminfo;

// REFKEEN: New specifier for return and param type of a CP_item routine.
// Functions are now adapted to use this, changing by the version.
// CP_itemvalue is used to conditionally pass an argument for v1.0+,
// as well as conditionally returning a value.
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define CP_routineargs void
#define CP_routineret void
#define CP_itemvalue
#else
#define CP_routineargs id0_int_t unused
#define CP_routineret id0_int_t
#define CP_itemvalue 0
#endif

typedef struct {
		id0_int_t active;
		// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		id0_char_t string[80];
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
		id0_char_t string[40];
#else
		id0_char_t string[36];
#endif
		CP_routineret (* routine)(CP_routineargs);
#if 0
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		void (* routine)(void);
#else
		void (* routine)(id0_int_t temp1);
#endif
#endif
		} CP_itemtype;

typedef struct {
		id0_int_t allowed[4];
		} CustomCtrls;

extern CP_itemtype GAMEVER_COND_FARPTR MainMenu[],GAMEVER_COND_FARPTR NewEMenu[];
extern CP_iteminfo MainItems;

//
// FUNCTION PROTOTYPES
//
void SetupControlPanel(void);
void CleanupControlPanel(void);

void DrawMenu(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items);
id0_int_t  HandleMenu(CP_iteminfo *item_i,
		CP_itemtype GAMEVER_COND_FARPTR *items,
		void (*routine)(id0_int_t w));
void ClearMScreen(void);
void DrawWindow(id0_int_t x,id0_int_t y,id0_int_t w,id0_int_t h,id0_int_t wcolor);
void DrawOutline(id0_int_t x,id0_int_t y,id0_int_t w,id0_int_t h,id0_int_t color1,id0_int_t color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(id0_int_t count);
void CacheLump(id0_int_t lumpstart,id0_int_t lumpend);
void UnCacheLump(id0_int_t lumpstart,id0_int_t lumpend);
void StartCPMusic(musicnames/*id0_int_t*/ song); // REFKEEN: For C++ build
id0_int_t  Confirm(const id0_char_t GAMEVER_COND_FARPTR *string);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void Message(const id0_char_t GAMEVER_COND_FARPTR *string);
#endif
void CheckPause(void);
// *** ALPHA RESTORATION ***
// Apparently this was originally duplicated (if not defined as a macro)
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define ShootSnd() SD_PlaySound(SHOOTSND)
#else
void ShootSnd(void);
#endif
void CheckSecretMissions(void);
void BossKey(void);

void DrawGun(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,id0_int_t x,id0_int_t *y,id0_int_t which,id0_int_t basey,void (*routine)(id0_int_t w));
void DrawHalfStep(id0_int_t x,id0_int_t y);
void EraseGun(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,id0_int_t x,id0_int_t y,id0_int_t which);
void SetTextColor(CP_itemtype GAMEVER_COND_FARPTR *items,id0_int_t hlight);
void DrawMenuGun(CP_iteminfo *iteminfo);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void DrawStripes(id0_int_t y);
#endif

void DefineMouseBtns(void);
void DefineJoyBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(id0_int_t index,CustomCtrls *cust,void (*DrawRtn)(id0_int_t),void (*PrintRtn)(id0_int_t),id0_int_t type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(id0_int_t loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(id0_int_t view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(id0_int_t which);
void DrawCustMouse(id0_int_t hilight);
void DrawCustJoy(id0_int_t hilight);
void DrawCustKeybd(id0_int_t hilight);
void DrawCustKeys(id0_int_t hilight);
void PrintCustMouse(id0_int_t i);
void PrintCustJoy(id0_int_t i);
void PrintCustKeybd(id0_int_t i);
void PrintCustKeys(id0_int_t i);

void PrintLSEntry(id0_int_t w,id0_int_t color);
void TrackWhichGame(id0_int_t w);
void DrawNewGameDiff(id0_int_t w);
void FixupCustom(id0_int_t w);

CP_routineret CP_NewGame(CP_routineargs);
CP_routineret CP_Sound(CP_routineargs);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void CP_LoadGame(void);
void CP_SaveGame(void);
#else
id0_int_t  CP_LoadGame(id0_int_t quick);
id0_int_t  CP_SaveGame(id0_int_t quick);
#endif
CP_routineret CP_Control(CP_routineargs);
CP_routineret CP_ChangeView(CP_routineargs);
CP_routineret CP_ExitOptions(CP_routineargs);
void CP_Quit(void);
CP_routineret CP_ViewScores(CP_routineargs);
id0_int_t  CP_EndGame(void);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_int_t  CP_CheckQuick(id0_unsigned_t scancode);
#endif
CP_routineret CustomControls(CP_routineargs);
CP_routineret MouseSensitivity(CP_routineargs);

void CheckForEpisodes(void);

//
// VARIABLES
//
// *** ALPHA RESTORATION ***
// Commenting these out for the alpha is a part of our brute-force
// way to try and force an additional, unused variable,
// following SaveGamesAvail (may be called, say, pickquick)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
extern id0_int_t SaveGamesAvail[10],StartGame,SoundStatus;
extern id0_char_t SaveGameNames[10][32],SaveName[13];
#endif

enum {MOUSE,JOYSTICK,KEYBOARDBTNS,KEYBOARDMOVE};	// FOR INPUT TYPES

enum
{
	newgame,
	soundmenu,
	control,
	loadgame,
	savegame,
	changeview,

#ifndef GOODTIMES
#ifndef SPEAR
	readthis,
#endif
#endif

	viewscores,
	backtodemo,
	quit
}
// *** PRE-V1.4 APOGEE RESTORATION ***
// REFKEEN: Disable; This var is unused and shouldn't be repeatedly defined.
#if 0
//#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
menuitems
#endif
;

//
// WL_INTER
//
typedef struct {
		id0_int_t kill,secret,treasure;
		id0_long_t time;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		id0_long_t par;
#endif
		} LRstruct;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern LRstruct LevelRatios[30];
#else
extern LRstruct LevelRatios[];
#endif

void Write (id0_int_t x,id0_int_t y,const id0_char_t *string);
void NonShareware(void);
id0_int_t GetYorN(id0_int_t x,id0_int_t y,id0_int_t pic);

REFKEEN_NS_E
