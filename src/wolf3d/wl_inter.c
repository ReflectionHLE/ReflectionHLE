// WL_INTER.C

#include "wl_def.h"
#pragma hdrstop


//==========================================================================

/*
==================
=
= CLearSplitVWB
=
==================
*/

void ClearSplitVWB (void)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (++screenpage == 3)
		screenpage = 0;
	bufferofs = screenloc[screenpage];
#endif
	memset (update,0,sizeof(update));
	WindowX = 0;
	WindowY = 0;
	WindowW = 320;
	WindowH = 160;
}


//==========================================================================

#ifdef SPEAR
#ifndef SPEARDEMO
////////////////////////////////////////////////////////
//
// End of Spear of Destiny
//
////////////////////////////////////////////////////////

void EndScreen (id0_int_t palette, id0_int_t screen)
{
	CA_CacheScreen (screen);
	VW_UpdateScreen ();
	CA_CacheGrChunk (palette);
	VL_FadeIn(0,255,grsegs[palette],30);
	UNCACHEGRCHUNK (palette);
	IN_ClearKeysDown ();
	IN_Ack ();
	VW_FadeOut ();
}


void EndSpear(void)
{
	EndScreen (END1PALETTE, ENDSCREEN11PIC);

	CA_CacheScreen (ENDSCREEN3PIC);
	VW_UpdateScreen ();
	CA_CacheGrChunk (END3PALETTE);
	VL_FadeIn(0,255,grsegs[END3PALETTE],30);
	UNCACHEGRCHUNK (END3PALETTE);
	fontnumber = 0;
	fontcolor = 0xd0;
	WindowX = 0;
	WindowW = 320;
	PrintX = 0;
	PrintY = 180;
	US_CPrint (STR_ENDGAME1"\n");
	US_CPrint (STR_ENDGAME2);
	VW_UpdateScreen ();
	IN_StartAck ();
	TimeCount = 0;
	while (!IN_CheckAck () && TimeCount < 700);

	PrintX = 0;
	PrintY = 180;
	VWB_Bar(0,180,320,20,0);
	US_CPrint (STR_ENDGAME3"\n");
	US_CPrint (STR_ENDGAME4);
	VW_UpdateScreen ();
	IN_StartAck ();
	TimeCount = 0;
	while (!IN_CheckAck () && TimeCount < 700);

	VW_FadeOut ();

	EndScreen (END4PALETTE, ENDSCREEN4PIC);
	EndScreen (END5PALETTE, ENDSCREEN5PIC);
	EndScreen (END6PALETTE, ENDSCREEN6PIC);
	EndScreen (END7PALETTE, ENDSCREEN7PIC);
	EndScreen (END8PALETTE, ENDSCREEN8PIC);
	EndScreen (END9PALETTE, ENDSCREEN9PIC);

	EndScreen (END2PALETTE, ENDSCREEN12PIC);

	MainMenu[savegame].active = 0;
}
#endif
#endif

// *** S3DNA RESTORATION ***
// This appears to contains variants of code from places like ThreeDRefresh
// (ASM code, frame counting) and PlayLoop (screen fading and user input checks)
#ifdef GAMEVER_NOAH3D

statetype *caststate[] = {&s_dogchase1,&s_grdchase1,&s_ofcchase1,&s_sschase1,
	&s_mutchase1,&s_bosschase1,&s_schabbchase1,&s_gretelchase1,
	&s_giftchase1,&s_fatchase1,&s_mechachase1,&s_hitlerchase1};

id0_char_t id0_far *casttext[] = {"Goat","Sheep","Ostrich","Antelope",
	"Ox","Carl the Camel","Melvin the Monkey","Ginny the Giraffe",
	"Kerry the Kangaroo","Ernie the Elephant","Hiding Burt","Burt the Bear"};

id0_int_t castsound[] = {D_GOATSND,D_SHEEPSND,D_OSTRCHSND,D_ANTLPESND,
	D_OXSND,D_CAMELSND,D_MONKEYSND,D_GIRAFESND,
	D_KANGROSND,D_ELPHNTSND,D_BEARSND,D_BEARSND};

void CharacterCast(void)
{
	id0_int_t i, en, count, cycle;
	id0_unsigned_t temp,buttons;
	statetype *state;

	ClearMemory ();
	StartCPMusic (NOAH02_MUS);
	gamestate.mapon = 30;
	SetupGameLevel();
	CA_CacheGrChunk (STARTFONT+1);
	SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
	PM_CheckMainMem ();
	PreloadGraphics ();

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
		VWB_Bar (0,160,320,40,VIEWCOLOR);
	}
	bufferofs = temp;

	en = 0;
	anglefrac = 0;
	WindowX = 0;
	WindowW = 320-16;
	fontnumber = 1;
	lasttimecount = TimeCount = 0;
	IN_StartAck ();

	do
	{
		state = caststate[en];
		count = state->tictime;
		cycle = 0;
nexttic:
		if (cycle == 0)
			SD_PlaySound (castsound[en]);
		CalcTics();
		cycle += 2*tics;
		if (cycle > 224)
		{
			if (++en
			    >= 12)
				en = 0;
			continue;
		}

		if (!(--count))
		{
			state = state->next;
			count = state->tictime;
		}

		// S3DNA RESTORATION - Taken off ThreeDRefresh;
		// If memset is used, a bit different code is generated.
	asm	mov	ax,ds
	asm	mov	es,ax
	asm	mov	di,OFFSET spotvis
	asm	xor	ax,ax
	asm	mov	cx,2048
	asm	rep stosw

		bufferofs += screenofs;
		PollControls ();
		WallRefresh ();
		if (!nofloors)
			DrawPlanes ();
		DrawScaleds ();

		SimpleScaleShape (viewwidth/2,state->shapenum,cycle/2+24);
		VWB_Bar (0,160,320,40,VIEWCOLOR);
		PrintY = 168;
		US_CPrintLine (casttext[en]);

		bufferofs -= screenofs;
		displayofs = bufferofs;

	asm	cli
	asm	mov	cx,[displayofs]
	asm	mov	dx,CRTC_INDEX
	asm	mov	al,CRTC_STARTHIGH
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al
	asm	sti

		bufferofs += SCREENSIZE;
		if (bufferofs > PAGE3START)
			bufferofs = 0;
		frameon++;
		PM_NextFrame ();
		SD_Poll ();
		if (screenfaded)
			VW_FadeIn();

		buttons = IN_JoyButtons()*16;
		if (MousePresent)
			buttons |= IN_MouseButtons();
		if ((cycle >= 16) && (Keyboard[sc_Space] || (buttons & 0x11)))
		{
			if (++en >= 12)
				en = 0;
			continue;
		}
		if (Keyboard[sc_Escape] || Keyboard[sc_Q] || Keyboard[sc_Return]
		    || (buttons & 0x22))
			break;
		goto nexttic;
	} while (en < 12);

	UNCACHEGRCHUNK (STARTFONT+1);
	StopMusic ();
	fontnumber = 0;
	VW_FadeOut ();
}
#endif

//==========================================================================

/*
==================
=
= Victory
=
==================
*/

