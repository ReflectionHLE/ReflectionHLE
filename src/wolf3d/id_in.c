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

//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

//
//	This module handles dealing with the various input devices
//
//	Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//				User Mgr (for command line parms)
//
//	Globals:
//		LastScan - The keyboard scan code of the last key pressed
//		LastASCII - The ASCII value of the last key pressed
//	DEBUG - there are more globals
//

#include "id_heads.h"
//#pragma	hdrstop

REFKEEN_NS_B

#define	KeyInt		9	// The keyboard ISR number

//
// mouse constants
//
#define	MReset		0
#define	MButtons	3
#define	MDelta		11

#define	MouseInt	0x33
#define	Mouse(x)	_AX = x,geninterrupt(MouseInt)

//
// joystick constants
//
#define	JoyScaleMax		32768
#define	JoyScaleShift	8
#define	MaxJoyValue		5000

/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
id0_boolean_t			MousePresent;
id0_boolean_t			JoysPresent[MaxJoys];
id0_boolean_t			JoyPadPresent;


// 	Global variables
		id0_boolean_t		Keyboard[NumCodes];
		id0_boolean_t		Paused;
		id0_char_t		LastASCII;
		ScanCode	LastScan;

		KeyboardDef	KbdDefs = {0x1d,0x38,0x47,0x48,0x49,0x4b,0x4d,0x4f,0x50,0x51};
		JoystickDef	JoyDefs[MaxJoys];
		ControlType	Controls[MaxPlayers];

		id0_longword_t	MouseDownCount;

		Demo		DemoMode = demo_Off;
		id0_byte_t id0_seg	*DemoBuffer;
		id0_word_t		DemoOffset,DemoSize;

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/
static	id0_byte_t        id0_far ASCIINames[] =		// Unshifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					},
					id0_far ShiftNames[] =		// Shifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,	// 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',	// 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',	// 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					id0_far SpecialNames[] =	// ASCII for 0xe0 prefixed codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
	0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					};


static	id0_boolean_t		IN_Started;
static	id0_boolean_t		CapsLock;
static	ScanCode	CurCode,LastCode;

static	Direction	DirTable[] =		// Quick lookup for total direction
					{
						dir_NorthWest,	dir_North,	dir_NorthEast,
						dir_West,		dir_None,	dir_East,
						dir_SouthWest,	dir_South,	dir_SouthEast
					};

static	void			(*INL_KeyHook)(void);
//static	void interrupt	(*OldKeyVect)(void);

// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
static	const id0_char_t			*ParmStrings[] = {"NOJOY","NOJOYS","NOMOUSE",id0_nil_t};
#else
static	const id0_char_t			*ParmStrings[] = {"nojoys","nomouse",id0_nil_t};
#endif

//	Internal routines

