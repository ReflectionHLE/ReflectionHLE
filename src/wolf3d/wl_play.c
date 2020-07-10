// WL_PLAY.C

#include "wl_def.h"
//#pragma hdrstop


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define sc_Question	0x35

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

//
// *** PRE-V1.4 APOGEE RESTORATION *** - There were apparently some unused variable here
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
id0_boolean_t		unusedplayvar;
#endif
id0_boolean_t		madenoise;					// true when shooting or screaming

exit_t		playstate;

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_int_t			DebugOk;
#endif

objtype 	objlist[MAXACTORS],*new,*obj,*player,*lastobj,
			*objfreelist,*killerobj;

id0_unsigned_t	farmapylookup[MAPSIZE];
id0_byte_t		*nearmapylookup[MAPSIZE];

id0_boolean_t		singlestep,godmode,noclip;
id0_int_t			extravbls;

id0_byte_t		tilemap[MAPSIZE][MAPSIZE];	// wall values only
id0_byte_t		spotvis[MAPSIZE][MAPSIZE];
objtype		*actorat[MAPSIZE][MAPSIZE];

//
// replacing refresh manager
//
id0_unsigned_t	mapwidth,mapheight,tics;
id0_boolean_t		compatability;
id0_byte_t		*updateptr;
id0_unsigned_t	mapwidthtable[64];
id0_unsigned_t	uwidthtable[UPDATEHIGH];
id0_unsigned_t	blockstarts[UPDATEWIDE*UPDATEHIGH];
id0_byte_t		update[UPDATESIZE];

//
// control info
//
id0_boolean_t		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
id0_int_t			joystickport;
id0_int_t			dirscan[4] = {sc_UpArrow,sc_RightArrow,sc_DownArrow,sc_LeftArrow};
id0_int_t			buttonscan[NUMBUTTONS] =
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			{sc_Control,sc_Alt,sc_RShift,sc_Space,sc_1,sc_2,sc_3,sc_4,sc_5,sc_6};
#else
			{sc_Control,sc_Alt,sc_RShift,sc_Space,sc_1,sc_2,sc_3,sc_4};
#endif
id0_int_t			buttonmouse[4]={bt_attack,bt_strafe,bt_use,bt_nobutton};
id0_int_t			buttonjoy[4]={bt_attack,bt_strafe,bt_use,bt_run};

id0_int_t			viewsize;

id0_boolean_t		buttonheld[NUMBUTTONS];

id0_boolean_t		demorecord,demoplayback;
id0_char_t		id0_far *demoptr, id0_far *lastdemoptr;
memptr		demobuffer;

//
// curent user input
//
id0_int_t			controlx,controly;		// range from -100 to 100 per tic
id0_boolean_t		buttonstate[NUMBUTTONS];



//===========================================================================


