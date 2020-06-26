//
// WL_MENU.H
//
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
#define MenuFadeIn()	VL_FadeIn(0,255,&gamepal,8)

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
#define MenuFadeIn()	VL_FadeIn(0,255,&gamepal,20)
#else
#define MenuFadeIn()	VL_FadeIn(0,255,&gamepal,10)
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
		int x,y,amount,curpos,indent;
		} CP_iteminfo;

typedef struct {
		int active;
		// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		char string[80];
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
		char string[40];
#else
		char string[36];
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		void (* routine)(void);
#else
		void (* routine)(int temp1);
#endif
		} CP_itemtype;

typedef struct {
		int allowed[4];
		} CustomCtrls;

extern CP_itemtype GAMEVER_COND_FARPTR MainMenu[],GAMEVER_COND_FARPTR NewEMenu[];
extern CP_iteminfo MainItems;

//
// FUNCTION PROTOTYPES
//
void SetupControlPanel(void);
void CleanupControlPanel(void);

void DrawMenu(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items);
int  HandleMenu(CP_iteminfo *item_i,
		CP_itemtype GAMEVER_COND_FARPTR *items,
		void (*routine)(int w));
void ClearMScreen(void);
void DrawWindow(int x,int y,int w,int h,int wcolor);
void DrawOutline(int x,int y,int w,int h,int color1,int color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(int count);
void CacheLump(int lumpstart,int lumpend);
void UnCacheLump(int lumpstart,int lumpend);
void StartCPMusic(int song);
int  Confirm(char GAMEVER_COND_FARPTR *string);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void Message(char GAMEVER_COND_FARPTR *string);
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

void DrawGun(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,int x,int *y,int which,int basey,void (*routine)(int w));
void DrawHalfStep(int x,int y);
void EraseGun(CP_iteminfo *item_i,CP_itemtype GAMEVER_COND_FARPTR *items,int x,int y,int which);
void SetTextColor(CP_itemtype GAMEVER_COND_FARPTR *items,int hlight);
void DrawMenuGun(CP_iteminfo *iteminfo);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
void DrawStripes(int y);
#endif

void DefineMouseBtns(void);
void DefineJoyBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(int index,CustomCtrls *cust,void (*DrawRtn)(int),void (*PrintRtn)(int),int type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(int loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(int view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(int which);
void DrawCustMouse(int hilight);
void DrawCustJoy(int hilight);
void DrawCustKeybd(int hilight);
void DrawCustKeys(int hilight);
void PrintCustMouse(int i);
void PrintCustJoy(int i);
void PrintCustKeybd(int i);
void PrintCustKeys(int i);

void PrintLSEntry(int w,int color);
void TrackWhichGame(int w);
void DrawNewGameDiff(int w);
void FixupCustom(int w);

void CP_NewGame(void);
void CP_Sound(void);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void CP_LoadGame(void);
void CP_SaveGame(void);
#else
int  CP_LoadGame(int quick);
int  CP_SaveGame(int quick);
#endif
void CP_Control(void);
void CP_ChangeView(void);
void CP_ExitOptions(void);
void CP_Quit(void);
void CP_ViewScores(void);
int  CP_EndGame(void);
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
int  CP_CheckQuick(unsigned scancode);
#endif
void CustomControls(void);
void MouseSensitivity(void);

void CheckForEpisodes(void);

//
// VARIABLES
//
// *** ALPHA RESTORATION ***
// Commenting these out for the alpha is a part of our brute-force
// way to try and force an additional, unused variable,
// following SaveGamesAvail (may be called, say, pickquick)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
extern int SaveGamesAvail[10],StartGame,SoundStatus;
extern char SaveGameNames[10][32],SaveName[13];
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
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
menuitems
#endif
;

//
// WL_INTER
//
typedef struct {
		int kill,secret,treasure;
		long time;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		long par;
#endif
		} LRstruct;

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
extern LRstruct LevelRatios[30];
#else
extern LRstruct LevelRatios[];
#endif

void Write (int x,int y,char *string);
void NonShareware(void);
int GetYorN(int x,int y,int pic);