///////////////////////////////////////////////////////////////////////////
//
//	INL_KeyService() - Handles a keyboard interrupt (key up/down)
//
///////////////////////////////////////////////////////////////////////////
static void
INL_KeyService(id0_byte_t k)
{
	// NOTE: The original signature of the function is static void(void),
	// but we get the scancode as an argument rather than via inportb now
	// (and there's no need to clear the key)

	static id0_boolean_t special;
	id0_byte_t c;
#if 0
	k = inportb(0x60);	// Get the scan code

	// Tell the XT keyboard controller to clear the key
	outportb(0x61,(temp = inportb(0x61)) | 0x80);
	outportb(0x61,temp);
#endif

	if (k == 0xe0)		// Special key prefix
		special = true;
	else if (k == 0xe1)	// Handle Pause key
		Paused = true;
	else
	{
		if (k & 0x80)	// Break code
		{
			k &= 0x7f;

// DEBUG - handle special keys: ctl-alt-delete, print scrn

			Keyboard[k] = false;
		}
		else			// Make code
		{
			LastCode = CurCode;
			CurCode = LastScan = k;
			Keyboard[k] = true;

			if (special)
				c = SpecialNames[k];
			else
			{
				if (k == sc_CapsLock)
				{
					CapsLock ^= true;
					// DEBUG - make caps lock light work
				}

				if (Keyboard[sc_LShift] || Keyboard[sc_RShift])	// If shifted
				{
					c = ShiftNames[k];
					if ((c >= 'A') && (c <= 'Z') && CapsLock)
						c += 'a' - 'A';
				}
				else
				{
					c = ASCIINames[k];
					if ((c >= 'a') && (c <= 'z') && CapsLock)
						c -= 'a' - 'A';
				}
			}
			if (c)
				LastASCII = c;
		}

		special = false;
	}

	if (INL_KeyHook && !special)
		INL_KeyHook();
	//outportb(0x20,0x20);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void
INL_GetMouseDelta(id0_int_t *x,id0_int_t *y)
{
	BE_ST_GetEmuAccuMouseMotion(x, y);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static id0_word_t
INL_GetMouseButtons(void)
{
	return BE_ST_GetEmuMouseButtons();
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyAbs() - Reads the absolute position of the specified joystick
//
///////////////////////////////////////////////////////////////////////////
void
IN_GetJoyAbs(id0_word_t joy,id0_word_t *xp,id0_word_t *yp)
{
	BE_ST_GetEmuJoyAxes(joy, xp, yp);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void INL_GetJoyDelta(id0_word_t joy,id0_int_t *dx,id0_int_t *dy)
{
	id0_word_t		x,y;
	id0_longword_t	time;
	JoystickDef	*def;
static	id0_longword_t	lasttime;

	IN_GetJoyAbs(joy,&x,&y);
	def = JoyDefs + joy;

	if (x < def->threshMinX)
	{
		if (x < def->joyMinX)
			x = def->joyMinX;

		x = -(x - def->threshMinX);
		x *= def->joyMultXL;
		x >>= JoyScaleShift;
		*dx = (x > 127)? -127 : -x;
	}
	else if (x > def->threshMaxX)
	{
		if (x > def->joyMaxX)
			x = def->joyMaxX;

		x = x - def->threshMaxX;
		x *= def->joyMultXH;
		x >>= JoyScaleShift;
		*dx = (x > 127)? 127 : x;
	}
	else
		*dx = 0;

	if (y < def->threshMinY)
	{
		if (y < def->joyMinY)
			y = def->joyMinY;

		y = -(y - def->threshMinY);
		y *= def->joyMultYL;
		y >>= JoyScaleShift;
		*dy = (y > 127)? -127 : -y;
	}
	else if (y > def->threshMaxY)
	{
		if (y > def->joyMaxY)
			y = def->joyMaxY;

		y = y - def->threshMaxY;
		y *= def->joyMultYH;
		y >>= JoyScaleShift;
		*dy = (y > 127)? 127 : y;
	}
	else
		*dy = 0;

	lasttime = SD_GetTimeCount();
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyButtons() - Returns the button status of the specified
//		joystick
//
///////////////////////////////////////////////////////////////////////////
static id0_word_t
INL_GetJoyButtons(id0_word_t joy)
{
	return BE_ST_GetEmuJoyButtons(joy);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyButtonsDB() - Returns the de-bounced button status of the
//		specified joystick
//
///////////////////////////////////////////////////////////////////////////
id0_word_t
IN_GetJoyButtonsDB(id0_word_t joy)
{
	id0_longword_t	lasttime;
	id0_word_t		result1,result2;

	do
	{
		result1 = INL_GetJoyButtons(joy);
		lasttime = TimeCount;
		SD_TimeCountWaitFromSrc(lasttime, 1);
#if 0
		while (TimeCount == lasttime)
			;
#endif
		result2 = INL_GetJoyButtons(joy);
		BE_ST_ShortSleep();
	} while (result1 != result2);
	return(result1);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
static void
INL_StartKbd(void)
{
	INL_KeyHook = NULL;			// no key hook routine

	IN_ClearKeysDown();

	//OldKeyVect = getvect(KeyInt);
	BE_ST_StartKeyboardService(&INL_KeyService);
	//setvect(KeyInt,INL_KeyService);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutKbd(void)
{
	//poke(0x40,0x17,peek(0x40,0x17) & 0xfaf0);	// Clear ctrl/alt/shift flags

	BE_ST_StopKeyboardService();
	//setvect(KeyInt,OldKeyVect);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
static id0_boolean_t
INL_StartMouse(void)
{
	// TODO (REFKEEN): Consider optionally returning false?
	BE_ST_ResetEmuMouse();
	return(true);
#if 0
	if (getvect(MouseInt))
	{
		Mouse(MReset);
		if (_AX == 0xffff)
			return(true);
	}
	return(false);
#endif
#if 0
 union REGS regs;
 id0_unsigned_char_t id0_far *vector;


 if ((vector=MK_FP(peek(0,0x33*4+2),peek(0,0x33*4)))==NULL)
   return false;

 if (*vector == 207)
   return false;

 Mouse(MReset);
 return true;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutMouse() - Cleans up after the mouse
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutMouse(void)
{
}

//
//	INL_SetJoyScale() - Sets up scaling values for the specified joystick
//
static void
INL_SetJoyScale(id0_word_t joy)
{
	JoystickDef	*def;

	def = &JoyDefs[joy];
	def->joyMultXL = JoyScaleMax / (def->threshMinX - def->joyMinX);
	def->joyMultXH = JoyScaleMax / (def->joyMaxX - def->threshMaxX);
	def->joyMultYL = JoyScaleMax / (def->threshMinY - def->joyMinY);
	def->joyMultYH = JoyScaleMax / (def->joyMaxY - def->threshMaxY);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetupJoy() - Sets up thresholding values and calls INL_SetJoyScale()
//		to set up scaling values
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetupJoy(id0_word_t joy,id0_word_t minx,id0_word_t maxx,id0_word_t miny,id0_word_t maxy)
{
	id0_word_t		d,r;
	JoystickDef	*def;

	def = &JoyDefs[joy];

	def->joyMinX = minx;
	def->joyMaxX = maxx;
	r = maxx - minx;
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	d = r / 5;
#else
	d = r / 3;
#endif
	def->threshMinX = ((r / 2) - d) + minx;
	def->threshMaxX = ((r / 2) + d) + minx;

	def->joyMinY = miny;
	def->joyMaxY = maxy;
	r = maxy - miny;
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	d = r / 5;
#else
	d = r / 3;
#endif
	def->threshMinY = ((r / 2) - d) + miny;
	def->threshMaxY = ((r / 2) + d) + miny;

	INL_SetJoyScale(joy);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartJoy() - Detects & auto-configures the specified joystick
//					The auto-config assumes the joystick is centered
//
///////////////////////////////////////////////////////////////////////////
static id0_boolean_t
INL_StartJoy(id0_word_t joy)
{
	id0_word_t		x,y;

	IN_GetJoyAbs(joy,&x,&y);

	if
	(
		((x == 0) || (x > MaxJoyValue - 10))
	||	((y == 0) || (y > MaxJoyValue - 10))
	)
		return(false);
	else
	{
		IN_SetupJoy(joy,0,x * 2,0,y * 2);
		return(true);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutJoy() - Cleans up the joystick stuff
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutJoy(id0_word_t joy)
{
	JoysPresent[joy] = false;
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	id0_boolean_t	checkjoys,checkmouse;
	id0_word_t	i;

	if (IN_Started)
		return;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	BE_ST_printf("IN_Startup: ");
#endif

	checkjoys = true;
	checkmouse = true;
	for (i = 1;i < id0_argc;i++)
	{
		switch (US_CheckParm(id0_argv[i],ParmStrings))
		{
		case 0:
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		case 1:
			BE_ST_printf("Skipping joystick detection\n");
#endif
			checkjoys = false;
			break;
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		case 2:
			BE_ST_printf("Skipping mouse detection\n");
#else
		case 1:
#endif
			checkmouse = false;
			break;
		}
	}

	INL_StartKbd();
	MousePresent = checkmouse? INL_StartMouse() : false;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (MousePresent)
		BE_ST_printf("Mouse detected\n");
#endif
	// REFKEEN - Alternative controllers support
	UpdateAltControllerMappingsByMousePresence(MousePresent);
	//

	for (i = 0;i < MaxJoys;i++)
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	{
#endif
		JoysPresent[i] = checkjoys? INL_StartJoy(i) : false;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (JoysPresent[i])
			BE_ST_printf("Joystick %" ID0_PRId " detected\n", i+1);
	}
#endif

	IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Default() - Sets up default conditions for the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Default(id0_boolean_t gotit,ControlType in)
{
	if
	(
		(!gotit)
	|| 	((in == ctrl_Joystick1) && !JoysPresent[0])
	|| 	((in == ctrl_Joystick2) && !JoysPresent[1])
	|| 	((in == ctrl_Mouse) && !MousePresent)
	)
		in = ctrl_Keyboard1;
	IN_SetControlType(0,in);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Shutdown(void)
{
	id0_word_t	i;

	if (!IN_Started)
		return;

	INL_ShutMouse();
	for (i = 0;i < MaxJoys;i++)
		INL_ShutJoy(i);
	INL_ShutKbd();

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetKeyHook() - Sets the routine that gets called by INL_KeyService()
//			everytime a real make/break code gets hit
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetKeyHook(void (*hook)())
{
	INL_KeyHook = hook;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void
IN_ClearKeysDown(void)
{
	id0_int_t	i;

	LastScan = sc_None;
	LastASCII = key_None;
	memset (Keyboard,0,sizeof(Keyboard));
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadControl(id0_int_t player,ControlInfo *info)
{
			id0_boolean_t		realdelta;
			id0_byte_t		dbyte;
			id0_word_t		buttons;
			id0_int_t			dx,dy;
			Motion		mx,my;
			ControlType	type;
register	KeyboardDef	*def;

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

	if (DemoMode == demo_Playback)
	{
		dbyte = DemoBuffer[DemoOffset + 1];
		// REFKEEN - Let's hope these casts (for C++) are OK..
		my = (Motion)((dbyte & 3) - 1);
		mx = (Motion)(((dbyte >> 2) & 3) - 1);
		buttons = (dbyte >> 4) & 3;

		if (!(--DemoBuffer[DemoOffset]))
		{
			DemoOffset += 2;
			if (DemoOffset >= DemoSize)
				DemoMode = demo_PlayDone;
		}

		realdelta = false;
	}
	else if (DemoMode == demo_PlayDone)
		Quit("Demo playback exceeded");
	else
	{
		switch (type = Controls[player])
		{
		case ctrl_Keyboard:
			def = &KbdDefs;

			if (Keyboard[def->upleft])
				mx = motion_Left,my = motion_Up;
			else if (Keyboard[def->upright])
				mx = motion_Right,my = motion_Up;
			else if (Keyboard[def->downleft])
				mx = motion_Left,my = motion_Down;
			else if (Keyboard[def->downright])
				mx = motion_Right,my = motion_Down;

			if (Keyboard[def->up])
				my = motion_Up;
			else if (Keyboard[def->down])
				my = motion_Down;

			if (Keyboard[def->left])
				mx = motion_Left;
			else if (Keyboard[def->right])
				mx = motion_Right;

			if (Keyboard[def->button0])
				buttons += 1 << 0;
			if (Keyboard[def->button1])
				buttons += 1 << 1;
			realdelta = false;
			break;
		case ctrl_Joystick1:
		case ctrl_Joystick2:
			INL_GetJoyDelta(type - ctrl_Joystick,&dx,&dy);
			buttons = INL_GetJoyButtons(type - ctrl_Joystick);
			realdelta = true;
			break;
		case ctrl_Mouse:
			INL_GetMouseDelta(&dx,&dy);
			buttons = INL_GetMouseButtons();
			realdelta = true;
			break;
		}
	}

	if (realdelta)
	{
		mx = (dx < 0)? motion_Left : ((dx > 0)? motion_Right : motion_None);
		my = (dy < 0)? motion_Up : ((dy > 0)? motion_Down : motion_None);
	}
	else
	{
		dx = mx * 127;
		dy = my * 127;
	}

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = buttons & (1 << 0);
	info->button1 = buttons & (1 << 1);
	info->button2 = buttons & (1 << 2);
	info->button3 = buttons & (1 << 3);
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];

	if (DemoMode == demo_Record)
	{
		// Pack the control info into a byte
		dbyte = (buttons << 4) | ((mx + 1) << 2) | (my + 1);

		if
		(
			(DemoBuffer[DemoOffset + 1] == dbyte)
		&&	(DemoBuffer[DemoOffset] < 255)
		)
			(DemoBuffer[DemoOffset])++;
		else
		{
			if (DemoOffset || DemoBuffer[DemoOffset])
				DemoOffset += 2;

			if (DemoOffset >= DemoSize)
				Quit("Demo buffer overflow");

			DemoBuffer[DemoOffset] = 1;
			DemoBuffer[DemoOffset + 1] = dbyte;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetControlType() - Sets the control type to be used by the specified
//		player
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetControlType(id0_int_t player,ControlType type)
{
	// DEBUG - check that requested type is present?
	Controls[player] = type;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForKey() - Waits for a scan code, then clears LastScan and
//		returns the scan code
//
///////////////////////////////////////////////////////////////////////////
ScanCode
IN_WaitForKey(void)
{
	ScanCode	result;

	while (!(result = LastScan))
		BE_ST_ShortSleep();
	LastScan = 0;
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForASCII() - Waits for an ASCII char, then clears LastASCII and
//		returns the ASCII value
//
///////////////////////////////////////////////////////////////////////////
id0_char_t
IN_WaitForASCII(void)
{
	id0_char_t		result;

	while (!(result = LastASCII))
		BE_ST_ShortSleep();
	LastASCII = '\0';
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

id0_boolean_t	btnstate[8];

void IN_StartAck(void)
{
	id0_unsigned_t	i,buttons;

//
// get initial state of everything
//
	IN_ClearKeysDown();
	memset (btnstate,0,sizeof(btnstate));

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if (buttons&1)
			btnstate[i] = true;
}


id0_boolean_t IN_CheckAck (void)
{
	id0_unsigned_t	i,buttons;

//
// see if something has been pressed
//
	if (LastScan)
		return true;

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if ( buttons&1 )
		{
			if (!btnstate[i])
				return true;
		}
		else
			btnstate[i]=false;

	return false;
}


void IN_Ack (void)
{
	// REFKEEN - Alternative controllers support
	extern BE_ST_ControllerMapping g_ingame_altcontrol_mapping_inackback;
	BE_ST_AltControlScheme_Push();
	BE_ST_AltControlScheme_PrepareControllerMapping(&g_ingame_altcontrol_mapping_inackback);

	IN_StartAck ();

	while (!IN_CheckAck ())
		BE_ST_ShortSleep();
	// REFKEEN - Alternative controllers support
	BE_ST_AltControlScheme_Pop();
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
id0_boolean_t IN_UserInput(id0_longword_t delay)
{
	id0_longword_t	lasttime;

	lasttime = SD_GetTimeCount();
	IN_StartAck ();
	do
	{
		BE_ST_ShortSleep();
		if (IN_CheckAck())
			return true;
	} while (SD_GetTimeCount() - lasttime < delay);
	return(false);
}

//===========================================================================

/*
===================
=
= IN_MouseButtons
=
===================
*/

id0_byte_t	IN_MouseButtons (void)
{
	// REFKEEN: This is almost a clone of INL_GetMouseButtons
	return MousePresent ? BE_ST_GetEmuMouseButtons() : 0;
}


/*
===================
=
= IN_JoyButtons
=
===================
*/

id0_byte_t	IN_JoyButtons (void)
{
	return BE_ST_GetEmuJoyButtons(0) | (BE_ST_GetEmuJoyButtons(1) << 2);
}

REFKEEN_NS_E