void	CenterWindow(id0_word_t w,id0_word_t h);
void 	InitObjList (void);
void 	RemoveObj (objtype *gone);
void 	PollControls (void);
void 	StopMusic(void);
void 	StartMusic(void);
void	PlayLoop (void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


objtype dummyobj;

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
//
// LIST OF SONGS FOR EACH VERSION
//
id0_int_t songs[]=
{
 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 //
 // Episode One
 //
 NOAH01_MUS,
 NOAH02_MUS,
 NOAH07_MUS,

 //
 // Episode Two
 //
 NOAH03_MUS,
 NOAH01_MUS,
 NOAH02_MUS,
 NOAH07_MUS,

 //
 // Episode Three
 //
 NOAH04_MUS,
 NOAH03_MUS,
 NOAH01_MUS,
 NOAH07_MUS,
 NOAH09_MUS,

 //
 // Episode Four
 //
 NOAH06_MUS,
 NOAH04_MUS,
 NOAH02_MUS,
 NOAH03_MUS,
 NOAH07_MUS,

 //
 // Episode Five
 //
 NOAH08_MUS,
 NOAH01_MUS,
 NOAH06_MUS,
 NOAH04_MUS,
 NOAH10_MUS,
 NOAH07_MUS,

 //
 // Episode Six
 //
 NOAH11_MUS,
 NOAH02_MUS,
 NOAH03_MUS,
 NOAH08_MUS,
 NOAH06_MUS,
 NOAH07_MUS,
 NOAH09_MUS,

 //
 // Finale
 //
 NOAH02_MUS

#elif (!defined SPEAR)
//#ifndef SPEAR
 //
 // Episode One
 //
 GETTHEM_MUS,
 SEARCHN_MUS,
 POW_MUS,
 SUSPENSE_MUS,
 GETTHEM_MUS,
 SEARCHN_MUS,
 POW_MUS,
 SUSPENSE_MUS,

 WARMARCH_MUS,	// Boss level
 CORNER_MUS,	// Secret level

 //
 // Episode Two
 //
 NAZI_OMI_MUS,
 PREGNANT_MUS,
 GOINGAFT_MUS,
 HEADACHE_MUS,
 NAZI_OMI_MUS,
 PREGNANT_MUS,
 HEADACHE_MUS,
 GOINGAFT_MUS,

 WARMARCH_MUS,	// Boss level
 DUNGEON_MUS,	// Secret level

 //
 // Episode Three
 //
 INTROCW3_MUS,
 NAZI_RAP_MUS,
 TWELFTH_MUS,
 ZEROHOUR_MUS,
 INTROCW3_MUS,
 NAZI_RAP_MUS,
 TWELFTH_MUS,
 ZEROHOUR_MUS,

 ULTIMATE_MUS,	// Boss level
 PACMAN_MUS,	// Secret level

 // *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
 //
 // Episode Four
 //
 GETTHEM_MUS,
 SEARCHN_MUS,
 POW_MUS,
 SUSPENSE_MUS,
 GETTHEM_MUS,
 SEARCHN_MUS,
 POW_MUS,
 SUSPENSE_MUS,

 WARMARCH_MUS,	// Boss level
 CORNER_MUS,	// Secret level

 //
 // Episode Five
 //
 NAZI_OMI_MUS,
 PREGNANT_MUS,
 GOINGAFT_MUS,
 HEADACHE_MUS,
 NAZI_OMI_MUS,
 PREGNANT_MUS,
 HEADACHE_MUS,
 GOINGAFT_MUS,

 WARMARCH_MUS,	// Boss level
 DUNGEON_MUS,	// Secret level

 //
 // Episode Six
 //
 INTROCW3_MUS,
 NAZI_RAP_MUS,
 TWELFTH_MUS,
 ZEROHOUR_MUS,
 INTROCW3_MUS,
 NAZI_RAP_MUS,
 TWELFTH_MUS,
 ZEROHOUR_MUS,

 ULTIMATE_MUS,	// Boss level
 FUNKYOU_MUS		// Secret level
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
#else

 //////////////////////////////////////////////////////////////
 //
 // SPEAR OF DESTINY TRACKS
 //
 //////////////////////////////////////////////////////////////
 XTIPTOE_MUS,
 XFUNKIE_MUS,
 XDEATH_MUS,
 XGETYOU_MUS,		// DON'T KNOW
 ULTIMATE_MUS,	// Trans Gr”sse

 DUNGEON_MUS,
 GOINGAFT_MUS,
 POW_MUS,
 TWELFTH_MUS,
 ULTIMATE_MUS,	// Barnacle Wilhelm BOSS

 NAZI_OMI_MUS,
 GETTHEM_MUS,
 SUSPENSE_MUS,
 SEARCHN_MUS,
 ZEROHOUR_MUS,
 ULTIMATE_MUS,	// Super Mutant BOSS

 XPUTIT_MUS,
 ULTIMATE_MUS,	// Death Knight BOSS

 XJAZNAZI_MUS,	// Secret level
 XFUNKIE_MUS,	// Secret level (DON'T KNOW)

 XEVIL_MUS		// Angel of Death BOSS

#endif
};
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


/*
=============================================================================

						  USER CONTROL

=============================================================================
*/


#define BASEMOVE		35
#define RUNMOVE			70
#define BASETURN		35
#define RUNTURN			70

#define JOYSCALE		2

/*
===================
=
= PollKeyboardButtons
=
===================
*/

void PollKeyboardButtons (void)
{
	id0_int_t		i;

	for (i=0;i<NUMBUTTONS;i++)
		if (Keyboard[buttonscan[i]])
			buttonstate[i] = true;
}


/*
===================
=
= PollMouseButtons
=
===================
*/

void PollMouseButtons (void)
{
	id0_int_t	buttons;

	buttons = IN_MouseButtons ();

	if (buttons&1)
		buttonstate[buttonmouse[0]] = true;
	if (buttons&2)
		buttonstate[buttonmouse[1]] = true;
	if (buttons&4)
		buttonstate[buttonmouse[2]] = true;
}



/*
===================
=
= PollJoystickButtons
=
===================
*/

void PollJoystickButtons (void)
{
	id0_int_t	buttons;

	buttons = IN_JoyButtons ();

	if (joystickport && !joypadenabled)
	{
		if (buttons&4)
			buttonstate[buttonjoy[0]] = true;
		if (buttons&8)
			buttonstate[buttonjoy[1]] = true;
	}
	else
	{
		if (buttons&1)
			buttonstate[buttonjoy[0]] = true;
		if (buttons&2)
			buttonstate[buttonjoy[1]] = true;
		if (joypadenabled)
		{
			if (buttons&4)
				buttonstate[buttonjoy[2]] = true;
			if (buttons&8)
				buttonstate[buttonjoy[3]] = true;
		}
	}
}


/*
===================
=
= PollKeyboardMove
=
===================
*/

void PollKeyboardMove (void)
{
	if (buttonstate[bt_run])
	{
		if (Keyboard[dirscan[di_north]])
			controly -= RUNMOVE*tics;
		if (Keyboard[dirscan[di_south]])
			controly += RUNMOVE*tics;
		if (Keyboard[dirscan[di_west]])
			controlx -= RUNMOVE*tics;
		if (Keyboard[dirscan[di_east]])
			controlx += RUNMOVE*tics;
	}
	else
	{
		if (Keyboard[dirscan[di_north]])
			controly -= BASEMOVE*tics;
		if (Keyboard[dirscan[di_south]])
			controly += BASEMOVE*tics;
		if (Keyboard[dirscan[di_west]])
			controlx -= BASEMOVE*tics;
		if (Keyboard[dirscan[di_east]])
			controlx += BASEMOVE*tics;
	}
}


/*
===================
=
= PollMouseMove
=
===================
*/

void PollMouseMove (void)
{
	id0_int_t	mousexmove,mouseymove;

	Mouse(MDelta);
	mousexmove = _CX;
	mouseymove = _DX;

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	controlx += mousexmove;
	controly += mouseymove*2;
#else
	controlx += mousexmove*10/(13-mouseadjustment);
	controly += mouseymove*20/(13-mouseadjustment);
#endif
}



/*
===================
=
= PollJoystickMove
=
===================
*/

void PollJoystickMove (void)
{
	id0_int_t	joyx,joyy;

	INL_GetJoyDelta(joystickport,&joyx,&joyy);

	if (joystickprogressive)
	{
		if (joyx > 64)
			controlx += (joyx-64)*JOYSCALE*tics;
		else if (joyx < -64)
			controlx -= (-joyx-64)*JOYSCALE*tics;
		if (joyy > 64)
			controlx += (joyy-64)*JOYSCALE*tics;
		else if (joyy < -64)
			controly -= (-joyy-64)*JOYSCALE*tics;
	}
	else if (buttonstate[bt_run])
	{
		if (joyx > 64)
			controlx += RUNMOVE*tics;
		else if (joyx < -64)
			controlx -= RUNMOVE*tics;
		if (joyy > 64)
			controly += RUNMOVE*tics;
		else if (joyy < -64)
			controly -= RUNMOVE*tics;
	}
	else
	{
		if (joyx > 64)
			controlx += BASEMOVE*tics;
		else if (joyx < -64)
			controlx -= BASEMOVE*tics;
		if (joyy > 64)
			controly += BASEMOVE*tics;
		else if (joyy < -64)
			controly -= BASEMOVE*tics;
	}
}


/*
===================
=
= PollControls
=
= Gets user or demo input, call once each frame
=
= controlx		set between -100 and 100 per tic
= controly
= buttonheld[]	the state of the buttons LAST frame
= buttonstate[]	the state of the buttons THIS frame
=
===================
*/

void PollControls (void)
{
	id0_int_t		max,min,i;
	id0_byte_t	buttonbits;

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
//
// get timing info for last frame
//
	if (demoplayback)
	{
		while (TimeCount<lasttimecount+DEMOTICS)
		;
		TimeCount = lasttimecount + DEMOTICS;
		lasttimecount += DEMOTICS;
		tics = DEMOTICS;
	}
	else if (demorecord)			// demo recording and playback needs
	{								// to be constant
//
// take DEMOTICS or more tics, and modify Timecount to reflect time taken
//
		while (TimeCount<lasttimecount+DEMOTICS)
		;
		TimeCount = lasttimecount + DEMOTICS;
		lasttimecount += DEMOTICS;
		tics = DEMOTICS;
	}
	else
		CalcTics ();
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11

	controlx = 0;
	controly = 0;
	memcpy (buttonheld,buttonstate,sizeof(buttonstate));
	memset (buttonstate,0,sizeof(buttonstate));

	if (demoplayback)
	{
	//
	// read commands from demo buffer
	//
		buttonbits = *demoptr++;
		for (i=0;i<NUMBUTTONS;i++)
		{
			buttonstate[i] = buttonbits&1;
			buttonbits >>= 1;
		}

		controlx = *demoptr++;
		controly = *demoptr++;
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		tics = *demoptr++;

		while (tics > TimeCount - lasttimecount);

		lasttimecount = TimeCount;
#endif

		if (demoptr == lastdemoptr)
			playstate = ex_completed;		// demo is done

		controlx *= (id0_int_t)tics;
		controly *= (id0_int_t)tics;

		return;
	}

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	CalcTics();
#endif

//
// get button states
//
	PollKeyboardButtons ();

	if (mouseenabled)
		PollMouseButtons ();

	if (joystickenabled)
		PollJoystickButtons ();

//
// get movements
//
	PollKeyboardMove ();

	if (mouseenabled)
		PollMouseMove ();

	if (joystickenabled)
		PollJoystickMove ();

//
// bound movement to a maximum
//
	max = 100*tics;
	min = -max;
	if (controlx > max)
		controlx = max;
	else if (controlx < min)
		controlx = min;

	if (controly > max)
		controly = max;
	else if (controly < min)
		controly = min;

	if (demorecord)
	{
	//
	// save info out to demo buffer
	//
		controlx /= (id0_int_t)tics;
		controly /= (id0_int_t)tics;

		buttonbits = 0;

		for (i=NUMBUTTONS-1;i>=0;i--)
		{
			buttonbits <<= 1;
			if (buttonstate[i])
				buttonbits |= 1;
		}

		*demoptr++ = buttonbits;
		*demoptr++ = controlx;
		*demoptr++ = controly;
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		*demoptr++ = tics;
#endif

		if (demoptr >= lastdemoptr)
			Quit ("Demo buffer overflowed!");

		controlx *= (id0_int_t)tics;
		controly *= (id0_int_t)tics;
	}
}



//==========================================================================



///////////////////////////////////////////////////////////////////////////
//
//	CenterWindow() - Generates a window of a given width & height in the
//		middle of the screen
//
///////////////////////////////////////////////////////////////////////////

#define MAXX	320
#define MAXY	160

void	CenterWindow(id0_word_t w,id0_word_t h)
{
	FixOfs ();
	US_DrawWindow(((MAXX / 8) - w) / 2,((MAXY / 8) - h) / 2,w,h);
}

//===========================================================================


/*
=====================
=
= CheckKeys
=
=====================
*/

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
void CheckKeys (id0_boolean_t mapmode)
#else
void CheckKeys (void)
#endif
{
	id0_int_t		i;
	id0_byte_t	scan;
	id0_unsigned_t	temp;


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (screenfaded)	// don't do anything with a faded screen
#else
	if (screenfaded || demoplayback)	// don't do anything with a faded screen
#endif
		return;

	scan = LastScan;


	#ifdef SPEAR
	//
	// SECRET CHEAT CODE: TAB-G-F10
	//
	if (Keyboard[sc_Tab] &&
		Keyboard[sc_G] &&
		Keyboard[sc_F10])
	{
		WindowH = 160;
		if (godmode)
		{
			Message ("God mode OFF");
			SD_PlaySound (NOBONUSSND);
		}
		else
		{
			Message ("God mode ON");
			SD_PlaySound (ENDBONUS2SND);
		}

		IN_Ack();
		godmode ^= 1;
		DrawAllPlayBorderSides ();
		IN_ClearKeysDown();
		return;
	}
	#endif


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (Keyboard[sc_J] && Keyboard[sc_I] && Keyboard[sc_M])
	{
		ClearMemory ();
		WindowH = 160;
		godmode ^= 1;
		if (godmode)
		{
			Message ("Invulnerability ON");
			SD_PlaySound (BONUS1UPSND);
		}
		else
		{
			Message ("Invulnerability OFF");
			SD_PlaySound (NOBONUSSND);
		}

		IN_Ack();
		IN_ClearKeysDown();

		gamestate.health = 100;
		gamestate.ammo = gamestate.maxammo = 299;
		gamestate.gas = gamestate.missiles = 99;
		gamestate.keys = 3;
		gamestate.automap = true;
		gamestate.machinegun = 1;
		gamestate.chaingun = 1;
		gamestate.flamethrower = 1;
		gamestate.missile = 1;
		GiveWeapon (wp_chaingun);
		gamestate.score = 0;
		gamestate.TimeCount += 42000L;

		DrawHealth();
		DrawKeys();
		DrawAmmo();
		DrawScore();

		PM_CheckMainMem ();

		DrawAllPlayBorder ();

		return;
	}
#else
	//
	// SECRET CHEAT CODE: 'MLI'
	//
	if (Keyboard[sc_M] &&
		Keyboard[sc_L] &&
		Keyboard[sc_I])
	{
		gamestate.health = 100;
		gamestate.ammo = 99;
		gamestate.keys = 3;
		gamestate.score = 0;
		gamestate.TimeCount += 42000L;
		GiveWeapon (wp_chaingun);

		DrawWeapon();
		DrawHealth();
		DrawKeys();
		DrawAmmo();
		DrawScore();

		ClearMemory ();
		CA_CacheGrChunk (STARTFONT+1);
		ClearSplitVWB ();
		VW_ScreenToScreen (displayofs,bufferofs,80,160);

		Message(STR_CHEATER1"\n"
				STR_CHEATER2"\n\n"
				STR_CHEATER3"\n"
				STR_CHEATER4"\n"
				STR_CHEATER5);

		UNCACHEGRCHUNK(STARTFONT+1);
		PM_CheckMainMem ();
		IN_ClearKeysDown();
		IN_Ack();

		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		for (i=0;i<3;i++)
		{
			bufferofs = screenloc[i];
			DrawPlayBorderSides ();
		}
#else
		DrawAllPlayBorder ();
#endif
	}
#endif // GAMEVER_NOAH3D

	//
	// OPEN UP DEBUG KEYS
	//
#ifndef SPEAR
	// *** PRE-V1.4 (INCLUDING V1.0) APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (Keyboard[sc_Tab] &&
		Keyboard[sc_Control] &&
		Keyboard[sc_Enter] &&
#else
	if (Keyboard[sc_BackSpace] &&
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		Keyboard[sc_Alt] &&
		Keyboard[sc_LShift] &&
#else
		Keyboard[sc_LShift] &&
		Keyboard[sc_Alt] &&
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10
		MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
#else
	if (Keyboard[sc_BackSpace] &&
		Keyboard[sc_LShift] &&
		Keyboard[sc_Alt] &&
		MS_CheckParm("debugmode"))
#endif
	{
	 ClearMemory ();
	 CA_CacheGrChunk (STARTFONT+1);
	 ClearSplitVWB ();
	 VW_ScreenToScreen (displayofs,bufferofs,80,160);

	 Message("Debugging keys are\nnow available!");
	 UNCACHEGRCHUNK(STARTFONT+1);
	 PM_CheckMainMem ();
	 IN_ClearKeysDown();
	 IN_Ack();

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorderSides ();
	}
#else
	 DrawAllPlayBorderSides ();
#endif
	 DebugOk=1;
	}

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	//
	// TRYING THE KEEN CHEAT CODE!
	//
	if (Keyboard[sc_B] &&
		Keyboard[sc_A] &&
		Keyboard[sc_T])
	{
	 ClearMemory ();
	 CA_CacheGrChunk (STARTFONT+1);
	 ClearSplitVWB ();
	 VW_ScreenToScreen (displayofs,bufferofs,80,160);

	 Message("Commander Keen is also\n"
			 "available from Apogee, but\n"
			 "then, you already know\n"
			 "that - right, Cheatmeister?!");

	 UNCACHEGRCHUNK(STARTFONT+1);
	 PM_CheckMainMem ();
	 IN_ClearKeysDown();
	 IN_Ack();

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorderSides ();
	}
#else
	 DrawAllPlayBorder ();
#endif
	}
#endif // GAMEVER_NOAH3D
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
//
// pause key weirdness can't be checked as a scan code
//
	if (Paused)
	{
		bufferofs = displayofs;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (!MS_CheckParm("photoshoot"))
#endif
			LatchDrawPic (20-4,80-2*8,PAUSEDPIC);
		SD_MusicOff();
		IN_Ack();
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		IN_ClearKeysDown ();
#endif
		SD_MusicOn();
		Paused = false;
		if (MousePresent)
			Mouse(MDelta);	// Clear accumulated mouse movement
		return;
	}


//
// F1-F7/ESC to enter control panel
//
	if (
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		scan == sc_F9)
#else
#ifndef DEBCHECK
		scan == sc_F10 ||
#endif
		scan == sc_F9 ||
		scan == sc_F7 ||
		scan == sc_F8)			// pop up quit dialog