void Victory (void)
{
#ifndef SPEARDEMO
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	id0_int_t	sec;
#elif (defined GAMEVER_NOAH3D)
	id0_long_t	sec,parsec;
#else
	id0_long_t	sec;
#endif
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_int_t i,hr,min,kr,sr,tr,x;
#else
	id0_int_t i,min,kr,sr,tr,x;
#endif
	id0_char_t tempstr[8];

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define RATIOX	9
#define RATIOY	12
#define TIMEX	23
#define TIMEY	9
#define PARTIMEX	23
#define PARTIMEY	7
#else
#define RATIOX	6
#define RATIOY	14
#define TIMEX	14
#define TIMEY	8
#endif


#ifdef SPEAR
	StartCPMusic (XTHEEND_MUS);

	CA_CacheGrChunk(BJCOLLAPSE1PIC);
	CA_CacheGrChunk(BJCOLLAPSE2PIC);
	CA_CacheGrChunk(BJCOLLAPSE3PIC);
	CA_CacheGrChunk(BJCOLLAPSE4PIC);

	VWB_Bar(0,0,320,200,VIEWCOLOR);
	VWB_DrawPic (124,44,BJCOLLAPSE1PIC);
	VW_UpdateScreen ();
	VW_FadeIn ();
	VW_WaitVBL(2*70);
	VWB_DrawPic (124,44,BJCOLLAPSE2PIC);
	VW_UpdateScreen ();
	VW_WaitVBL(105);
	VWB_DrawPic (124,44,BJCOLLAPSE3PIC);
	VW_UpdateScreen ();
	VW_WaitVBL(105);
	VWB_DrawPic (124,44,BJCOLLAPSE4PIC);
	VW_UpdateScreen ();
	VW_WaitVBL(3*70);

	UNCACHEGRCHUNK(BJCOLLAPSE1PIC);
	UNCACHEGRCHUNK(BJCOLLAPSE2PIC);
	UNCACHEGRCHUNK(BJCOLLAPSE3PIC);
	UNCACHEGRCHUNK(BJCOLLAPSE4PIC);
	VL_FadeOut (0,255,0,17,17,5);
#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	StartCPMusic (NOAH07_MUS);
#else
	StartCPMusic (URAHERO_MUS);
#endif
	ClearSplitVWB ();
	CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	CA_CacheGrChunk(STARTFONT);

#ifndef SPEAR
	CA_CacheGrChunk(C_TIMECODEPIC);
#endif
#endif


	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Bar (0,0,320,200-STATUSLINES,218);
#else
	VWB_Bar (0,0,320,200-STATUSLINES,127);
#endif
#ifdef JAPAN
#ifndef JAPDEMO
	CA_CacheGrChunk(C_ENDRATIOSPIC);
	VWB_DrawPic(0,0,C_ENDRATIOSPIC);
	UNCACHEGRCHUNK(C_ENDRATIOSPIC);
#endif
	// ***SHAREWARE/REGISTERED V1.4 APOGEE RESTORATION ***
	// Possibly relocated this, depending on version
#if (GAMEVER_WOLFREV <= GV_WR_SODFG14A)
	VWB_DrawPic (8,4,L_BJWINSPIC);
#endif
	// *** S3DNA RESTORATION ***
#elif (!defined GAMEVER_NOAH3D)
//#else
	// ***SHAREWARE/REGISTERED V1.4 APOGEE RESTORATION ***
	// Possibly relocated this, depending on version
#if (GAMEVER_WOLFREV <= GV_WR_SODFG14A)
	VWB_DrawPic (8,4,L_BJWINSPIC);
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	Write(18,0,STR_YOUWIN);
#else
	Write(18,2,STR_YOUWIN);
#endif

	// *** S3DNA RESTORATION ***
	// (TIMEX and TIMEY redefined with different values now)
#ifdef GAMEVER_NOAH3D
	Write(14,8-2,STR_TOTALTIME);
#else
	Write(TIMEX,TIMEY-2,STR_TOTALTIME);
#endif

	Write(12,RATIOY-2,"averages");

	#ifdef SPANISH
	Write(RATIOX+2,  RATIOY,      STR_RATKILL);
	Write(RATIOX+2,  RATIOY+2,  STR_RATSECRET);
	Write(RATIOX+2,  RATIOY+4,STR_RATTREASURE);
	#else
	Write(RATIOX+8,RATIOY,      STR_RATKILL);
	Write(RATIOX+4,RATIOY+2,  STR_RATSECRET);
	Write(RATIOX,  RATIOY+4,STR_RATTREASURE);
	#endif

#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_DrawPic (8,8,L_BJWINSPIC);

	VWB_DrawPic (104,32,W_RATINGPIC);
	VWB_DrawPic (120,56,W_PARPIC);
	VWB_DrawPic (112,72,W_TIMEPIC);

	VWB_DrawPic (104,96,W_KILLSPIC);
	Write(33,12,"%");

	VWB_DrawPic (128,112,W_TREASURESPIC);
	Write(33,14,"%");

	VWB_DrawPic (96,128,W_SECRETSPIC);
	Write(33,16,"%");
#else
	// ***SHAREWARE/REGISTERED V1.4 APOGEE RESTORATION ***
	// Possibly relocate this, depending on version
#if (GAMEVER_WOLFREV > GV_WR_SODFG14A)
#ifndef JAPDEMO
	VWB_DrawPic (8,4,L_BJWINSPIC);
#endif
#endif
#endif // GAMEVER_NOAH3D


	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	for (kr = sr = tr = sec = parsec = i = 0;i < 30;i++)
#elif (!defined SPEAR)
//#ifndef SPEAR
	for (kr = sr = tr = sec = i = 0;i < 8;i++)
#else
	for (kr = sr = tr = sec = i = 0;i < 20;i++)
#endif
	{
		sec += LevelRatios[i].time;
		kr += LevelRatios[i].kill;
		sr += LevelRatios[i].secret;
		tr += LevelRatios[i].treasure;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		parsec += LevelRatios[i].par;
#endif
	}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	min = parsec/60;
	parsec %= 60;
	hr = min/60;
	min %= 60;
	if (hr > 9)
	{
		hr = 9;
		min = parsec = 99;
	}
	i = PARTIMEX*8;
	VWB_DrawPic (i,PARTIMEY*8,L_NUM0PIC+hr);
	i += 2*8;
	Write (i/8,PARTIMEY,":");
	i += 1*8;
	VWB_DrawPic (i,PARTIMEY*8,L_NUM0PIC+(min/10));
	i += 2*8;
	VWB_DrawPic (i,PARTIMEY*8,L_NUM0PIC+(min%10));
	i += 2*8;
	Write (i/8,PARTIMEY,":");
	i += 1*8;
	VWB_DrawPic (i,PARTIMEY*8,L_NUM0PIC+(parsec/10));
	i += 2*8;
	VWB_DrawPic (i,PARTIMEY*8,L_NUM0PIC+(parsec%10));
	VW_UpdateScreen ();
#else

#ifndef SPEAR
	kr /= 8;
	sr /= 8;
	tr /= 8;
#else
	kr /= 14;
	sr /= 14;
	tr /= 14;
#endif
	// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
	if (sec > 415800)
		sec = 415800;
#endif

#endif // GAMEVER_NOAH3D

	min = sec/60;
	sec %= 60;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	hr = min/60;
	min %= 60;
	if (hr > 9)
	{
		hr = 9;
		min = sec = 99;
	}
	// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#elif (GAMEVER_WOLFREV > GV_WR_WL1AP11)
	if (min > 99)
		min = sec = 99;
#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	i = TIMEX*8;
	VWB_DrawPic(i,TIMEY*8,L_NUM0PIC+hr);
	i += 2*8;
	Write(i/8,TIMEY,":");
	i += 1*8;
#else
	i = TIMEX*8+1;
#endif
	VWB_DrawPic(i,TIMEY*8,L_NUM0PIC+(min/10));
	i += 2*8;
	VWB_DrawPic(i,TIMEY*8,L_NUM0PIC+(min%10));
	i += 2*8;
	Write(i/8,TIMEY,":");
	i += 1*8;
	VWB_DrawPic(i,TIMEY*8,L_NUM0PIC+(sec/10));
	i += 2*8;
	VWB_DrawPic(i,TIMEY*8,L_NUM0PIC+(sec%10));
	VW_UpdateScreen ();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	kr /= 30;
	sr /= 30;
	tr /= 30;
#endif

	itoa(kr,tempstr,10);
	x=RATIOX+24-strlen(tempstr)*2;
	Write(x,RATIOY,tempstr);

	itoa(sr,tempstr,10);
	x=RATIOX+24-strlen(tempstr)*2;
	Write(x,RATIOY+2,tempstr);

	itoa(tr,tempstr,10);
	x=RATIOX+24-strlen(tempstr)*2;
	Write(x,RATIOY+4,tempstr);


#ifndef SPANISH
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (!defined UPLOAD) || (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
//#ifndef UPLOAD
	// *** S3DNA RESTORATION ***
#if (!defined SPEAR) && (!defined GAMEVER_NOAH3D)
//#ifndef SPEAR
	//
	// TOTAL TIME VERIFICATION CODE
	//

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	if (gamestate.difficulty>=gd_medium)
#endif
	{
		VWB_DrawPic (30*8,TIMEY*8,C_TIMECODEPIC);
		fontnumber = 0;
		fontcolor = READHCOLOR;
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PrintX = 30*8;
#else
		PrintX = 30*8-3;
#endif
		PrintY = TIMEY*8+8;
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		VWB_Bar (PrintX,PrintY,24,10,0x29);
#endif
		PrintX+=4;
		tempstr[0] = (((min/10)^(min%10))^0xa)+'A';
		tempstr[1] = (((sec/10)^(sec%10))^0xa)+'A';
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		tempstr[2] = 0;
#else
		tempstr[2] = (tempstr[0]^tempstr[1])+'A';
		tempstr[3] = 0;
#endif
		US_Print(tempstr);
	}
#endif
#endif
#endif


	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	fontnumber = 1;
#endif

	VW_UpdateScreen ();
	VW_FadeIn ();

	IN_Ack();

	// *** SHAREWARE/REGISTERED APOGEE RESTORATION ***
	// This is also skipped in the Apogee EXEs
	#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A)
	//#ifndef SPEAR
	if (Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		PicturePause();
	#endif

	VW_FadeOut ();

	// *** S3DNA RESTORATION ***
#if (!defined SPEAR) && (!defined GAMEVER_NOAH3D)
//#ifndef SPEAR
	UNCACHEGRCHUNK(C_TIMECODEPIC);
#endif
	UnCacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	CharacterCast();
#elif (!defined SPEAR)
//#ifndef SPEAR
	EndText();
#else
	EndSpear();
#endif

#endif // SPEARDEMO
}


//==========================================================================

// *** S3DNA RESTORATION ***
#if (!defined JAPAN) && (!defined GAMEVER_NOAH3D)
//#ifndef JAPAN
/*
==================
=
= PG13
=
==================
*/

void PG13 (void)
{
	VW_FadeOut();
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	VWB_Bar(0,0,319,200,0x82);			// background
#else
	VWB_Bar(0,0,320,200,0x82);			// background
#endif

	CA_CacheGrChunk (PG13PIC);
	VWB_DrawPic (216,110,PG13PIC);
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	VW_UpdateScreen ();
#endif

	UNCACHEGRCHUNK (PG13PIC);

	VW_FadeIn();
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	IN_Ack();
#else
	IN_UserInput(TickBase*7);
#endif

	VW_FadeOut ();
}
#endif


//==========================================================================

void Write(id0_int_t x,id0_int_t y,id0_char_t *string)
{
 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 id0_int_t alpha[]={L_NUM0PIC,L_NUM1PIC,L_NUM2PIC,L_NUM3PIC,L_NUM4PIC,L_NUM5PIC,
	L_NUM6PIC,L_NUM7PIC,L_NUM8PIC,L_NUM9PIC,L_COLONPIC,0,0,0,0,0};
#else
 id0_int_t alpha[]={L_NUM0PIC,L_NUM1PIC,L_NUM2PIC,L_NUM3PIC,L_NUM4PIC,L_NUM5PIC,
	L_NUM6PIC,L_NUM7PIC,L_NUM8PIC,L_NUM9PIC,L_COLONPIC,0,0,0,0,0,0,L_APIC,L_BPIC,
	L_CPIC,L_DPIC,L_EPIC,L_FPIC,L_GPIC,L_HPIC,L_IPIC,L_JPIC,L_KPIC,
	L_LPIC,L_MPIC,L_NPIC,L_OPIC,L_PPIC,L_QPIC,L_RPIC,L_SPIC,L_TPIC,
	L_UPIC,L_VPIC,L_WPIC,L_XPIC,L_YPIC,L_ZPIC};
#endif

 id0_int_t i,ox,nx,ny;
 id0_char_t ch;


 ox=nx=x*8;
 ny=y*8;
 for (i=0;i<strlen(string);i++)
   if (string[i]=='\n')
   {
	nx=ox;
	ny+=16;
   }
   else
   {
	ch=string[i];
	if (ch>='a')
	  ch-=('a'-'A');
	ch-='0';

	switch(string[i])
	{
	 case '!':
	   VWB_DrawPic(nx,ny,L_EXPOINTPIC);
	   nx+=8;
	   continue;

	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	 case '\'':
	   VWB_DrawPic(nx,ny,L_APOSTROPHEPIC);
	   nx+=8;
	   continue;
#endif

	 case ' ': break;
	 case 0x3a:	// ':'

	   VWB_DrawPic(nx,ny,L_COLONPIC);
	   nx+=8;
	   continue;

	 case '%':
	   VWB_DrawPic(nx,ny,L_PERCENTPIC);
	   break;

	 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	 case '-':
	   VWB_DrawPic(nx,ny,L_MINUSPIC);
	   nx+=8;
	   continue;

	 default:
	  if (ch >= 16)
	  {
		sprintf(str,"Write(): Invalid character $%02X!",ch);
		Quit(str);
	  }
	   VWB_DrawPic(nx,ny,alpha[ch]);
#else
	 default:
	   VWB_DrawPic(nx,ny,alpha[ch]);
#endif
	}
	nx+=16;
   }
}


//
// Breathe Mr. BJ!!!
//
void BJ_Breathe(void)
{
	static id0_int_t which=0,max=10;
	id0_int_t pics[2]={L_GUYPIC,L_GUY2PIC};


	if (TimeCount>max)
	{
		which^=1;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		VWB_DrawPic(8,8,pics[which]);
#else
		VWB_DrawPic(0,16,pics[which]);
#endif
		VW_UpdateScreen();
		TimeCount=0;
		max=35;
	}
}



// *** S3DNA RESTORATION ***
// Additional functions for LevelCompleted
#ifdef GAMEVER_NOAH3D
void ShowBonus (id0_long_t bonus)
{
	id0_char_t tempstr[10];
	id0_char_t	ch;
	id0_int_t	x,i;

	ltoa(bonus,tempstr,10);
	x=8*(36-strlen(tempstr)*2);

	for (i=0;i<strlen(tempstr);i++)
	{
		ch = tempstr[i];
		if ((ch >= '0') && (ch <= '9'))
		{
			VWB_DrawPic(x,72,ch+(Y_NUM0PIC-'0'));
			x += 16;
		}
	}
}

void DrawEndLevelScreen (id0_int_t secretlvl)
{
	id0_char_t tempstr[10];

	VWB_Bar (0,0,320,160,VIEWCOLOR);
	VWB_DrawPic (8,8,L_GUYPIC);
	if (secretlvl)
	{
		VWB_DrawPic (104,8,W_SECRETPIC);
		VWB_DrawPic (104,24,W_LEVELPIC);
		VWB_DrawPic (104,40,W_FINISHPIC);
		VWB_DrawPic (184,24,secretlvl+L_NUM0PIC);
	}
	else
	{
		VWB_DrawPic (104,8,W_LEVELPIC);
		VWB_DrawPic (104,24,W_FLOORPIC);
		VWB_DrawPic (104,40,W_FINISHPIC);
		sprintf (tempstr,"%d",MapEpisode[gamestate.mapon]);
		Write (23,1,tempstr);
		sprintf (tempstr,"%d",MapLevel[gamestate.mapon]);
		Write (23,3,tempstr);
	}
	VWB_DrawPic (104,72,W_BONUSPIC);
}

void RollDelay (void)
{
	id0_unsigned_long_t lasttime = TimeCount;
	while (TimeCount == lasttime)
		;
}
#endif


/*
==================
=
= LevelCompleted
=
= Entered with the screen faded out
= Still in split screen mode with the status bar
=
= Exit with the screen faded out
=
==================
*/

 // *** S3DNA  RESTORATION ***
#ifdef GAMEVER_NOAH3D
LRstruct LevelRatios[30];
#elif (!defined SPEAR)
//#ifndef SPEAR
LRstruct LevelRatios[8];
#else
LRstruct LevelRatios[20];
#endif

void LevelCompleted (void)
{
	#define VBLWAIT	30
	#define PAR_AMOUNT	500
	#define PERCENT100AMT	10000
	typedef struct {
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			id0_word_t time;
#else
			float time;
#endif
			id0_char_t timestr[6];
			} times;

	// *** S3DNA  RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_int_t	x,i,min,sec,secret,ratio;
	id0_boolean_t perfect;
	id0_long_t bonus,timeleft;
	id0_int_t	kr,sr,tr;
#else
	id0_int_t	x,i,min,sec,ratio,kr,sr,tr;
#endif
	id0_unsigned_t	temp;
	id0_char_t tempstr[10];
	// *** S3DNA  RESTORATION ***
#ifdef GAMEVER_NOAH3D
	id0_unsigned_long_t	lasttime;
#else
	id0_long_t bonus,timeleft=0;
#endif
	times parTimes[]=
	{
	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	 //
	 // Episode One Par Times
	 //
	 {6300,  "01:30"},
	 {8400,  "02:00"},
	 {8400,  "02:00"},
	 {14700, "03:30"},
	 {12600, "03:00"},
	 {12600, "03:00"},
	 {10500, "02:30"},
	 {10500, "02:30"},
	 {0,     "??:??"},	// Boss level
	 {12600, "03:00"},	// Secret level

	 //
	 // Episode Two Par Times
	 //
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {0,     "??:??"},
	 {2100,  "00:30"},

	 //
	 // Episode Three Par Times
	 //
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {0,     "??:??"},
	 {2100,  "00:30"},

	 //
	 // Episode Four Par Times
	 //
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {0,     "??:??"},
	 {2100,  "00:30"},

	 //
	 // Episode Five Par Times
	 //
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {0,     "??:??"},
	 {2100,  "00:30"},

	 //
	 // Episode Six Par Times
	 //
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {2100,  "00:30"},
	 {0,     "??:??"},
	 {2100,  "00:30"}
	 // *** S3DNA  RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
	 //
	 // Episode One Par Times
	 //
	 {0.5,  "00:30"},
	 {1,	"01:00"},
	 {1,	"01:00"},

	 //
	 // Episode Two Par Times
	 //
	 {2,	"02:00"},
	 {1.75,	"01:45"},
	 {1.5,  "01:30"},
	 {1.75,	"01:45"},

	 //
	 // Episode Three Par Times
	 //
	 {2.75,	"01:45"}, // S3DNA RESTORATION - Looks like a typo
	 {2,	"02:00"},
	 {2.25,	"02:15"},
	 {1,	"01:00"},
	 {2,	"02:00"},

	 //
	 // Episode Four Par Times
	 //
	 {3.5,	"03:30"},
	 {2.5,	"02:30"},
	 {4,	"04:00"},
	 {2.25,	"02:15"},
	 {3.5,	"03:30"},

	 //
	 // Episode Five Par Times
	 //
	 {2.5,	"02:30"},
	 {2.75,	"02:45"},
	 {3,	"03:00"},
	 {3.5,	"03:30"},
	 {3,	"03:00"},
	 {3,	"03:00"},

	 //
	 // Episode Six Par Times
	 //
	 {1.75,	"01:45"},
	 {3,	"03:00"},
	 {3,	"03:00"},
	 {1.5,  "01:30"},
	 {2.5,	"02:30"},
	 {3.1900001,	"03:11"},
	 {2.5,	"02:00"} // S3DNA RESTORATION - Another typo
#elif (!defined SPEAR)
//#ifndef SPEAR
	 //
	 // Episode One Par Times
	 //
	 {1.5,	"01:30"},
	 {2,	"02:00"},
	 {2,	"02:00"},
	 {3.5,	"03:30"},
	 {3,	"03:00"},
	 {3,	"03:00"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {0,	"??:??"},	// Boss level
	 {0,	"??:??"},	// Secret level

	 //
	 // Episode Two Par Times
	 //
	 {1.5,	"01:30"},
	 {3.5,	"03:30"},
	 {3,	"03:00"},
	 {2,	"02:00"},
	 {4,	"04:00"},
	 {6,	"06:00"},
	 {1,	"01:00"},
	 {3,	"03:00"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Three Par Times
	 //
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {3.5,	"03:30"},
	 {2.5,	"02:30"},
	 {2,	"02:00"},
	 {6,	"06:00"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	// *** SHAREWARE V1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
	 //
	 // Episode Four Par Times
	 //
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Five Par Times
	 //
	 {2.5,	"02:30"},
	 {1.5,	"01:30"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Six Par Times
	 //
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {0,	"??:??"},
	 {0,	"??:??"}
#else
	 //
	 // Episode Four Par Times
	 //
	 {2,	"02:00"},
	 {2,	"02:00"},
	 {1.5,	"01:30"},
	 {1,	"01:00"},
	 {4.5,	"04:30"},
	 {3.5,	"03:30"},
	 {2,	"02:00"},
	 {4.5,	"04:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Five Par Times
	 //
	 {2.5,	"02:30"},
	 {1.5,	"01:30"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {4,	"04:00"},
	 {3,	"03:00"},
	 {4.5,	"04:30"},
	 {3.5,	"03:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Six Par Times
	 //
	 {6.5,	"06:30"},
	 {4,	"04:00"},
	 {4.5,	"04:30"},
	 {6,	"06:00"},
	 {5,	"05:00"},
	 {5.5,	"05:30"},
	 {5.5,	"05:30"},
	 {8.5,	"08:30"},
	 {0,	"??:??"},
	 {0,	"??:??"}
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP11
#else
	 //
	 // SPEAR OF DESTINY TIMES
	 //
	 {1.5,	"01:30"},
	 {3.5,	"03:30"},
	 {2.75,	"02:45"},
	 {3.5,	"03:30"},
	 {0,	"??:??"},	// Boss 1
	 {4.5,	"04:30"},
	 {3.25,	"03:15"},
	 {2.75,	"02:45"},
	 {4.75,	"04:45"},
	 {0,	"??:??"},	// Boss 2
	 {6.5,	"06:30"},
	 {4.5,	"04:30"},
	 {2.75,	"02:45"},
	 {4.5,	"04:30"},
	 {6,	"06:00"},
	 {0,	"??:??"},	// Boss 3
	 {6,	"06:00"},
	 {0,	"??:??"},	// Boss 4
	 {0,	"??:??"},	// Secret level 1
	 {0,	"??:??"},	// Secret level 2
#endif
	};



	// *** S3DNA  RESTORATION ***
#ifdef GAMEVER_NOAH3D
	StartCPMusic(NOAH05_MUS);
	CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	ClearSplitVWB ();			// set up for double buffering in split screen
	ClearMemory();
	CA_LoadAllSounds();
	PM_CheckMainMem ();

	switch (gamestate.mapon)
	{
		case 11: secret = 1;
			break;
		case 29: secret = 2;
			break;
		default: secret = 0;
	}
	DrawEndLevelScreen (secret);
	bonus = 0;
	ShowBonus (0);
	IN_ClearKeysDown ();
	IN_StartAck ();

	kr = sr = tr
		= 0;
	if (gamestate.killtotal)
		kr=(gamestate.killcount*100)/gamestate.killtotal;
	if (gamestate.secrettotal)
		sr=(gamestate.secretcount*100)/gamestate.secrettotal;
	if (gamestate.treasuretotal)
		tr=(gamestate.treasurecount*100)/gamestate.treasuretotal;

	if (secret)
	{
		VW_UpdateScreen();
		VW_FadeIn();
		VL_WaitVBL(VBLWAIT);
		SD_PlaySound(PERCENT100SND);
		bonus = 15000;
		ShowBonus(15000);
		TimeCount = 0;
		IN_StartAck();
		lasttime = TimeCount;
		do
		{
			BJ_Breathe();
			VW_UpdateScreen();
			if (IN_CheckAck())
				break;
		} while (TimeCount - lasttime < 2*TickBase);

		if (Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			PicturePause();

		while (SD_SoundPlaying())
			BJ_Breathe();
	}
	VWB_DrawPic(96,112,W_PARPIC);
	Write(19,14,parTimes[gamestate.mapon].timestr);
	VWB_DrawPic(88,128,W_TIMEPIC);

	//
	// PRINT TIME
	//
	sec=gamestate.TimeCount/70;

	if (sec > 99*60)		// 99 minutes max
		sec = 99*60;

	if (gamestate.TimeCount<parTimes[gamestate.mapon].time*4200)
		timeleft=parTimes[gamestate.mapon].time*4200/70-sec;
	else
		timeleft=0;

	min=sec/60;
	sec%=60;

	i=19*8;
	VWB_DrawPic(i,16*8,L_NUM0PIC+(min/10));
	i+=2*8;
	VWB_DrawPic(i,16*8,L_NUM0PIC+(min%10));
	i+=2*8;
	Write(i/8,16,":");
	i+=1*8;
	VWB_DrawPic(i,16*8,L_NUM0PIC+(sec/10));
	i+=2*8;
	VWB_DrawPic(i,16*8,L_NUM0PIC+(sec%10));

	VW_UpdateScreen ();
	VW_FadeIn ();

	TimeCount=0;
	//
	// PRINT TIME BONUS
	//
	if (timeleft)
	{
		for (i=0;i<=timeleft;i++)
		{
			ShowBonus(bonus+(id0_long_t)i*PAR_AMOUNT);
			SD_PlaySound(D_INCSND);
			VW_UpdateScreen();
			BJ_Breathe();
			if (IN_CheckAck())
				break;
			RollDelay();
		}
		VW_UpdateScreen();
		SD_PlaySound(D_INCSND);
		while (SD_SoundPlaying())
			BJ_Breathe();
	}
	else
		SD_PlaySound(NOBONUSSND);

	bonus+=timeleft*PAR_AMOUNT;
	IN_StartAck();
	lasttime = TimeCount;
	do
	{
		BJ_Breathe();
		VW_UpdateScreen();
		if (IN_CheckAck())
			break;
	} while (TimeCount - lasttime < 2*TickBase);

	if (Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		PicturePause();

	DrawEndLevelScreen(secret);
	ShowBonus(bonus);
	VWB_DrawPic(80,104,W_KILLSPIC);
	Write(27,13,"0%");
	VWB_DrawPic(104,120,W_TREASURESPIC);
	Write(27,15,"0%");
	VWB_DrawPic(72,136,W_SECRETSPIC);
	Write(27,17,"0%");

	VW_UpdateScreen();
	IN_StartAck();

	#define RATIOXX		29
	for (i=0;i<=kr;i++)
	{
		itoa(i,tempstr,10);
		x=RATIOXX-strlen(tempstr)*2;
		Write(x,13,tempstr);
		SD_PlaySound(D_INCSND);
		VW_UpdateScreen();
		BJ_Breathe();
		if (IN_CheckAck())
			goto done;
		RollDelay();
	}

	if (kr==100)
	{
		VW_WaitVBL(VBLWAIT);
		SD_StopSound();
		bonus+=PERCENT100AMT;
		ShowBonus(bonus);
		SD_PlaySound(PERCENT100SND);
	}
	else if (kr==0)
	{
		VW_WaitVBL(VBLWAIT);
		SD_StopSound();
		SD_PlaySound(NOBONUSSND);
	}
	else
		SD_PlaySound(D_INCSND);

	VW_UpdateScreen();
	while (SD_SoundPlaying())
		BJ_Breathe();

	for (i=0;i<=tr;i++)
	{
		itoa(i,tempstr,10);
		x=RATIOXX-strlen(tempstr)*2;
		Write(x,15,tempstr);
		SD_PlaySound(D_INCSND);
		VW_UpdateScreen();
		BJ_Breathe();
		if (IN_CheckAck())
			goto done;
		RollDelay();
	}

	if (tr==100)
	{
		VW_WaitVBL(VBLWAIT);
		SD_StopSound();
		bonus+=PERCENT100AMT;
		ShowBonus(bonus);
		SD_PlaySound(PERCENT100SND);
	}
	else if (tr==0)
	{
		VW_WaitVBL(VBLWAIT);
		SD_StopSound();
		SD_PlaySound(NOBONUSSND);
	}
	else
		SD_PlaySound(D_INCSND);

	VW_UpdateScreen();
	while (SD_SoundPlaying())
		BJ_Breathe();

	for (i=0;i<=sr;i++)
	{
		itoa(i,tempstr,10);
		x=RATIOXX-strlen(tempstr)*2;
		Write(x,17,tempstr);
		SD_PlaySound(D_INCSND);
		VW_UpdateScreen();
		BJ_Breathe();
		if (IN_CheckAck())
			goto done;
		RollDelay();
	}

	if (sr==100)
	{
		VW_WaitVBL(VBLWAIT);
		SD_StopSound();
		bonus+=PERCENT100AMT;
		ShowBonus(bonus);
		SD_PlaySound(PERCENT100SND);
	}
	else if (sr==0)
	{
		VW_WaitVBL(VBLWAIT);
		SD_StopSound();
		SD_PlaySound(NOBONUSSND);
	}
	else
		SD_PlaySound(D_INCSND);

	VW_UpdateScreen();
	while (SD_SoundPlaying())
		BJ_Breathe();

	//
	// JUMP STRAIGHT HERE IF KEY PRESSED
	//
	done:

	perfect = (kr==100)&&(tr==100)&&(sr==100);

	itoa(kr,tempstr,10);
	x=RATIOXX-strlen(tempstr)*2;
	Write(x,13,tempstr);

	itoa(tr,tempstr,10);
	x=RATIOXX-strlen(tempstr)*2;
	Write(x,15,tempstr);

	itoa(sr,tempstr,10);
	x=RATIOXX-strlen(tempstr)*2;
	Write(x,17,tempstr);

	//
	// SAVE RATIO INFORMATION FOR ENDGAME
	//
	LevelRatios[gamestate.mapon].kill=kr;
	LevelRatios[gamestate.mapon].secret=sr;
	LevelRatios[gamestate.mapon].treasure=tr;
	LevelRatios[gamestate.mapon].time=min*60+sec;
	LevelRatios[gamestate.mapon].par=parTimes[gamestate.mapon].time*(4200/70);


	GivePoints(bonus);
	ShowBonus(bonus);
	DrawScore();
	VW_UpdateScreen();

	if (Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		PicturePause();

	if (perfect)
	{
		DrawEndLevelScreen(secret);
		ShowBonus(bonus);
		VWB_DrawPic(96,120,W_PERFECTPIC);
		VW_UpdateScreen();
		SD_StopSound();
		SD_PlaySound(PERCENT100SND);
	}
	IN_StartAck();
	TimeCount = 0;
	while (!IN_CheckAck())
		BJ_Breathe();

	if (Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		PicturePause();

	VW_FadeOut();
	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;

	UnCacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);

	{
		id0_int_t start;
		SD_WaitSoundDone();
		switch (SoundMode)
		{
			case sdm_Off: return;
			case sdm_PC: start = STARTPCSOUNDS; break;
			case sdm_AdLib: start = STARTADLIBSOUNDS; break;
		}
		for (i=0;i<NUMSOUNDS;i++,start++)
			MM_FreePtr((memptr *)&audiosegs[start]);
	}

#else // GAMEVER_NOAH3D
	CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	ClearSplitVWB ();			// set up for double buffering in split screen
	VWB_Bar (0,0,320,200-STATUSLINES,127);
	StartCPMusic(ENDLEVEL_MUS);

//
// do the intermission
//
	IN_ClearKeysDown();
	IN_StartAck();

#ifdef JAPAN
	CA_CacheGrChunk(C_INTERMISSIONPIC);
	VWB_DrawPic(0,0,C_INTERMISSIONPIC);
	UNCACHEGRCHUNK(C_INTERMISSIONPIC);
#endif
	VWB_DrawPic(0,16,L_GUYPIC);

#ifndef SPEAR
	if (mapon<8)
#else
	if (mapon != 4 &&
		mapon != 9 &&
		mapon != 15 &&
		mapon < 17)
#endif
	{
#ifndef JAPAN
	 #ifdef SPANISH
	 Write(14,2,"piso\ncompletado");
	 #else
	 Write(14,2,"floor\ncompleted");
	 #endif

	 Write(14,7,STR_BONUS"     0");
	 Write(16,10,STR_TIME);
	 Write(16,12,STR_PAR);

	 // *** SHAREWARE/REGISTERED V1.4 APOGEE RESTORATION ***
	 // Possibly relocated this, depending on version
	#if (GAMEVER_WOLFREV <= GV_WR_SODFG14A)
	 #ifdef SPANISH
	 Write(30,12,parTimes[gamestate.episode*10+mapon].timestr);
	 #else
	 Write(26,12,parTimes[gamestate.episode*10+mapon].timestr);
	 #endif
	#endif

	 #ifdef SPANISH
	 Write(11,14,    STR_RAT2KILL);
	 Write(11,16,  STR_RAT2SECRET);
	 Write(11,18,STR_RAT2TREASURE);
	 #else
	 Write(9,14,    STR_RAT2KILL);
	 Write(5,16,  STR_RAT2SECRET);
	 Write(1,18,STR_RAT2TREASURE);
	 #endif

	 Write(26,2,itoa(gamestate.mapon+1,tempstr,10));
#endif


	 // *** SHAREWARE/REGISTERED V1.4 APOGEE + RESTORATION ***
	 // Possibly relocate this, depending on version
	#if (GAMEVER_WOLFREV > GV_WR_SODFG14A)
	 #ifdef SPANISH
	 Write(30,12,parTimes[gamestate.episode*10+mapon].timestr);
	 #else
	 Write(26,12,parTimes[gamestate.episode*10+mapon].timestr);
	 #endif
	#endif

	 //
	 // PRINT TIME
	 //
	 sec=gamestate.TimeCount/70;

	 if (sec > 99*60)		// 99 minutes max
	   sec = 99*60;

	 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	 if (gamestate.TimeCount<parTimes[gamestate.episode*10+mapon].time)
		timeleft=parTimes[gamestate.episode*10+mapon].time/70-sec;
#else
	 if (gamestate.TimeCount<parTimes[gamestate.episode*10+mapon].time*4200)
		timeleft=(parTimes[gamestate.episode*10+mapon].time*4200)/70-sec;
#endif

	 min=sec/60;
	 sec%=60;

	 #ifdef SPANISH
	 i=30*8;
	 #else
	 i=26*8;
	 #endif
	 VWB_DrawPic(i,10*8,L_NUM0PIC+(min/10));
	 i+=2*8;
	 VWB_DrawPic(i,10*8,L_NUM0PIC+(min%10));
	 i+=2*8;
	 Write(i/8,10,":");
	 i+=1*8;
	 VWB_DrawPic(i,10*8,L_NUM0PIC+(sec/10));
	 i+=2*8;
	 VWB_DrawPic(i,10*8,L_NUM0PIC+(sec%10));

	 VW_UpdateScreen ();
	 VW_FadeIn ();


	 //
	 // FIGURE RATIOS OUT BEFOREHAND
	 //

	 // *** SHAREWARE V1.0 APOGEE RESTORATION *** (Always calculate in v1.0)
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	 kr = sr = tr = 0;
	 if (gamestate.killtotal)
#endif
		kr=(gamestate.killcount*100)/gamestate.killtotal;
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	 if (gamestate.secrettotal)
#endif
		sr=(gamestate.secretcount*100)/gamestate.secrettotal;
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	 if (gamestate.treasuretotal)
#endif
		tr=(gamestate.treasurecount*100)/gamestate.treasuretotal;


	 //
	 // PRINT TIME BONUS
	 //
	 bonus=timeleft*PAR_AMOUNT;
	 if (bonus)
	 {
	  for (i=0;i<=timeleft;i++)
	  {
	   ltoa((id0_long_t)i*PAR_AMOUNT,tempstr,10);
	   x=36-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   if (!(i%(PAR_AMOUNT/10)))
		 SD_PlaySound(ENDBONUS1SND);
	   VW_UpdateScreen();
	   while(SD_SoundPlaying())
		 BJ_Breathe();
	   if (IN_CheckAck())
		 goto done;
	  }

	  VW_UpdateScreen();
	  SD_PlaySound(ENDBONUS2SND);
	  while(SD_SoundPlaying())
		BJ_Breathe();
	 }


	 #ifdef SPANISH
	 #define RATIOXX		33
	 #else
	 #define RATIOXX		37
	 #endif
	 //
	 // KILL RATIO
	 //
	 ratio=kr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=RATIOXX-strlen(tempstr)*2;
	  Write(x,14,tempstr);
	  if (!(i%10))
		SD_PlaySound(ENDBONUS1SND);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();

	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=(RATIOXX-1)-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(PERCENT100SND);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(NOBONUSSND);
	 }
	 else
	 SD_PlaySound(ENDBONUS2SND);

	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
	   BJ_Breathe();


	 //
	 // SECRET RATIO
	 //
	 ratio=sr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=RATIOXX-strlen(tempstr)*2;
	  Write(x,16,tempstr);
	  if (!(i%10))
		SD_PlaySound(ENDBONUS1SND);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();
	  BJ_Breathe();

	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=(RATIOXX-1)-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(PERCENT100SND);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(NOBONUSSND);
	 }
	 else
	   SD_PlaySound(ENDBONUS2SND);
	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
	   BJ_Breathe();


	 //
	 // TREASURE RATIO
	 //
	 ratio=tr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=RATIOXX-strlen(tempstr)*2;
	  Write(x,18,tempstr);
	  if (!(i%10))
		SD_PlaySound(ENDBONUS1SND);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();
	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=(RATIOXX-1)-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(PERCENT100SND);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(NOBONUSSND);
	 }
	 else
	 SD_PlaySound(ENDBONUS2SND);
	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
	   BJ_Breathe();


	 //
	 // JUMP STRAIGHT HERE IF KEY PRESSED
	 //
	 done:

	 itoa(kr,tempstr,10);
	 x=RATIOXX-strlen(tempstr)*2;
	 Write(x,14,tempstr);

	 itoa(sr,tempstr,10);
	 x=RATIOXX-strlen(tempstr)*2;
	 Write(x,16,tempstr);

	 itoa(tr,tempstr,10);
	 x=RATIOXX-strlen(tempstr)*2;
	 Write(x,18,tempstr);

	 bonus=(id0_long_t)timeleft*PAR_AMOUNT+
		   (PERCENT100AMT*(kr==100))+
		   (PERCENT100AMT*(sr==100))+
		   (PERCENT100AMT*(tr==100));

	 GivePoints(bonus);
	 ltoa(bonus,tempstr,10);
	 x=36-strlen(tempstr)*2;
	 Write(x,7,tempstr);

	 //
	 // SAVE RATIO INFORMATION FOR ENDGAME
	 //
	 LevelRatios[mapon].kill=kr;
	 LevelRatios[mapon].secret=sr;
	 LevelRatios[mapon].treasure=tr;
	 LevelRatios[mapon].time=min*60+sec;
	}
	else
	{
#ifdef SPEAR
#ifndef SPEARDEMO
	  switch(mapon)
	  {
	   case 4: Write(14,4," trans\n"
						  " grosse\n"
						  STR_DEFEATED); break;
	   case 9: Write(14,4,"barnacle\n"
						  "wilhelm\n"
						  STR_DEFEATED); break;
	   case 15: Write(14,4,"ubermutant\n"
						   STR_DEFEATED); break;
	   case 17: Write(14,4," death\n"
						   " knight\n"
						   STR_DEFEATED); break;
	   case 18: Write(13,4,"secret tunnel\n"
						   "    area\n"
						   "  completed!"); break;
	   case 19: Write(13,4,"secret castle\n"
						   "    area\n"
						   "  completed!"); break;
	  }
#endif
#else
	  // *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	  Write(14,4,"secret level\n completed!");
#else
	  Write(14,4,"secret floor\n completed!");
#endif
#endif

	  Write(10,16,"15000 bonus!");

	  VW_UpdateScreen();
	  VW_FadeIn();

	  GivePoints(15000);
	}


	DrawScore();
	VW_UpdateScreen();

	TimeCount=0;
	IN_StartAck();
	while(!IN_CheckAck())
	  BJ_Breathe();

//
// done
//
#ifdef SPEARDEMO
	if (gamestate.mapon == 1)
	{
		SD_PlaySound (BONUS1UPSND);

		CA_CacheGrChunk (STARTFONT+1);
		Message ("This concludes your demo\n"
				 "of Spear of Destiny! Now,\n"
				 "go to your local software\n"
				 "store and buy it!");
		UNCACHEGRCHUNK (STARTFONT+1);

		IN_ClearKeysDown();
		IN_Ack();
	}
#endif

#ifdef JAPDEMO
	if (gamestate.mapon == 3)
	{
		SD_PlaySound (BONUS1UPSND);

		CA_CacheGrChunk (STARTFONT+1);
		Message ("This concludes your demo\n"
				 "of Wolfenstein 3-D! Now,\n"
				 "go to your local software\n"
				 "store and buy it!");
		UNCACHEGRCHUNK (STARTFONT+1);

		IN_ClearKeysDown();
		IN_Ack();
	}
#endif

	// *** SHAREWARE/REGISTERED APOGEE RESTORATION ***
	// This is also skipped in the Apogee EXEs
	#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A)
	//#ifndef SPEAR
	if (Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
		PicturePause();
	#endif

	VW_FadeOut ();
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	temp = bufferofs;
#endif
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	bufferofs = temp;
#endif

	UnCacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
#endif // GAMEVER_NOAH3D
}



//==========================================================================


/*
=================
=
= PreloadGraphics
=
= Fill the cache up
=
=================
*/

id0_boolean_t PreloadUpdate(id0_unsigned_t current, id0_unsigned_t total)
{
	id0_unsigned_t w = WindowW - 10;


	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w,2,BLACK);
#endif
	w = ((id0_long_t)w * current) / total;
	if (w)
	{
	 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	 VWB_Bar(WindowX + 5,WindowY + WindowH,w,2,0x55);
	 VWB_Bar(WindowX + 5,WindowY + WindowH,w-1,1,0x52);
#else
	 VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w,2,0x37); //SECONDCOLOR);
	 VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w-1,1,0x32);
#endif

	}
	VW_UpdateScreen();
//	if (LastScan == sc_Escape)
//	{
//		IN_ClearKeysDown();
//		return(true);
//	}
//	else
		return(false);
}

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_int_t	FloorTile[] = {0,0,2,1,1,1,0,2,2,2,1,3,0,0,0,0,2,1,1,1,1,1,0,2,2,2,2,2,1,3,3};
#endif

void PreloadGraphics(void)
{
	DrawLevel ();
	ClearSplitVWB ();			// set up for double buffering in split screen

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Bar (0,0,320,200-STATUSLINES,VIEWCOLOR);
#else
	VWB_Bar (0,0,320,200-STATUSLINES,127);
#endif

	LatchDrawPic (20-14,80-3*8,GETPSYCHEDPIC);

	WindowX = 160-14*8;
	WindowY = 80-3*8;
	WindowW = 28*8;
	WindowH = 48;
	VW_UpdateScreen();
	VW_FadeIn ();

	PM_Preload (PreloadUpdate);
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	LoadFloorTiles(FloorTile[gamestate.mapon]);
#endif
	IN_UserInput (70);
	VW_FadeOut ();

	DrawPlayBorder ();
	VW_UpdateScreen ();
}


// *** ALPHA RESTORATION ***
// The functions included here, DrawHighScores and CheckHighScore,
// were present in WL_MAIN.C as of an earlier alpha build
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#include "wl_hscor.c"
#endif

// *** S3DNA RESTORATION ***
// Separating the briefings seems to do the job here (in terms of EXE layout)
#ifdef GAMEVER_NOAH3D
id0_char_t	mission1text[] = {
	"You'll be out of the ark in six\n"
	"days, Noah. Unfortunately, the\n"
	"animals are a tad bit restless and\n"
	"want to get out now. Good thing\n"
	"you brought all that food with\n"
	"you. You'll need it to put the\n"
	"busy ones to sleep.\n"
	"\n"
	"At the end of the first day, be\n"
	"prepared to deal with Carl the\n"
	"Camel. He's been real cranky lately\n"
	"and is a bit out of control.\n"
	"\n"
	"Good luck and be careful...\n"
},

mission2text[] = {
	"Wow, Noah! Carl sure was hard to\n"
	"calm down. But remember, you\n"
	"were chosen to guide this ark to\n"
	"safety because you know how to\n"
	"get the job done.\n"
	"\n"
	"The closing of day two ends with\n"
	"irritable Ginny the Giraffe. She\n"
	"may be tall and quick, but give\n"
	"her enough food and it's off to\n"
	"sleep.\n"
	"\n"
	"Oh, and Noah, remember to keep\n"
	"an eye out for hidden rooms...\n"
},

mission3text[] = {
	"ZZZZZ... Great job, Noah!. Ginny is\n"
	"snoring away.\n"
	"\n"
	"Boy, it seems the animals never\n"
	"seem to stay asleep very long.\n"
	"\n"
	"Keep your eye out for Melvin the\n"
	"Monkey. He can be pretty tricky\n"
	"and may try distracting you with\n"
	"coconuts, but it's your job to get\n"
	"him settled down for the rest of\n"
	"your time aboard the ark."
},

mission4text[] = {
	"You're doing a great job, Noah!\n"
	"You took care of that Melvin like\n"
	"a real pro.\n"
	"\n"
	"I hope you haven't worn yourself\n"
	"out yet, because more challenges\n"
	"are ahead.\n"
	"\n"
	"Kerry the Kangaroo awaits you,\n"
	"and she doesn't look the least\n"
	"bit tired. Are you ready for her?\n"
	"She needs to rest like the other\n"
	"animals."
},

mission5text[] = {
	"Wow! Kerry was no challenge for\n"
	"you! Can you keep up the pace\n"
	"when faced with Ernie the\n"
	"Elephant?\n"
	"\n"
	"Don't worry. Very soon you will\n"
	"be on dry land, and you won't\n"
	"have to chase the animals around\n"
	"the ark anymore."
},

mission6text[] = {
	"You are almost there! If you can\n"
	"get through one more day, you\n"
	"will be out of here for good!\n"
	"\n"
	"Rumor has it one of the bears\n"
	"does not want to be found. In\n"
	"fact he may try to hide from\n"
	"you. Be on the alert for Burt\n"
	"the Bear.\n"
	"\n"
	"Get him to sleep and your work\n"
	"is done!"
},

creditstext[] = {
	"\n"
	"Super 3-D Noah's Ark\n"
	"\n"
	"\n"
	"(C)1995 Wisdom Tree, Inc.\n"
	"All rights reserved.\n"
	"\n"
	"\n"
	"This game contains copyrighted\n"
	"software code owned by\n"
	"Id Software, Inc\n"
	"(C)1992 Id Software, Inc.\n"
	"All rights reserved.\n"
};


id0_char_t *missiontext[] =
	{mission1text,mission2text,mission3text,mission4text,mission5text,mission6text,creditstext};


void Briefing (id0_int_t mission, id0_boolean_t credits)
{
	ClearMemory ();
	if (mission >= 7)
	{
		sprintf (str,"Briefing(): Invalid briefing number %d!",mission);
		Quit (str);
	}
	CacheLump (LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	ClearSplitVWB ();
	ClearMScreen ();
	CA_CacheGrChunk (STARTFONT+1);
	fontnumber = 1;
	WindowX = PrintX = 8;
	WindowY = PrintY = 8;
	WindowW = 320-16;
	WindowH = 200-16;
	SETFONTCOLOR (READHCOLOR,BKGDCOLOR);

	if (credits)
		US_CPrint (missiontext[mission]);
	else
		US_Print (missiontext[mission]);

	VW_UpdateScreen ();
	VW_FadeIn ();

	if (credits)
		UnCacheLump (LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	else
	{
		StartCPMusic (NOAH09_MUS);
		IN_ClearKeysDown();
		IN_Ack();
		StopMusic();
		UnCacheLump (LEVELEND_LUMP_START,LEVELEND_LUMP_END);
		DrawPlayScreen();
	}
	UNCACHEGRCHUNK (STARTFONT+1);
	ClearMemory ();
}
#endif


#ifndef UPLOAD
#ifndef SPEAR
#ifndef JAPAN
////////////////////////////////////////////////////////
//
// NON-SHAREWARE NOTICE
//
////////////////////////////////////////////////////////
void NonShareware(void)
{
	VW_FadeOut();

	ClearMScreen();
	DrawStripes(10);

	CA_CacheGrChunk(STARTFONT+1);
	fontnumber = 1;

	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintX=110;
	PrintY=15;

	#ifdef SPANISH
	US_Print("Atencion");
	#else
	US_Print("Attention");
	#endif

	SETFONTCOLOR(HIGHLIGHT,BKGDCOLOR);
	WindowX=PrintX=40;
	PrintY=60;
	#ifdef SPANISH
	US_Print("Este juego NO es gratis y\n");
	US_Print("NO es Shareware; favor de\n");
	US_Print("no distribuirlo.\n\n");
	#else
	US_Print("This game is NOT shareware.\n");
	US_Print("Please do not distribute it.\n");
	US_Print("Thanks.\n\n");
	#endif
	US_Print("        Id Software\n");

	VW_UpdateScreen ();
	VW_FadeIn();
	IN_Ack();
}
#endif
#endif
#endif

#ifdef SPEAR
#ifndef SPEARDEMO
////////////////////////////////////////////////////////
//
// COPY PROTECTION FOR FormGen
//
////////////////////////////////////////////////////////
id0_char_t 	id0_far CopyProFailedStrs[][100] = {
			STR_COPY1,
			STR_COPY2,

			STR_COPY3,
			STR_COPY4,

			STR_COPY5,
			STR_COPY6,

			STR_COPY7,
			STR_COPY8,

			STR_COPY9,
			"",

			STR_COPY10,
			STR_COPY11,

			STR_COPY12,
			"",

			STR_COPY13,
			"",

			STR_COPY14,
			""
			},

		id0_far BackDoorStrs[5][16] = {
			"a spoon?",
			"bite me!",
			"joshua",
			"pelt",
#ifdef BETA
			"beta"
#else
			"snoops"
#endif
			},

		id0_far GoodBoyStrs[10][40] = {
			"...is the CORRECT ANSWER!",
			"",

			"Consider yourself bitten, sir.",
			"",

			"Greetings Professor Falken, would you",
			"like to play Spear of Destiny?",

			"Do you have any gold spray paint?",
			"",

#ifdef BETA
			"Beta testing approved.",
#else
			"I wish I had a 21\" monitor...",
#endif
			""
			},

		id0_far bossstrs[4][24] = {
			"DEATH KNIGHT",
			"BARNACLE WILHELM",
			"UBERMUTANTUBER MUTANT",
			"TRANS GROSSE"
			},

		id0_far WordStr[5][20] = {
			"New Game",
			"Sound...F4",
			"Control...F6",
			"Change View...F5",
			"Quit...F10"},

		id0_far	WordCorrect[5][2] = {"3","4","4","5","5"},

		id0_far MemberStr[10][40] = {
			STR_COPY15,
			"",

			STR_COPY16,
			"",

			STR_COPY17,
			STR_COPY18,

			STR_COPY19,
			STR_COPY20,

			STR_COPY21,
			STR_COPY22},

		id0_far MemberCorrect[5][24] = {
			"adrian carmack",
			"john carmackjohn romero",
			"tom hall",
			"jay wilbur",
			"kevin cloud"},

		id0_far DosMessages[9][80] = {
			STR_NOPE1,
			STR_NOPE2,
			STR_NOPE3,
			STR_NOPE4,
			STR_NOPE5,
			STR_NOPE6,
			STR_NOPE7,
			STR_NOPE8,
			STR_NOPE9},

		id0_far MiscTitle[4][20] = {
			"BLOOD TEST",
			"STRAIGHT-LACED",
			"QUITE SHAPELY",
			"I AM WHAT I AMMO"
			},

		id0_far MiscStr[12][40] = {
			STR_MISC1,
			STR_MISC2,
			"",

			STR_MISC3,
			STR_MISC4,
			"",

			STR_MISC5,
			STR_MISC6,
			"",

			STR_MISC7,
			STR_MISC8,
			STR_MISC9
			},

		id0_far MiscCorrect[4][5] = {"ss","8",STR_STAR,"45"};


id0_int_t  BackDoor(id0_char_t *s)
{
	id0_int_t i;


	strlwr(s);

	for (i=0;i<5;i++)
		if (!_fstrcmp(s,BackDoorStrs[i]))
		{
			SETFONTCOLOR(14,15);
			fontnumber = 0;
			PrintY = 175;
			VWB_DrawPic (0,20*8,COPYPROTBOXPIC);
			US_CPrint(GoodBoyStrs[i*2]);
			US_CPrint(GoodBoyStrs[i*2+1]);
			VW_UpdateScreen();
			return 1;
		}

	return 0;
}


void CopyProtection(void)
{
#define TYPEBOX_Y		177
#define TYPEBOX_BKGD	0x9c
#define PRINTCOLOR		HIGHLIGHT

	id0_int_t	i,match,whichboss,bossnum,try,whichline,enemypicked[4]={0,0,0,0},
		bosses[4] = { BOSSPIC1PIC,BOSSPIC2PIC,BOSSPIC3PIC,BOSSPIC4PIC },
		whichone,whichpicked[4]={0,0,0,0},quiztype,whichmem,
		memberpicked[5]={0,0,0,0,0},wordpicked[5]={0,0,0,0,0},whichword;

	id0_char_t	inputbuffer[20],
			message[80];

	enum
	{
		debriefing,
		checkmanual,
		staffquiz,
		miscquiz,

		totaltypes
	};



	try = 0;
	VW_FadeOut();
	CA_CacheGrChunk(C_BACKDROPPIC);
	CacheLump(COPYPROT_LUMP_START,COPYPROT_LUMP_END);
	CA_CacheGrChunk(STARTFONT+1);
	CA_LoadAllSounds();
	StartCPMusic(COPYPRO_MUS);
	US_InitRndT(true);

	while (try<3)
	{
		fontnumber = 1;
		SETFONTCOLOR(PRINTCOLOR-2,15);
		VWB_DrawPic (0,0,C_BACKDROPPIC);
		VWB_DrawPic (0,0,COPYPROTTOPPIC);
		VWB_DrawPic (0,20*8,COPYPROTBOXPIC);
		WindowX = WindowY = 0;
		WindowW = 320;
		WindowH = 200;
		PrintY = 65;

		quiztype = US_RndT()%totaltypes;
		switch(quiztype)
		{
			//
			// BOSSES QUIZ
			//
			case debriefing:
				PrintX = 0;
				US_Print(STR_DEBRIEF);
				SETFONTCOLOR(PRINTCOLOR,15);

				while (enemypicked[whichboss = US_RndT()&3]);
				enemypicked[whichboss] = 1;
				bossnum = bosses[whichboss];
				VWB_DrawPic(128,60,bossnum);
				fontnumber = 0;
				PrintY = 130;
				US_CPrint(STR_ENEMY1"\n");
				US_CPrint(STR_ENEMY2"\n\n");

				VW_UpdateScreen();
				VW_FadeIn();

				PrintX = 100;
				fontcolor = 15;
				backcolor = TYPEBOX_BKGD;
				inputbuffer[0] = 0;
				PrintY = TYPEBOX_Y;
				fontnumber = 1;
				US_LineInput(PrintX,PrintY,inputbuffer,nil,true,20,100);

				match = 0;
				for (i=0;i<_fstrlen(bossstrs[whichboss]);i++)
					if (!_fstrnicmp(inputbuffer,bossstrs[whichboss]+i,strlen(inputbuffer)) &&
						strlen(inputbuffer)>3)
						match = 1;

				match += BackDoor(inputbuffer);
				break;

			//
			// MANUAL CHECK
			//
			case checkmanual:
				while (wordpicked[whichword = US_RndT()%5]);
				wordpicked[whichword] = 1;
				US_CPrint(STR_CHECKMAN);
				SETFONTCOLOR(PRINTCOLOR,15);
				PrintY += 25;
				US_CPrint(STR_MAN1);
				US_CPrint(STR_MAN2);
				_fstrcpy(message,STR_MAN3" \"");
				_fstrcat(message,WordStr[whichword]);
				_fstrcat(message,"\" "STR_MAN4);
				US_CPrint(message);
				VW_UpdateScreen();
				VW_FadeIn();

				PrintX = 146;
				fontcolor = 15;
				backcolor = TYPEBOX_BKGD;
				inputbuffer[0] = 0;
				PrintY = TYPEBOX_Y;
				US_LineInput(PrintX,PrintY,inputbuffer,nil,true,6,100);

				strlwr(inputbuffer);
				match = 1-(_fstrcmp(inputbuffer,WordCorrect[whichword])!=0);
				match += BackDoor(inputbuffer);
				break;

			//
			// STAFF QUIZ
			//
			case staffquiz:
				while (memberpicked[whichmem = US_RndT()%5]);
				memberpicked[whichmem] = 1;
				US_CPrint(STR_ID1);
				SETFONTCOLOR(PRINTCOLOR,15);
				PrintY += 25;
				US_CPrint(MemberStr[whichmem*2]);
				US_CPrint(MemberStr[whichmem*2+1]);
				VW_UpdateScreen();
				VW_FadeIn();

				PrintX = 100;
				fontcolor = 15;
				backcolor = TYPEBOX_BKGD;
				inputbuffer[0] = 0;
				PrintY = TYPEBOX_Y;
				US_LineInput(PrintX,PrintY,inputbuffer,nil,true,20,120);

				strlwr(inputbuffer);
				match = 0;
				for (i=0;i<_fstrlen(MemberCorrect[whichmem]);i++)
					if (!_fstrnicmp(inputbuffer,MemberCorrect[whichmem]+i,strlen(inputbuffer)) &&
						strlen(inputbuffer)>2)
							match = 1;
				match += BackDoor(inputbuffer);
				break;

			//
			// MISCELLANEOUS QUESTIONS
			//
			case miscquiz:
				while (whichpicked[whichone = US_RndT()&3]);
				whichpicked[whichone] = 1;
				US_CPrint(MiscTitle[whichone]);
				SETFONTCOLOR(PRINTCOLOR,15);
				PrintY += 25;
				US_CPrint(MiscStr[whichone*3]);
				US_CPrint(MiscStr[whichone*3+1]);
				US_CPrint(MiscStr[whichone*3+2]);
				VW_UpdateScreen();
				VW_FadeIn();

				PrintX = 146;
				fontcolor = 15;
				backcolor = TYPEBOX_BKGD;
				inputbuffer[0] = 0;
				PrintY = TYPEBOX_Y;
				US_LineInput(PrintX,PrintY,inputbuffer,nil,true,6,100);

				strlwr(inputbuffer);
				match = 1-(_fstrcmp(inputbuffer,MiscCorrect[whichone])!=0);
				match += BackDoor(inputbuffer);
				break;
			}

		//
		// IF NO MATCH, WE'VE GOT A (MINOR) PROBLEM!
		//

		if (!match)
		{
			whichline = 2*(US_RndT()%9);
			SETFONTCOLOR(14,15);
			fontnumber = 0;
			PrintY = 175;
			VWB_DrawPic (0,20*8,COPYPROTBOXPIC);
			US_CPrint(CopyProFailedStrs[whichline]);
			US_CPrint(CopyProFailedStrs[whichline+1]);

			VW_UpdateScreen();
			SD_PlaySound(NOWAYSND);
			IN_UserInput(TickBase*3);
			VW_FadeOut();
			try++;
		}
		else
		{
			id0_int_t start;


			SD_PlaySound(BONUS1UPSND);
			SD_WaitSoundDone();
			UNCACHEGRCHUNK (STARTFONT+1);
			UNCACHEGRCHUNK (C_BACKDROPPIC);
			UnCacheLump (COPYPROT_LUMP_START,COPYPROT_LUMP_END);

			switch(SoundMode)
			{
				case sdm_Off: return;
				case sdm_PC: start = STARTPCSOUNDS; break;
				case sdm_AdLib: start = STARTADLIBSOUNDS;
			}

			for (i=0;i<NUMSOUNDS;i++,start++)
				MM_FreePtr ((memptr *)&audiosegs[start]);
			return;
		}
	}

	ClearMemory();
	ShutdownId();

	_fstrcpy(message,DosMessages[US_RndT()%9]);

	_AX = 3;
	geninterrupt(0x10);

	printf("%s\n",message);
	exit(1);
}

#endif // SPEARDEMO
#endif // SPEAR
//===========================================================================