#endif
	{
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		PM_UnlockMainMem ();
		StopMusic ();
		SD_StopSound ();
		SD_StopDigitized ();
		if (++screenpage == 3)
			screenpage = 0;
		bufferofs = screenloc[screenpage];
#else
		ClearMemory ();
		ClearSplitVWB ();
#endif
		VW_ScreenToScreen (displayofs,bufferofs,80,160);
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		loadedgame = false;
#endif
		US_ControlPanel(scan);

		 // *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		 for (i=0;i<3;i++)
		 {
			 bufferofs = screenloc[i];
	#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			 DrawPlayBorder ();
	#else
			 DrawPlayBorderSides ();
	#endif
		 }
#else
		 DrawAllPlayBorderSides ();
#endif

		// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if ((scan == sc_F9) && loadedgame)
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
		if (scan == sc_F9)
#endif
		  StartMusic ();

		PM_CheckMainMem ();
		SETFONTCOLOR(0,15);
		IN_ClearKeysDown();
		return;
	}

	if ( (scan >= sc_F1 && scan <= sc_F9) || scan == sc_Escape)
	{
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		PM_UnlockMainMem ();
#endif
		StopMusic ();
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		SD_StopDigitized ();
		SD_StopSound ();
#else
		ClearMemory ();
#endif
		VW_FadeOut ();
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		ResetSplitScreen ();
#endif

		US_ControlPanel(scan);

		SETFONTCOLOR(0,15);
		IN_ClearKeysDown();
		DrawPlayScreen ();
		if (!startgame && !loadedgame)
		{
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if (!mapmode)
#endif
				VW_FadeIn ();
			StartMusic ();
		}
		if (loadedgame)
			playstate = ex_abort;
		lasttimecount = TimeCount;
		if (MousePresent)
			Mouse(MDelta);	// Clear accumulated mouse movement
		PM_CheckMainMem ();
		return;
	}

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!mapmode && Keyboard[sc_Tab])
	{
		AutoMap();
		return;
	}
#endif
//
// TAB-? debug keys
//
	// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (!mapmode && Keyboard[sc_Tilde] && DebugOk)
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (Keyboard[sc_F10])
#else
	if (Keyboard[sc_Tab] && DebugOk)
#endif
	{
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		CA_CacheGrChunk (STARTFONT);
		fontnumber=0;
		SETFONTCOLOR(0,15);
#endif
		DebugKeys();
		if (MousePresent)
			Mouse(MDelta);	// Clear accumulated mouse movement
		lasttimecount = TimeCount;
		return;
	}

}


//===========================================================================

/*
#############################################################################

				  The objlist data structure

#############################################################################

objlist containt structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawn another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/


/*
=========================
=
= InitActorList
=
= Call to clear out the actor object lists returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
id0_int_t	objcount;
#endif

void InitActorList (void)
{
	id0_int_t	i;

//
// init the actor lists
//
	for (i=0;i<MAXACTORS;i++)
	{
		objlist[i].prev = &objlist[i+1];
		objlist[i].next = NULL;
	}

	objlist[MAXACTORS-1].prev = NULL;

	objfreelist = &objlist[0];
	lastobj = NULL;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	objcount = 0;
#endif

//
// give the player the first free spots
//
	GetNewActor ();
	player = new;

}

//===========================================================================

/*
=========================
=
= GetNewActor
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out ot
= return a dummy object pointer that will never get used
=
=========================
*/

void GetNewActor (void)
{
	if (!objfreelist)
		Quit ("GetNewActor: No free spots in objlist!");

	new = objfreelist;
	objfreelist = new->prev;
	memset (new,0,sizeof(*new));

	if (lastobj)
		lastobj->next = new;
	new->prev = lastobj;	// new->next is allready NULL from memset

	new->active = false;
	lastobj = new;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	objcount++;
#endif
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

void RemoveObj (objtype *gone)
{
	objtype **spotat;

	if (gone == player)
		Quit ("RemoveObj: Tried to remove the player!");

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	gone->state = NULL;
#endif

//
// fix the next object's back link
//
	if (gone == lastobj)
		lastobj = (objtype *)gone->prev;
	else
		gone->next->prev = gone->prev;

//
// fix the previous object's forward link
//
	gone->prev->next = gone->next;

//
// add it back in to the free list
//
	gone->prev = objfreelist;
	objfreelist = gone;

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	objcount--;
#endif
}

/*
=============================================================================

						MUSIC STUFF

=============================================================================
*/


/*
=================
=
= StopMusic
=
=================
*/

void StopMusic(void)
{
	id0_int_t	i;

	SD_MusicOff();
	for (i = 0;i < LASTMUSIC;i++)
		if (audiosegs[STARTMUSIC + i])
		{
			MM_SetPurge(&((memptr)audiosegs[STARTMUSIC + i]),3);
			MM_SetLock(&((memptr)audiosegs[STARTMUSIC + i]),false);
		}
}

//==========================================================================


/*
=================
=
= StartMusic
=
=================
*/

void StartMusic(void)
{
	musicnames	chunk;

	SD_MusicOff();
	// *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	chunk = GETTHEM_MUS;
#elif (defined GAMEVER_NOAH3D)
	chunk = songs[gamestate.mapon];
#else
	chunk = songs[gamestate.mapon+gamestate.episode*10];
#endif

//	if ((chunk == -1) || (MusicMode != smm_AdLib))
//DEBUG control panel		return;

	MM_BombOnError (false);
	CA_CacheAudioChunk(STARTMUSIC + chunk);
	MM_BombOnError (true);
	if (mmerror)
		mmerror = false;
	else
	{
		MM_SetLock(&((memptr)audiosegs[STARTMUSIC + chunk]),true);
		SD_StartMusic((MusicGroup id0_far *)audiosegs[STARTMUSIC + chunk]);
	}
}

/*** ALPHA RESTORATION ***/
// Earlier versions of the functions found in WL_INTER.C for v1.0+.
// A few notes:
// - PreloadUpdate is similar. Differences are, the first value of "w"
// is *not* casted to id0_long_t in the calculation of the new value of "w",
// and in case the latter is nonzero, VWB_Bar is called once with the
// color SECONDCOLOR (commented out in the released sources).
// There is also a check for the Escape key
// (also commented out in the later sources).
// - PreloadGraphics is different (smaller).

#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
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


	VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w,2,BLACK);
	w = (w * current) / total;
	if (w)
	{
	 VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w,2,SECONDCOLOR);

	}
	VW_UpdateScreen();
	if (LastScan == sc_Escape)
	{
		IN_ClearKeysDown();
		return(true);
	}
	else
		return(false);
}

void PreloadGraphics(void)
{
	CenterWindow (10,4);
	PrintY += 5;
	US_CPrint("Preloading\nCache\n");
	VW_UpdateScreen();

	PM_Preload (PreloadUpdate);
}
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312


/*
=============================================================================

					PALETTE SHIFTING STUFF

=============================================================================
*/

#define NUMREDSHIFTS	6
#define REDSTEPS		8

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
#define NUMWHITESHIFTS	4
#define WHITESTEPS		14
#define WHITETICS		4
#else
#define NUMWHITESHIFTS	3
#define WHITESTEPS		20
#define WHITETICS		6
#endif


id0_byte_t	id0_far redshifts[NUMREDSHIFTS][768];
id0_byte_t	id0_far whiteshifts[NUMREDSHIFTS][768];

id0_int_t		damagecount,bonuscount;
id0_boolean_t	palshifted;

extern 	id0_byte_t	id0_far	gamepal;

/*
=====================
=
= InitRedShifts
=
=====================
*/

void InitRedShifts (void)
{
	id0_byte_t	id0_far *workptr, id0_far *baseptr;
	id0_int_t		i,j,delta;


//
// fade through intermediate frames
//
	for (i=1;i<=NUMREDSHIFTS;i++)
	{
		workptr = (id0_byte_t id0_far *)&redshifts[i-1][0];
		baseptr = &gamepal;

		for (j=0;j<=255;j++)
		{
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			delta = 32-*baseptr;
#else
			delta = 64-*baseptr;
#endif
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
			delta = -*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			delta = 64-*baseptr;
#else
			delta = -*baseptr;
#endif
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
		}
	}

	for (i=1;i<=NUMWHITESHIFTS;i++)
	{
		workptr = (id0_byte_t id0_far *)&whiteshifts[i-1][0];
		baseptr = &gamepal;

		for (j=0;j<=255;j++)
		{
			delta = 64-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			delta = 64-*baseptr;
#else
			delta = 62-*baseptr;
#endif
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
			delta = 0-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
		}
	}
}


/*
=====================
=
= ClearPaletteShifts
=
=====================
*/

void ClearPaletteShifts (void)
{
	bonuscount = damagecount = 0;
}


/*
=====================
=
= StartBonusFlash
=
=====================
*/

void StartBonusFlash (void)
{
	bonuscount = NUMWHITESHIFTS*WHITETICS;		// white shift palette
}


/*
=====================
=
= StartDamageFlash
=
=====================
*/

void StartDamageFlash (id0_int_t damage)
{
	damagecount += damage;
}


/*
=====================
=
= UpdatePaletteShifts
=
=====================
*/

void UpdatePaletteShifts (void)
{
	id0_int_t	red,white;

	if (bonuscount)
	{
		white = bonuscount/WHITETICS +1;
		if (white>NUMWHITESHIFTS)
			white = NUMWHITESHIFTS;
		bonuscount -= tics;
		if (bonuscount < 0)
			bonuscount = 0;
	}
	else
		white = 0;


	if (damagecount)
	{
		red = damagecount/10 +1;
		if (red>NUMREDSHIFTS)
			red = NUMREDSHIFTS;

		damagecount -= tics;
		if (damagecount < 0)
			damagecount = 0;
	}
	else
		red = 0;

	if (red)
	{
		VW_WaitVBL(1);
		VL_SetPalette (redshifts[red-1]);
		palshifted = true;
	}
	else if (white)
	{
		VW_WaitVBL(1);
		VL_SetPalette (whiteshifts[white-1]);
		palshifted = true;
	}
	else if (palshifted)
	{
		VW_WaitVBL(1);
		VL_SetPalette (&gamepal);		// back to normal
		palshifted = false;
	}
}


/*
=====================
=
= FinishPaletteShifts
=
= Resets palette to normal if needed
=
=====================
*/

void FinishPaletteShifts (void)
{
	if (palshifted)
	{
		palshifted = 0;
		VW_WaitVBL(1);
		VL_SetPalette (&gamepal);
	}
}


/*
=============================================================================

						CORE PLAYLOOP

=============================================================================
*/


/*
=====================
=
= DoActor
=
=====================
*/

void DoActor (objtype *ob)
{
	void (*think)(objtype *);

	if (!ob->active && !areabyplayer[ob->areanumber])
		return;

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (ob != player)
#else
	if (!(ob->flags&(FL_NONMARK|FL_NEVERMARK)) )
#endif
		actorat[ob->tilex][ob->tiley] = NULL;

//
// non transitional object
//

	if (!ob->ticcount)
	{
		think =	ob->state->think;
		if (think)
		{
			think (ob);
			if (!ob->state)
			{
				RemoveObj (ob);
				return;
			}
		}

		// *** PRE-V1.4 APOGEE RESTORATION *** - Including special case for v1.0
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		if (ob != player)
			actorat[ob->tilex][ob->tiley] = ob;
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		if (!(ob->flags&FL_NEVERMARK))
			if ( !((ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley]))
				actorat[ob->tilex][ob->tiley] = ob;
#else
		if (ob->flags&FL_NEVERMARK)
			return;

		if ( (ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley])
			return;

		actorat[ob->tilex][ob->tiley] = ob;
#endif
		return;
	}

//
// transitional object
//
	ob->ticcount-=tics;
	while ( ob->ticcount <= 0)
	{
		think = ob->state->action;			// end of state action
		if (think)
		{
			think (ob);
			if (!ob->state)
			{
				RemoveObj (ob);
				return;
			}
		}

		ob->state = ob->state->next;

		if (!ob->state)
		{
			RemoveObj (ob);
			return;
		}

		if (!ob->state->tictime)
		{
			ob->ticcount = 0;
			goto think;
		}

		ob->ticcount += ob->state->tictime;
	}

think:
	//
	// think
	//
	think =	ob->state->think;
	if (think)
	{
		think (ob);
		if (!ob->state)
		{
			RemoveObj (ob);
			return;
		}
	}

	// *** PRE-V1.4 APOGEE RESTORATION *** - Including special case for v1.0
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (ob != player)
		actorat[ob->tilex][ob->tiley] = ob;
	return;
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	if (!(ob->flags&FL_NEVERMARK))
		if ( !((ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley]))
			actorat[ob->tilex][ob->tiley] = ob;
	return;
#else
	if (ob->flags&FL_NEVERMARK)
		return;

	if ( (ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley])
		return;

	actorat[ob->tilex][ob->tiley] = ob;
#endif
}

//==========================================================================


/*
===================
=
= PlayLoop
=
===================
*/

// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
id0_long_t funnyticount;
#endif


void PlayLoop (void)
{
	id0_int_t		give;
	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
	id0_int_t	helmetangle;
#endif

	playstate = TimeCount = lasttimecount = 0;
	frameon = 0;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	running = false;
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	pwallstate = anglefrac = 0;
	facecount = 0;
#else
	anglefrac = 0;
	facecount = 0;
	funnyticount = 0;
#endif
	memset (buttonstate,0,sizeof(buttonstate));
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	ClearPaletteShifts ();

	if (MousePresent)
		Mouse(MDelta);	// Clear accumulated mouse movement

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	tics = 1;
#endif
	if (demoplayback)
		IN_StartAck ();

	do
	{
		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
		if (virtualreality)
		{
			helmetangle = peek (0x40,0xf0);
			player->angle += helmetangle;
			if (player->angle >= ANGLES)
				player->angle -= ANGLES;
		}
#endif


		PollControls();

//
// actor thinking
//
		madenoise = false;

		MoveDoors ();
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		MovePWalls ();
#endif

		for (obj = player;obj;obj = obj->next)
			DoActor (obj);

		UpdatePaletteShifts ();

		ThreeDRefresh ();

		//
		// MAKE FUNNY FACE IF BJ DOESN'T MOVE FOR AWHILE
		//
		#ifdef SPEAR
		funnyticount += tics;
		if (funnyticount > 30l*70)
		{
			funnyticount = 0;
			StatusDrawPic (17,4,BJWAITING1PIC+(US_RndT()&1));
			facecount = 0;
		}
		#endif

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		gamestate.TimeCount+=tics;
#endif

		SD_Poll ();
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		UpdateSoundLoc();	// JAB
#endif

		if (screenfaded)
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		{
			screenfaded = false,
			VW_FadeIn ();
		}
#else
			VW_FadeIn ();
#endif

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		CheckKeys(false);
#else
		CheckKeys();
#endif

		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		if (demoplayback)
		{
			if (demoptr == lastdemoptr)
				playstate = ex_demodone;
		}
#endif

//
// debug aids
//
		if (singlestep)
		{
			VW_WaitVBL(14);
			lasttimecount = TimeCount;
		}
		if (extravbls)
			VW_WaitVBL(extravbls);

		if (demoplayback)
		{
			if (IN_CheckAck ())
			{
				IN_ClearKeysDown ();
				playstate = ex_abort;
			}
		}


		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
		if (virtualreality)
		{
			player->angle -= helmetangle;
			if (player->angle < 0)
				player->angle += ANGLES;
		}
#endif

	}while (!playstate && !startgame);

	if (playstate != ex_died)
		FinishPaletteShifts ();
}

