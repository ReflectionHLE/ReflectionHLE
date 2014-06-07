/* Catacomb Abyss Source Code
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

#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <dir.h>
#include "mem.h"
#include "string.h"
#include "time.h"
#include "stdarg.h"
#include "io.h"

#include "DEF.H"
#include "gelib.h"

#define MAX_GAMELIST_NAMES 20
#define FNAME_LEN				9

////////////////////////////////////////////////////////////////////////////
//
// Global variables
//
boolean InLoadSaveGame = false;
//AudioDeviceType ge_DigiMode;
boolean ConserveMemory = false;
char GameListNames[MAX_GAMELIST_NAMES+1][FNAME_LEN],current_disk=1;
short NumGames;
short PPT_LeftEdge=0,PPT_RightEdge=320;
boolean LeaveDriveOn=false,ge_textmode=true;
char Filename[FILENAME_LEN+1], ID[sizeof(GAMENAME)], VER[sizeof(SAVEVER_DATA)];


////////////////////////////////////////////////////////////////////////////
//
// CalibrateJoystick()
//
void CalibrateJoystick(short joynum)
{
	word	minx,maxx,
			miny,maxy;

	IN_ClearKeysDown();

	VW_HideCursor();

	VW_FixRefreshBuffer();
	CenterWindow(30,8);

	US_Print("\n");
	US_CPrintLine("Move joystick to the upper-left");
	US_CPrintLine("and press one of the buttons.");
	VW_UpdateScreen();

	while ((LastScan != sc_Escape) && !IN_GetJoyButtonsDB(joynum));
	if (LastScan == sc_Escape)
		return;

	IN_GetJoyAbs(joynum,&minx,&miny);
	while (IN_GetJoyButtonsDB(joynum));

	US_Print("\n");
	US_CPrintLine("Move joystick to the lower-right");
	US_CPrintLine("and press one of the buttons.");
	VW_UpdateScreen();

	while ((LastScan != sc_Escape) && !IN_GetJoyButtonsDB(joynum));
	if (LastScan == sc_Escape)
		return;

	IN_GetJoyAbs(joynum,&maxx,&maxy);
	if ((minx == maxx) && (miny == maxy))
		return;

	IN_SetupJoy(joynum,minx,maxx,miny,maxy);

	while (IN_GetJoyButtonsDB(joynum));
	if (LastScan)
		IN_ClearKeysDown();

	JoystickCalibrated = true;
}

////////////////////////////////////////////////////////////////////////////
//
// WaitKeyVBL()
//
void WaitKeyVBL(short key, short vbls)
{
	while (vbls--)
	{
		VW_WaitVBL(1);
		IN_ReadControl(0,&control);
		if ((control.button0|control.button1)||(Keyboard[key]))
			break;
	}
}

////////////////////////////////////////////////////////////////////////////
//
// MoveScreen()
//
// panadjust must be saved and restored if MoveScreen is being called from
// inside a level.
//
void MoveScreen(short x, short y)
{
	unsigned address;

	address = (y*linewidth)+(x/8);
	VW_SetScreen(address,0);
	bufferofs = displayofs = address;
	panadjust=0;
}

////////////////////////////////////////////////////////////////////////////
//
// MoveGfxDst()
//
void MoveGfxDst(short x, short y)
{
	unsigned address;

	address = (y*linewidth)+(x/8);
	bufferofs = displayofs = address;
}

#if 0

#if GRAPHIC_PIRATE

///////////////////////////////////////////////////////////////////////////
//
// DoPiracy() - Graphics piracy code...
//
void DoPiracy()
{
	struct Shape Pirate1Shp;
	struct Shape Pirate2Shp;

	VW_SetScreenMode (EGA320GR);
	VW_ClearVideo(BLACK);

	// Load shapes...
	//
	if (LoadShape("PIRATE1E."EXT,&Pirate1Shp))
		TrashProg("Can't load PIRATE1E.BIO");

	if (LoadShape("PIRATE2E."EXT,&Pirate2Shp))
		TrashProg("Can't load PIRATE2E.BIO");

	// Deal with shapes...
	//
	VW_SetLineWidth(40);

	VW_FadeOut();

	MoveScreen(0,0);
	UnpackEGAShapeToScreen(&Pirate1Shp,(linewidth-Pirate1Shp.BPR)<<2,0);

	MoveScreen(0,200);
	UnpackEGAShapeToScreen(&Pirate2Shp,(linewidth-Pirate2Shp.BPR)<<2,0);

	MoveScreen(0,0);
	VW_FadeIn();
	WaitKeyVBL(57,200);
	while (Keyboard[57]);

	SD_PlaySound(GOOD_PICKSND);

	MoveScreen(0,200);
	WaitKeyVBL(57,300);
	while (Keyboard[57]);
	VW_FadeOut();

	FreeShape(&Pirate1Shp);
	FreeShape(&Pirate2Shp);
}

#else

///////////////////////////////////////////////////////////////////////////
//
// DoPiracy() - Text-based piracy code...
//
void DoPiracy()
{
}

#endif
#endif


//--------------------------------------------------------------------------
// PrintPropText()
//--------------------------------------------------------------------------

// THE FOLLOWING MUST BE INITIALIZED BEFORE CALLING THIS ROUTINE:
//

// WindowX, WindowW, PrintY - These are ID global variables and are
// automatically initialized when using their window routines.
//
// WindowX is the left edge of the window.
// WindowW is the width of the window.
// PrintY is the top edge of the window area.
//
// All values are represented in unshifted pixels.

// PPT_LeftEdge, PPT_RightEdge - These are globals used by PrintPropText().
// They define the left and right edge of the text area in pixels.

void PrintPropText(char far *text)
{
	#define RETURN_CHAR '\n'

	char pb[200];

	fontstruct _seg *font = (fontstruct _seg *)grsegs[STARTFONT];
	char savech;
	short	length,maxend,maxx,loop,curx;
	boolean centerit,lastcharcr;

	while (*text)
	{
		if (*text == '^')
			centerit=true,text++;
		else
			centerit=false;

		/* Search forward for the last possible character in the line. This
		** character is:  1) RETURN (end of line)  2) ZERO (end of buffer)
		** and  3) the character at "WP->vWidth" bytes from the start of the line.
		*/
		curx=PPT_LeftEdge;
		length=0;
		while ((curx+font->width[text[length+1]] < PPT_RightEdge) &&
				(text[length]) &&
				(text[length] != RETURN_CHAR))
						curx+=font->width[text[length++]];

		/* Search backward from the point we just found for a SPACE (for word
		** wrapping).
		*/
		if ((text[length]) && (text[length] != RETURN_CHAR))
		{
			maxx = curx;
			maxend = length;
			while ((length) && (text[length] != ' '))
				curx-=font->width[text[length--]];

			/* Were there any SPACES on this line? If not, take the MAX!
			*/
			if (!length)
				length=maxend,curx=maxx;
		}

		/* If we can, lets keep the SPACE or RETURN that follows a line at
		** the end of that line.
		*/
		if (((text[length] == ' ') || (text[length] == RETURN_CHAR)) && (length < PPT_RightEdge))
			length++;

		// All of this is kludged to work with ID _Print routines...
		//
		savech=text[length];
		text[length]=0;
		if (text[length-1] == RETURN_CHAR)
		{
			lastcharcr=true;
			text[length-1]=0;
		}
		else
			lastcharcr=false;
		_fmemcpy(pb,text,length+1);
		if (centerit)
		{
			US_CPrintLine(pb);
		}
		else
		{
			PrintX = PPT_LeftEdge;
			US_Print(pb);
			US_Print("\n");
		}
		if (lastcharcr)
			text[length-1]=RETURN_CHAR;
		text[length]=savech;
		//
		// end of ID _Print kludge...

		text += length;
	}
}

////////////////////////////////////////////////////////////////////////////
//
// DisplayText()
//
void DisplayText(textinfo *textinfo)
{
	#define PAGE_WIDTH 	78

	int loop, PageNum, LastNum,num;
	boolean InHelp = true,faded_in = false;
	unsigned holddisp,holdpan,holdbuffer,holdaddress;

// Can you believe it takes all this just to change to 640 mode!!???!
//
	VW_ScreenToScreen(0,FREESTART-STATUSLEN,40,80);
	VW_SetLineWidth(80);
	MoveScreen(0,0);
	VW_Bar (0,0,640,200,0);
	VW_SetScreenMode(EGA640GR);
	VW_SetLineWidth(80);
	BlackPalette();

// Now, let's display some text...
//
	PPT_RightEdge=PAGE_WIDTH*8;
	PPT_LeftEdge=16;
	PrintY= 30;
	WindowX=WindowY=0;

	LastNum = -1;
	PageNum = 1;
	while (InHelp)
	{
		// Display new page of text.
		//
		if (PageNum != LastNum)
		{
			US_DrawWindow(1,1,PAGE_WIDTH,23);
			PrintPropText(textinfo->pages[PageNum-1]);
			LastNum = PageNum;
		}

		VW_UpdateScreen();
		if (!faded_in)
		{
			VW_FadeIn();
			faded_in = true;
		}

		// Scroll through text / exit.
		//
		IN_ReadControl(0,&control);
		if (control.button1 || Keyboard[1])
			InHelp=false;
		else
		{
			if (ControlTypeUsed != ctrl_Keyboard)
				control.dir = dir_None;

			if (((control.dir == dir_North) || (control.dir == dir_West)) && (PageNum > 1))
			{
				PageNum--;
				while ((control.dir == dir_North) || (control.dir == dir_West))
					IN_ReadControl(0,&control);
			}
			else
				if (((control.dir == dir_South) || (control.dir == dir_East)) && (PageNum < textinfo->totalpages))
				{
					PageNum++;
					while ((control.dir == dir_South) || (control.dir == dir_East))
						IN_ReadControl(0,&control);
				}
		}
	}

	// Wait for 'exit key' to be released.
	//
	while (control.button1 || Keyboard[1])
		IN_ReadControl(0,&control);

// Can you believe it takes all this just to change to 320 mode!!???!
//
	VW_FadeOut();
	VW_SetLineWidth(40);
	MoveScreen(0,0);
	VW_Bar (0,0,320,200,0);
	VW_SetScreenMode(EGA320GR);
	BlackPalette();
	VW_ScreenToScreen(FREESTART-STATUSLEN,0,40,80);
}

//--------------------------------------------------------------------------
// BlackPalette()
//--------------------------------------------------------------------------
void BlackPalette()
{
	extern char colors[7][17];

	_ES=FP_SEG(&colors[0]);
	_DX=FP_OFF(&colors[0]);
	_AX=0x1002;
	geninterrupt(0x10);
	screenfaded = true;
}

//--------------------------------------------------------------------------
// ColoredPalette()
//--------------------------------------------------------------------------
void ColoredPalette()
{
	extern char colors[7][17];

	_ES=FP_SEG(&colors[3]);
	_DX=FP_OFF(&colors[3]);
	_AX=0x1002;
	geninterrupt(0x10);
	screenfaded = false;
}

////////////////////////////////////////////////////////////////////////////
//
// Verify()
//
long Verify(char *filename)
{
	int handle;
	long size;

	if ((handle=open(filename,O_BINARY))==-1)
		return (0);
	size=filelength(handle);
	close(handle);
	return(size);
}

///////////////////////////////////////////////////////////////////////////
//
//	GE_SaveGame
//
//	Handles user i/o for saving a game
//
///////////////////////////////////////////////////////////////////////////

void GE_SaveGame()
{
	boolean GettingFilename=true;
	char drive;
//	char Filename[FILENAME_LEN+1],drive; //, ID[sizeof(GAMENAME)], VER[sizeof(SAVEVER_DATA)];
	int handle;
	struct dfree dfree;
	long davail;

	VW_FixRefreshBuffer();
	ReadGameList();
	while (GettingFilename)
	{
		DisplayGameList(2,7,3,10);
		US_DrawWindow(5,1,30,3);
		memset(Filename,0,sizeof(Filename));
		US_CPrint("Enter name to SAVE this game:");
		VW_UpdateScreen();
		if (screenfaded)
			VW_FadeIn();
		if (!US_LineInput((linewidth<<2)-32,20,Filename,"",true,8,0))
			goto EXIT_FUNC;
		if (!strlen(Filename))
			goto EXIT_FUNC;

		drive = getdisk();
		getdfree(drive+1,&dfree);
		davail = (long)dfree.df_avail*(long)dfree.df_bsec*(long)dfree.df_sclus;

		if (davail < 10000l)
		{
			char status[40] = "\nDrive:     Free: ";

			US_CenterWindow(30,6);
			US_Print("\n");
			US_CPrintLine("Disk Full: Can't save game.");
			US_CPrintLine("Try inserting another disk.");
			status[8] = drive+'A';
			itoa(davail,&status[18],10);
			US_CPrint(status);
			VW_UpdateScreen();

			IN_Ack();
		}
		else
		{
			strcat(Filename,".SAV");
			GettingFilename = false;
			if (Verify(Filename))								// FILE EXISTS
			{
				US_CenterWindow(22,4);
				US_CPrintLine("That file already exists...");
				US_CPrintLine("Overwrite it ????");
				US_CPrintLine("(Y)es or (N)o?");
				VW_UpdateScreen();

				while((!Keyboard[21]) && (!Keyboard[49]) && !Keyboard[27]);

				if (Keyboard[27])
					goto EXIT_FUNC;
				if (Keyboard[49])
				{
					GettingFilename = true;
					VW_UpdateScreen();
				}
			}
		}
	}

	handle = open(Filename,O_RDWR|O_CREAT|O_BINARY,S_IREAD|S_IWRITE);
	if (handle==-1)
		goto EXIT_FUNC;

	if ((!CA_FarWrite(handle,(void far *)GAMENAME,sizeof(GAMENAME))) || (!CA_FarWrite(handle,(void far *)SAVEVER_DATA,sizeof(SAVEVER_DATA))))
	{
		if (!screenfaded)
			VW_FadeOut();

		return;
	}

	if (!USL_SaveGame(handle))
		Quit("Save game error");



EXIT_FUNC:;

	if (handle!=-1)
		close(handle);

	if (handle==-1)
	{
		remove(Filename);
		US_CenterWindow(22,6);
		US_CPrintLine("DISK ERROR");
		US_CPrintLine("Check: Write protect...");
		US_CPrintLine("File name...");
		US_CPrintLine("Bytes free on disk...");
		US_CPrintLine("Press SPACE to continue.");
		VW_UpdateScreen();
		while (!Keyboard[57]);
		while (Keyboard[57]);
	}

	while (Keyboard[1]);

	if (!screenfaded)
		VW_FadeOut();
}


///////////////////////////////////////////////////////////////////////////
//
//	GE_LoadGame
//
//	Handles user i/o for loading a game
//
///////////////////////////////////////////////////////////////////////////

boolean GE_LoadGame()
{
	boolean GettingFilename=true,rt_code=false;
	int handle;

	IN_ClearKeysDown();
	memset(ID,0,sizeof(ID));
	memset(VER,0,sizeof(VER));
	VW_FixRefreshBuffer();
	ReadGameList();
	while (GettingFilename)
	{
		DisplayGameList(2,7,3,10);
		US_DrawWindow(5,1,30,3);
		memset(Filename,0,sizeof(Filename));
		US_CPrint("Enter name of game to RESTORE:");
		VW_UpdateScreen();
		if (screenfaded)
			VW_FadeIn();
		if (!US_LineInput((linewidth<<2)-32,20,Filename,"",true,8,0))
			goto EXIT_FUNC;
		strcat(Filename,".SAV");
		GettingFilename = false;

		if (!Verify(Filename))								// FILE DOESN'T EXIST
		{
			US_CenterWindow(22,3);
			US_CPrintLine(" That file doesn't exist....");
			US_CPrintLine("Press SPACE to try again.");
			VW_UpdateScreen();

			while (!Keyboard[57]);
			while (Keyboard[57]);
			GettingFilename = true;
		}
	}

	handle = open(Filename,O_RDWR|O_BINARY);
	if (handle==-1)
		goto EXIT_FUNC;

	if ((!CA_FarRead(handle,(void far *)&ID,sizeof(ID))) || (!CA_FarRead(handle,(void far *)&VER,sizeof(VER))))
		return(false);

	if ((strcmp(ID,GAMENAME)) || (strcmp(VER,SAVEVER_DATA)))
	{
		US_CenterWindow(32,4);
		US_CPrintLine("That isn't a "GAMENAME);
		US_CPrintLine(".SAV file.");
		US_CPrintLine("Press SPACE to continue.");
		VW_UpdateScreen();
		while (!Keyboard[57]);
		while (Keyboard[57]);

		if (!screenfaded)
			VW_FadeOut();

		return(false);
	}

	if (!USL_LoadGame(handle))
		Quit("Load game error.");

	rt_code = true;


EXIT_FUNC:;
	if (handle==-1)
	{
		US_CenterWindow(22,3);
		US_CPrintLine("DISK ERROR ** LOAD **");
		US_CPrintLine("Press SPACE to continue.");
		while (!Keyboard[57]);
		while (Keyboard[57]);
	}
	else
		close(handle);

	if (!screenfaded)
		VW_FadeOut();

	return(rt_code);
}

///////////////////////////////////////////////////////////////////////////
//
//	GE_HardError() - Handles the Abort/Retry/Fail sort of errors passed
//			from DOS. Hard coded to ignore if during Load/Save Game.
//
///////////////////////////////////////////////////////////////////////////
#pragma	warn	-par
#pragma	warn	-rch
int GE_HardError(word errval,int ax,int bp,int si)
{
#define IGNORE  0
#define RETRY   1
#define	ABORT   2
extern	void	ShutdownId(void);

static	char		buf[32];
static	WindowRec	wr;
static	boolean		oldleavedriveon;
		int			di;
		char		c,*s,*t;
boolean holdscreenfaded;

	if (InLoadSaveGame)
		hardresume(IGNORE);


	di = _DI;

	oldleavedriveon = LeaveDriveOn;
	LeaveDriveOn = false;

	if (ax < 0)
		s = "Device Error";
	else
	{
		if ((di & 0x00ff) == 0)
			s = "Drive ~ is Write Protected";
		else
			s = "Error on Drive ~";
		for (t = buf;*s;s++,t++)	// Can't use sprintf()
			if ((*t = *s) == '~')
				*t = (ax & 0x00ff) + 'A';
		*t = '\0';
		s = buf;
	}

	c = peekb(0x40,0x49);	// Get the current screen mode
	if ((c < 4) || (c == 7))
		goto oh_kill_me;

	// DEBUG - handle screen cleanup
	holdscreenfaded=screenfaded;

	US_SaveWindow(&wr);
	VW_ClearVideo(0);            ////////////// added for exiting
	US_CenterWindow(30,3);
	US_CPrint(s);
	US_CPrint("(R)etry or (A)bort?");
	VW_UpdateScreen();
	if (holdscreenfaded)
		VW_FadeIn();
	IN_ClearKeysDown();

asm	sti	// Let the keyboard interrupts come through

	while (true)
	{
		switch (IN_WaitForASCII())
		{
		case key_Escape:
		case 'a':
		case 'A':
			goto oh_kill_me;
			break;
		case key_Return:
		case key_Space:
		case 'r':
		case 'R':
			if (holdscreenfaded)
				VW_FadeOut();
			US_ClearWindow();
			VW_UpdateScreen();
			US_RestoreWindow(&wr);
			LeaveDriveOn = oldleavedriveon;
			return(RETRY);
			break;
		}
	}

oh_kill_me:
	abortprogram = s;
	TrashProg("Terminal Error: %s\n",s);
//	if (tedlevel)
//		fprintf(stderr,"You launched from TED. I suggest that you reboot...\n");

	return(ABORT);
#undef	IGNORE
#undef	RETRY
#undef	ABORT
}
#pragma	warn	+par
#pragma	warn	+rch

//--------------------------------------------------------------------------
//
//
//                          B O B   ROUTINES
//
//
//--------------------------------------------------------------------------



#ifdef BOBLIST

////////////////////////////////////////////////////////////////////////////
//
// UpdateBOBList() - Adds a sprite to an objects BOBlist.  The BOB List
//						 	must already be allocated and have an available slot.
//
//	RETURNS : true = Success adding Sprite / false = Failure.
//
// NOTE : This also sets the users 'needtoreact' flag to true.
//
boolean UpdateBOBList(objtype *obj,struct Simple_Shape *Shape,shapeclass Class, short priority, spriteflags sprflags)
{
	struct BOB_Shape *CurBOBShape = NULL;

#pragma warn -pia

	if (CurBOBShape = obj->nextshape)
	{
		// Treverse down BOBList looking for a sprite with the same class
		// OR an empty shape struct to store the new shape.

		while ((CurBOBShape->class != Class) && (CurBOBShape->class) && CurBOBShape)
		{
			CurBOBShape = CurBOBShape->nextshape;
		}

		if (CurBOBShape)
		{
			RF_RemoveSprite(&CurBOBShape->sprite);
			CurBOBShape->shapenum = Shape->shapenum;
			CurBOBShape->x_offset = Shape->x_offset;
			CurBOBShape->y_offset = Shape->y_offset;
			CurBOBShape->priority = priority;
			CurBOBShape->sprflags = sprflags;
			CurBOBShape->class = Class;
			return(true);
		}
	}
	return(false);

#pragma warn +pia

}

/////////////////////////////////////////////////////////////////////////////
//
// RemoveBOBShape() - Removes a sprite from a BOBList.
//
// RETURNS : true = Success / false = Failure (shape not found)
//
boolean RemoveBOBShape(objtype *obj, shapeclass Class)
{
	struct BOB_Shape *CurBOBShape = NULL;

#pragma warn -pia

	if (CurBOBShape = obj->nextshape)
	{
		while ((CurBOBShape->class != Class) && (!CurBOBShape->class) && CurBOBShape)
		{
			CurBOBShape = CurBOBShape->nextshape;
		}

		if (CurBOBShape)
		{
			CurBOBShape->class = noshape;
			return(true);
		}
	}
	return(false);

#pragma warn +pia

}


/////////////////////////////////////////////////////////////////////////////
//
// RemoveBOBList() - Removes an entire BOBList attached to an object.
//
//
void RemoveBOBList(objtype *obj)
{
	struct BOB_Shape *CurBOBShape;

#pragma warn -pia

	if (CurBOBShape = obj->nextshape)
	{
		// Treverse down BOBList looking for a sprite with the same class
		// OR an empty shape struct to store the new shape.

		while (CurBOBShape)
		{
			if (CurBOBShape->class)
			{
				CurBOBShape->class = noshape;
				RF_RemoveSprite (&CurBOBShape->sprite);
			}
			CurBOBShape = CurBOBShape->nextshape;
		}
	}

#pragma warn +pia

}



/////////////////////////////////////////////////////////////////////////////
//
// InitBOBList() -- This initializes a BOB list for all the possible shapes
//						  attached at one time.  This is done with an array of
//						  BOB_Shape structs and links the 'nextshape' pointer to
//						  to the next element.
//
//
void InitBOBList(objtype *obj, struct BOB_Shape *BOB_Shape, short NumElements)
{
	struct BOB_Shape *CurShape;
	short loop;

	obj->nextshape = BOB_Shape;

	for (loop=1;loop<NumElements;loop++)
	{
		CurShape = BOB_Shape++;
		CurShape->nextshape = BOB_Shape;
	}

	BOB_Shape->nextshape = NULL;
}


////////////////////////////////////////////////////////////////////////////
//
// RefreshBOBList() -- This routine updates all sprites attached to the
//							  BOBList and refreshes there position in the sprite
//							  list.
//
void RefreshBOBList(objtype *obj)
{
	struct BOB_Shape *Shape;

	Shape = obj->nextshape;

	while (Shape)
	{
		if (Shape->class)
			RF_PlaceSprite(&Shape->sprite,obj->x+Shape->x_offset,obj->y+Shape->y_offset, Shape->shapenum, spritedraw,Shape->priority,Shape->sprflags);
		Shape = Shape->nextshape;
	}
}
#endif










//==========================================================================
// JAMPAK routines
//==========================================================================
#define N		4096
#define F		18

// THRESHOLD : encode string into position and length if match_length is
// greater than this

#define THRESHOLD				2

// index for root of binary search trees
//

#define NIL       			N
#define COMP					"COMP"

unsigned long 	textsize = 0,  		// text size counter
					codesize = 0,			// code size counter
					printcount = 0;     	// counter for reporting progress every 1K bytes

unsigned char far text_buf[N + F - 1];

memptr segptr;

BufferedIO lzwBIO;

//--------------------------------------------------------------------------
// BLoad()
//--------------------------------------------------------------------------
unsigned long BLoad(char *SourceFile, memptr *DstPtr)
{
	int handle;

	memptr SrcPtr;
	longword i, j, k, r, c;
	word flags;
	byte Buffer[8];
	longword DstLen, SrcLen;
	boolean comp;

	if ((handle = open(SourceFile, O_RDONLY|O_BINARY)) == -1)
		return(0);

	// Look for 'COMP' header
	//
	read(handle,Buffer,4);
	comp = !strncmp(Buffer,COMP,4);

	// Get source and destination length.
	//
	if (comp)
	{
		SrcLen = Verify(SourceFile);
		read(handle,(void *)&DstLen,4);
		MM_GetPtr(DstPtr,DstLen);
		if (!*DstPtr)
			return(0);
	}
	else
		DstLen = Verify(SourceFile);

	// LZW decompress OR simply load the file.
	//
	if (comp)
	{

		if (MM_TotalFree() < SrcLen)
		{
			if (!InitBufferedIO(handle,&lzwBIO))
				TrashProg("No memory for buffered I/O.");
			lzwDecompressFromFile(&lzwBIO,MK_FP(*DstPtr,0),SrcLen+8);
			FreeBufferedIO(&lzwBIO);
		}
		else
		{
			CA_LoadFile(SourceFile,&SrcPtr);
			lzwDecompressFromRAM(MK_FP(SrcPtr,8),MK_FP(*DstPtr,0),SrcLen+8);
			MM_FreePtr(&SrcPtr);
		}
	}
	else
		CA_LoadFile(SourceFile,DstPtr);

	close(handle);
	return(DstLen);
}

//--------------------------------------------------------------------------
// lzwDecompressFromRAM()
//
// SrcPtr - pointer to first byte of compressed data.
// DstPtr - pointer to decompress memory area.
// SrcLen - length of compressed data.
//
//--------------------------------------------------------------------------

#undef nextch()
#define nextch(ptr)	*ptr++

void lzwDecompressFromRAM(byte far *SrcPtr, byte far *DstPtr, longword SrcLen)
{
	longword i, j, k, r, c;
	word flags;
	byte ch;


	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';

	 r = N - F;
	 flags = 0;

	 for ( ; ; )
	 {
			if (((flags >>= 1) & 256) == 0)
			{
				if (!(--SrcLen))
					break;
				c=nextch(SrcPtr);

				flags = c | 0xff00;      /* uses higher byte cleverly */
			}                                  /* to count eight */

			if (flags & 1)
			{
				if (!(--SrcLen))
					break;
				c=nextch(SrcPtr);

				*DstPtr++ = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
			else
			{
				if (!(--SrcLen))
					break;
				i=nextch(SrcPtr);

				if (!(--SrcLen))
					break;
				j=nextch(SrcPtr);

				i |= ((j & 0xf0) << 4);
				j = (j & 0x0f) + THRESHOLD;

				for (k = 0; k <= j; k++)
				{
						 c = text_buf[(i + k) & (N - 1)];
						 *DstPtr++ = c;
						 text_buf[r++] = c;
						 r &= (N - 1);
				}
			}
	 }
}

//--------------------------------------------------------------------------
// lzwDecompressFromHandle()
//
// SrcPtr - pointer to first byte of compressed data.
// DstPtr - pointer to decompress memory area.
// SrcLen - length of compressed data.
//
//--------------------------------------------------------------------------

#undef nextch()
#define nextch(handle)	bio_readch(&lzwBIO)

void lzwDecompressFromFile(BufferedIO *SrcPtr, byte far *DstPtr, longword SrcLen)
{
	longword i, j, k, r, c;
	word flags;
	byte ch;


	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';

	 r = N - F;
	 flags = 0;

	 for ( ; ; )
	 {
			if (((flags >>= 1) & 256) == 0)
			{
				if (!(--SrcLen))
					break;
				c=nextch(SrcPtr);

				flags = c | 0xff00;      /* uses higher byte cleverly */
			}                                  /* to count eight */

			if (flags & 1)
			{
				if (!(--SrcLen))
					break;
				c=nextch(SrcPtr);

				*DstPtr++ = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
			else
			{
				if (!(--SrcLen))
					break;
				i=nextch(SrcPtr);

				if (!(--SrcLen))
					break;
				j=nextch(SrcPtr);

				i |= ((j & 0xf0) << 4);
				j = (j & 0x0f) + THRESHOLD;

				for (k = 0; k <= j; k++)
				{
						 c = text_buf[(i + k) & (N - 1)];
						 *DstPtr++ = c;
						 text_buf[r++] = c;
						 r &= (N - 1);
				}
			}
	 }
}

//--------------------------------------------------------------------------
// InitBufferedIO()
//--------------------------------------------------------------------------
memptr InitBufferedIO(int handle, BufferedIO *bio)
{
	bio->handle = handle;
	bio->offset = BIO_BUFFER_LEN;
	bio->status = 0;
	MM_GetPtr(&bio->buffer,BIO_BUFFER_LEN);

	return(bio->buffer);
}

//--------------------------------------------------------------------------
// FreeBufferedIO()
//--------------------------------------------------------------------------
void FreeBufferedIO(BufferedIO *bio)
{
	if (bio->buffer)
		MM_FreePtr(&bio->buffer);
}

//--------------------------------------------------------------------------
// bio_readch()
//--------------------------------------------------------------------------
byte bio_readch(BufferedIO *bio)
{
	byte far *buffer;

	if (bio->offset == BIO_BUFFER_LEN)
	{
		bio->offset = 0;
		bio_fillbuffer(bio);
	}

	buffer = MK_FP(bio->buffer,bio->offset++);

	return(*buffer);
}

//--------------------------------------------------------------------------
// bio_fillbuffer()
//
// BUGS (Not really bugs... More like RULES!)
//
//    1) This code assumes BIO_BUFFER_LEN is no smaller than
//       NEAR_BUFFER_LEN!!
//
//    2) BufferedIO.status should be altered by this code to report
//       read errors, end of file, etc... If you know how big the file
//       is you're reading, determining EOF should be no problem.
//
//--------------------------------------------------------------------------
void bio_fillbuffer(BufferedIO *bio)
{
	#define NEAR_BUFFER_LEN	(64)
	byte near_buffer[NEAR_BUFFER_LEN];
	short bio_length,bytes_read,bytes_requested;

	bytes_read = 0;
	bio_length = BIO_BUFFER_LEN;
	while (bio_length)
	{
		if (bio_length > NEAR_BUFFER_LEN-1)
			bytes_requested = NEAR_BUFFER_LEN;
		else
			bytes_requested = bio_length;

		read(bio->handle,near_buffer,bytes_requested);
		_fmemcpy(MK_FP(bio->buffer,bytes_read),near_buffer,bytes_requested);

		bio_length -= bytes_requested;
		bytes_read += bytes_requested;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// SwapLong()
//
void SwapLong(long far *Var)
{
	asm		les	bx,Var
	asm		mov	ax,[es:bx]
	asm		xchg	ah,al
	asm		xchg	ax,[es:bx+2]
	asm		xchg	ah,al
	asm 		mov	[es:bx],ax
}

///////////////////////////////////////////////////////////////////////////
//
// SwapWord()
//
void SwapWord(unsigned int far *Var)
{
	asm		les	bx,Var
	asm		mov	ax,[es:bx]
	asm		xchg	ah,al
	asm		mov	[es:bx],ax
}

////////////////////////////////////////////////////////////////////////////
//
// LoadShape()
//
int LoadShape(char *Filename,struct Shape *SHP)
{
	#define CHUNK(Name)	(*ptr == *Name) &&			\
								(*(ptr+1) == *(Name+1)) &&	\
								(*(ptr+2) == *(Name+2)) &&	\
								(*(ptr+3) == *(Name+3))


	int RT_CODE;
//	struct ffblk ffblk;
	FILE *fp;
	char CHUNK[5];
	char far *ptr;
	memptr IFFfile = NULL;
	unsigned long FileLen, size, ChunkLen;
	int loop;


	RT_CODE = 1;

	// Decompress to ram and return ptr to data and return len of data in
	//	passed variable...

	if (!(FileLen = BLoad(Filename,&IFFfile)))
		TrashProg("Can't load Compressed Shape - Possibly corrupt file!");

	// Evaluate the file
	//
	ptr = MK_FP(IFFfile,0);
	if (!CHUNK("FORM"))
		goto EXIT_FUNC;
	ptr += 4;

	FileLen = *(long far *)ptr;
	SwapLong((long far *)&FileLen);
	ptr += 4;

	if (!CHUNK("ILBM"))
		goto EXIT_FUNC;
	ptr += 4;

	FileLen += 4;
	while (FileLen)
	{
		ChunkLen = *(long far *)(ptr+4);
		SwapLong((long far *)&ChunkLen);
		ChunkLen = (ChunkLen+1) & 0xFFFFFFFE;

		if (CHUNK("BMHD"))
		{
			ptr += 8;
			SHP->bmHdr.w = ((struct BitMapHeader far *)ptr)->w;
			SHP->bmHdr.h = ((struct BitMapHeader far *)ptr)->h;
			SHP->bmHdr.x = ((struct BitMapHeader far *)ptr)->x;
			SHP->bmHdr.y = ((struct BitMapHeader far *)ptr)->y;
			SHP->bmHdr.d = ((struct BitMapHeader far *)ptr)->d;
			SHP->bmHdr.trans = ((struct BitMapHeader far *)ptr)->trans;
			SHP->bmHdr.comp = ((struct BitMapHeader far *)ptr)->comp;
			SHP->bmHdr.pad = ((struct BitMapHeader far *)ptr)->pad;
			SwapWord(&SHP->bmHdr.w);
			SwapWord(&SHP->bmHdr.h);
			SwapWord(&SHP->bmHdr.x);
			SwapWord(&SHP->bmHdr.y);
			ptr += ChunkLen;
		}
		else
		if (CHUNK("BODY"))
		{
			ptr += 4;
			size = *((long far *)ptr);
			ptr += 4;
			SwapLong((long far *)&size);
			SHP->BPR = (SHP->bmHdr.w+7) >> 3;
			MM_GetPtr(&SHP->Data,size);
			if (!SHP->Data)
				goto EXIT_FUNC;
			movedata(FP_SEG(ptr),FP_OFF(ptr),FP_SEG(SHP->Data),0,size);
			ptr += ChunkLen;

			break;
		}
		else
			ptr += ChunkLen+8;

		FileLen -= ChunkLen+8;
	}

	RT_CODE = 0;

EXIT_FUNC:;
	if (IFFfile)
	{
//		segptr = (memptr)FP_SEG(IFFfile);
		MM_FreePtr(&IFFfile);
	}

	return (RT_CODE);
}


////////////////////////////////////////////////////////////////////////////
//
// FreeShape()
//
void FreeShape(struct Shape *shape)
{
	if (shape->Data)
		MM_FreePtr(&shape->Data);
}

////////////////////////////////////////////////////////////////////////////
//
// UnpackEGAShapeToScreen()
//
int UnpackEGAShapeToScreen(struct Shape *SHP,int startx,int starty)
{
	int currenty;
	signed char n, Rep, far *Src, far *Dst[8], loop, Plane;
	unsigned int BPR, Height;
	boolean NotWordAligned;

	NotWordAligned = SHP->BPR & 1;
	startx>>=3;
	Src = MK_FP(SHP->Data,0);
	currenty = starty;
	Plane = 0;
	Height = SHP->bmHdr.h;
	while (Height--)
	{
		Dst[0] = (MK_FP(0xA000,displayofs));
		Dst[0] += ylookup[currenty];
		Dst[0] += startx;
		for (loop=1; loop<SHP->bmHdr.d; loop++)
			Dst[loop] = Dst[0];


		for (Plane=0; Plane<SHP->bmHdr.d; Plane++)
		{
			outport(0x3c4,((1<<Plane)<<8)|2);

			BPR = ((SHP->BPR+1) >> 1) << 1;               // IGNORE WORD ALIGN
			while (BPR)
			{
				if (SHP->bmHdr.comp)
					n = *Src++;
				else
					n = BPR-1;

				if (n < 0)
				{
					if (n != -128)
					{
						n = (-n)+1;
						BPR -= n;
						Rep = *Src++;
						if ((!BPR) && (NotWordAligned))   // IGNORE WORD ALIGN
							n--;

						while (n--)
							*Dst[Plane]++ = Rep;
					}
					else
						BPR--;
				}
				else
				{
					n++;
					BPR -= n;
					if ((!BPR) && (NotWordAligned))     // IGNORE WORD ALIGN
						n--;

					while (n--)
						*Dst[Plane]++ = *Src++;

					if ((!BPR) && (NotWordAligned))     // IGNORE WORD ALIGN
						Src++;
				}
			}
		}
		currenty++;
	}

	return(0);
}

////////////////////////////////////////////////////////////////////////////
//
// GetKeyChoice()
//
char GetKeyChoice(char *choices,boolean clear)
{
	extern void DoEvents(void);

	boolean waiting;
	char *s,*ss;

	IN_ClearKeysDown();

	waiting = true;
	while (waiting)
	{
		s = choices;
		while (*s)
		{
			if (Keyboard[*s++])
			{
				waiting=false;
				break;
			}
		}
	}

	IN_ClearKeysDown();

	return(*(--s));
}

#if 0

////////////////////////////////////////////////////////////////////////////
//
// AnimateObj()
//
boolean AnimateObj(objtype *obj)
{
	boolean Done;

	Done = false;

	if (obj->animtype == at_NONE)		// Animation finished?
		return(true);						// YEP!

	if (obj->animdelay)					// Check animation delay.
	{
		obj->animdelay -= tics;
		if (obj->animdelay < 0)
			obj->animdelay = 0;
		return(false);
	}

	switch (obj->animtype)				// Animate this object!
	{
		case at_ONCE:
		case at_CYCLE:
			switch (obj->animdir)
			{
				case at_FWD:
					if (obj->curframe < obj->maxframe)
						AdvanceAnimFWD(obj);
					else
						if (obj->animtype == at_CYCLE)
						{
							obj->curframe = 0;		 // RESET CYCLE ANIMATION
							obj->animdelay=1;
						}
						else
						{
							obj->animtype = at_NONE; // TERMINATE ONCE ANIM
							Done = true;
						}
					break;

				case at_REV:
					if (obj->curframe > 0)
						AdvanceAnimREV(obj);
					else
						if (obj->animtype == at_CYCLE)
						{
							obj->curframe = obj->maxframe;	// RESET CYCLE ANIMATION
							obj->animdelay = 1;
						}
						else
						{
							obj->animtype = at_NONE;   // TERMINATE ONCE ANIM
							Done = true;
						}
					break; // REV
			}
			break;

		case at_REBOUND:
			switch (obj->animdir)
			{
				case at_FWD:
					if (obj->curframe < obj->maxframe)
						AdvanceAnimFWD(obj);
					else
					{
						obj->animdir = at_REV;
						obj->animdelay = 1;
					}
					break;

				case at_REV:
					if (obj->curframe > 0)
						AdvanceAnimREV(obj);
					else
					{
						obj->animdir = at_FWD;
						obj->animdelay = 1;
						Done = true;
					}
					break;
			}
			break; /* REBOUND */

		case at_WAIT:
			Done = true;
			break;
	}

	return(Done);
}

void AdvanceAnimFWD(objtype *obj)	// Advances a Frame of ANIM for ONCE,CYCLE, REBOUND
{
	obj->curframe++; // INC frames
	obj->animdelay = obj->maxdelay;		 // Init Delay Counter.
	obj->needtoreact = true;
}


void AdvanceAnimREV(objtype *obj)  // Advances a Frame of ANIM for ONCE,CYCLE, REBOUND
{
	obj->curframe--; // DEC frames
	obj->animdelay = obj->maxdelay;		 // Init Delay Counter.
	obj->needtoreact = true;
}
#endif

#if 0
///////////////////////////////////////////////////////////////////////////
//
// LoadASArray()  - Load an array of audio samples in FAR memory.
//
void LoadASArray(struct Sample *ASArray)
{
	int loop = 0;

	while (ASArray[loop].filename)
	{
	  if (!BLoad(ASArray[loop].filename,(memptr *)&ASArray[loop].data))
		  TrashProg("Unable to load sample in LoadASArray()");
	  loop++;
	}
}



////////////////////////////////////////////////////////////////////////////
//
// FreeASArray() - Frees an ASArray from memory that has been loaded with
//						 LoadASArray()
//
void FreeASArray(struct Sample *ASArray)
{
	unsigned loop = 0;

	while (ASArray[loop].data)
	{
		MM_SetPurge((memptr *)&ASArray[loop++].data,3);
		MM_FreePtr((memptr *)&ASArray[loop++].data);
	}
}

///////////////////////////////////////////////////////////////////////////
//
// GE_LoadAllDigiSounds()	- This is a hook that CA_LoadAllSounds()
//									  calls to load all of the Digitized sounds for
//									  Sound Blaster & Sound Source.
//
// NOTE : This stub would do any other necessary routines for DigiSounds
//			 specific to GAMERS EDGE code. (Keeping seperate from ID's)
//
void GE_LoadAllDigiSounds()
{
	LoadASArray(DigiSounds);
}



/////////////////////////////////////////////////////////////////////////
//
// GE_FreeAllDigiSounds() -- This is a hook that CA_LoadAllSounds()
//									  calls to free all digitized sounds for
//									  which ever hardware and allow for any necessary
//									  clean up.
//
//
// NOTE : This stub would do any other necessary routines for DigiSounds
//			 specific to GAMERS EDGE code. (Keeping seperate from ID's)
//
void GE_FreeAllDigiSounds()
{
	FreeASArray(DigiSounds);
}



///////////////////////////////////////////////////////////////////////////
//
// GE_LoadAllSounds()	- Loads ALL sounds needed for detected hardware.
//
void GE_LoadAllSounds()
{
	unsigned i,start;

	start = STARTPCSOUNDS;
	for (i=0;i<NUMSOUNDS;i++,start++)
		CA_CacheAudioChunk (start);

	if (AdLibPresent)
	{
		start = STARTADLIBSOUNDS;
		for (i=0;i<NUMSOUNDS;i++,start++)
			CA_CacheAudioChunk (start);
	}

	if (SoundBlasterPresent)
		LoadASArray(DigiSounds);
}


//////////////////////////////////////////////////////////////////////////
//
// GE_PurgeAllSounds() - Frees all sounds that were loaded.
//
void GE_PurgeAllSounds()
{
	unsigned start,i;

	start = STARTPCSOUNDS;
	for (i=0;i<NUMSOUNDS;i++,start++)
		if (audiosegs[start])
			MM_SetPurge (&(memptr)audiosegs[start],3);		// make purgable


	if (AdLibPresent)
	{
		start = STARTADLIBSOUNDS;
		for (i=0;i<NUMSOUNDS;i++,start++)
			if (audiosegs[start])
				MM_SetPurge (&(memptr)audiosegs[start],3);		// make purgable
	}

	if (SoundBlasterPresent)
		GE_FreeAllDigiSounds();
}


/////////////////////////////////////////////////////////////////////////////
//
// PlaySample() -- Plays a DIGITIZED sample using SoundBlaster OR SoundSource
//
// PARAMETERS : Sample Number (Corresponding to ASArray "DigiSounds[]".
//
void PlaySample(unsigned SampleNum)
{

	if (!DigiSounds[SampleNum].data)
		TrashProg("PlaySample - Trying to play an unloaded digi sound!");


	switch (SoundMode)				// external variable in ID_SD for sound mode..
	{
		case sdm_SoundBlaster:
		case sdm_SoundBlasterAdLib:
			SDL_SBPlaySample(MK_FP(DigiSounds[SampleNum].data,0));
			break;

		default:
			TrashProg("PlaySample() - incorrect SoundMode for PlaySample()");
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// SelectDigiAudio() -- This routine intergrates multi sound hardware with
//								id's routines.
//
void SelectDigiAudio(AudioDeviceType Device)
{
	switch (Device)
	{
		case ged_SoundSource:
		case ged_SoundBlaster:
			break;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
// DisplayGameList()
//
void DisplayGameList(short winx, short winy, short list_width, short list_height)
{
	#define SPACES 2

	short width,col,row,orgcol,games_printed=0,h;

	// Possibly shrink window.
	//
	h = (NumGames / list_width) + ((NumGames % list_width) > 0);
	if (h < list_height)
		list_height = h;

	// Open window and print header...
	//
	US_DrawWindow(winx,winy,list_width*(8+SPACES*2),list_height+3);
	US_CPrintLine("LIST OF SAVED GAMES");
	US_Print("\n");

	col = orgcol = PrintX;
	row = PrintY;

	// Display as many 'save game' files as can fit in the window.
	//
	width = list_width;
	while ((games_printed<NumGames) && (list_height))
	{
		// Print filename and padding spaces.
		//
		US_Printxy(col+(SPACES*8),row,GameListNames[games_printed]);
		col += 8*((SPACES*2)+8);

		// Check for end-of-line or end-of-window.
		//
		width--;
		if (!width)
		{
			col = orgcol;
			row += 8;
			width = list_width;
			list_height--;
			US_Print("\n");
		}

		games_printed++;
	}
}

////////////////////////////////////////////////////////////////////////////
//
// ReadGameList()
//
void ReadGameList()
{
	struct ffblk ffblk;
	short done,len;

	NumGames = -1;
	done = findfirst("*.sav",&ffblk,0);

	while (!done)
	{
		if (NumGames == MAX_GAMELIST_NAMES)
			memcpy(GameListNames,GameListNames[1],MAX_GAMELIST_NAMES*sizeof(GameListNames[0]));
		else
			NumGames++;

		fnsplit(ffblk.ff_name,NULL,NULL,GameListNames[NumGames],NULL);

		done=findnext(&ffblk);
	}

	NumGames++;
}

////////////////////////////////////////////////////////////////////////////
//
// LoadTextFile()
//
long LoadTextFile(char *filename,textinfo *textinfo)
{
#pragma warn -pia
	long size;

	if (!(size=BLoad(filename,&textinfo->textptr)))
		return(0);
	InitTextFile(textinfo);

	return(size);
#pragma warn +pia
}

//-------------------------------------------------------------------------
// FreeTextFile()
//-------------------------------------------------------------------------
void FreeTextFile(textinfo *textinfo)
{
	if (textinfo->textptr)
		MM_FreePtr(&textinfo->textptr);
}

////////////////////////////////////////////////////////////////////////////
//
// InitTextFile()
//
void InitTextFile(textinfo *textinfo)
{
	#define END_PAGE  '@'

	char far *text = MK_FP(textinfo->textptr,0);

	textinfo->totalpages = 0;
	while (*text != END_PAGE)
	{
		if (textinfo->totalpages < MAX_TEXT_PAGES)
			textinfo->pages[textinfo->totalpages++] = text;
		else
			TrashProg("GE ERROR: Too many text pages. --> %d",textinfo->totalpages);

		while (*text != END_PAGE)
		{
			if ((*text == '\r') && (*(text+1) == '\n'))
			{
				*text = 32;
				*(text+1) = '\n';
				text+=2;
			}
			else
				text++;
		}
		*text = 0;
		text += 3;
	}
	*text = 0;
}

#if 0
////////////////////////////////////////////////////////////////////////////
//
// CenterObj()
//
void CenterObj(objtype *obj, unsigned x, unsigned y)
{
	spritetabletype far *sprite;
	unsigned width, height;

	sprite=&spritetable[obj->baseshape+obj->curframe-STARTSPRITES];

	width = (sprite->xh-sprite->xl+(1<<G_P_SHIFT)) >> 1;
	if (obj->sprflags&sf_vertflip)
	{
		height = (sprite->yl-(sprite->height<<G_P_SHIFT) + sprite->yh+(1<<G_P_SHIFT)) >> 1;
	}
	else
		height = (sprite->yh-sprite->yl+(1<<G_P_SHIFT)) >> 1;

	obj->x = x-width;
	obj->y = y-height;
}
#endif

//-------------------------------------------------------------------------
// cacheout()
//-------------------------------------------------------------------------
void cacheout(short s,short e)
{
	short i;

	for(i=(s);i<=(e);i++)
	{
		grneeded[i]&=~ca_levelbit;
		if (grsegs[i])
			MM_SetPurge(&grsegs[i],3);
	}

}

//-------------------------------------------------------------------------
// cachein()
//-------------------------------------------------------------------------
void cachein(short s,short e)
{
	short i;

	for(i=(s);i<=(e);i++)
	{
		CA_MarkGrChunk(i);
		if (grsegs[i])
			MM_SetPurge(&grsegs[i],0);
	}
}

#if 0
////////////////////////////////////////////////////////////////////////////
//
// SetUpdateBlock()
//
void SetUpdateBlock(unsigned x,unsigned y,unsigned width,unsigned height,char refresh)
{
	eraseblocktype *erase;

#if 0 //SP unsure if this is needed
	x = (x+((MAPBORDER+MAPXLEFTOFFSET)<<4))>>3;
	y += ((MAPBORDER+MAPYTOPOFFSET)<<4);
#else
	x = (x+(MAPBORDER<<4))>>3;
	y += (MAPBORDER<<4);
#endif
	width >>= 3;

	if (refresh & 1)
	{
		erase = eraselistptr[0]++;
		erase->screenx=x;
		erase->screeny=y;
		erase->width=width;
		erase->height=height;
	}

	if (refresh & 2)
	{
		erase = eraselistptr[1]++;
		erase->screenx=x;
		erase->screeny=y;
		erase->width=width;
		erase->height=height;
	}
}


////////////////////////////////////////////////////////////////////////////
//
// ObjHeight
//
unsigned ObjHeight(objtype *obj)
{
	spritetabletype far *sprite;

	sprite=&spritetable[obj->baseshape+obj->curframe-STARTSPRITES];

	if (obj->sprflags&sf_vertflip)
	{
		return((sprite->yl-(sprite->height<<G_P_SHIFT) + sprite->yh+(1<<G_P_SHIFT)) >> 1);
	}
	else
		return((sprite->yh-sprite->yl+(1<<G_P_SHIFT)) >> 1);
}


////////////////////////////////////////////////////////////////////////////
//
// ObjWidth
//
unsigned ObjWidth(objtype *obj)
{
	spritetabletype far *sprite;

	sprite=&spritetable[obj->baseshape+obj->curframe-STARTSPRITES];

	return((sprite->xh-sprite->xl+(1<<G_P_SHIFT)) >> 1);
}
#endif

#if 0
//--------------------------------------------------------------------------
// visible_on()
//--------------------------------------------------------------------------
boolean visible_on(objtype *obj)
{
	if (!(obj->flags & of_visible))
	{
		obj->needtoreact=true;
		obj->flags |= of_visible;
		return(true);
	}

	return(false);
}

//--------------------------------------------------------------------------
// visible_off()
//--------------------------------------------------------------------------
boolean visible_off(objtype *obj)
{
	if (obj->flags & of_visible)
	{
		obj->needtoreact=true;
		obj->flags &= ~of_visible;
		return(true);
	}

	return(false);
}
#endif


/*
===================
=
= FizzleFade
=
===================
*/

#define PIXPERFRAME     10000	//1600

void FizzleFade (unsigned source, unsigned dest,
	unsigned width,unsigned height, boolean abortable)
{
	unsigned        drawofs,pagedelta;
	unsigned        char maskb[8] = {1,2,4,8,16,32,64,128};
	unsigned        x,y,p,frame;
	long            rndval;
	ScanCode			 lastLastScan=LastScan=0;

	width--;
	height--;

	pagedelta = dest-source;
//	VW_SetScreen (dest,0);
	rndval = 1;
	y = 0;

asm     mov     es,[screenseg]
asm     mov     dx,SC_INDEX
asm     mov     al,SC_MAPMASK
asm     out     dx,al

	TimeCount=frame=0;
	do      // while (1)
	{
		if ((abortable) || (Flags & FL_QUICK))
		{
			IN_ReadControl(0,&control);
			if (control.button0 || control.button1 || (lastLastScan != LastScan)
			|| Keyboard[sc_Escape] || (Flags & FL_QUICK))
			{
				VW_ScreenToScreen (source,dest,(width+1)/8,height+1);
				goto exitfunc;
			}
		}

		for (p=0;p<PIXPERFRAME;p++)
		{
			//
			// seperate random value into x/y pair
			//
			asm     mov     ax,[WORD PTR rndval]
			asm     mov     dx,[WORD PTR rndval+2]
			asm     mov     bx,ax
			asm     dec     bl
			asm     mov     [BYTE PTR y],bl                 // low 8 bits - 1 = y xoordinate
			asm     mov     bx,ax
			asm     mov     cx,dx
			asm     shr     cx,1
			asm     rcr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     mov     [x],bx                                  // next 9 bits = x xoordinate
			//
			// advance to next random element
			//
			asm     shr     dx,1
			asm     rcr     ax,1
			asm     jnc     noxor
			asm     xor     dx,0x0001
			asm     xor     ax,0x2000
noxor:
			asm     mov     [WORD PTR rndval],ax
			asm     mov     [WORD PTR rndval+2],dx

			if (x>width || y>height)
				continue;
			drawofs = source+ylookup[y];

			asm     mov     cx,[x]
			asm     mov     si,cx
			asm     and     si,7
			asm     mov dx,GC_INDEX
			asm     mov al,GC_BITMASK
			asm     mov     ah,BYTE PTR [maskb+si]
			asm     out dx,ax

			asm     mov     si,[drawofs]
			asm     shr     cx,1
			asm     shr     cx,1
			asm     shr     cx,1
			asm     add     si,cx
			asm     mov     di,si
			asm     add     di,[pagedelta]

			asm     mov     dx,GC_INDEX
			asm     mov     al,GC_READMAP                   // leave GC_INDEX set to READMAP
			asm     out     dx,al

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,1
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,0
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,2
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,1
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,4
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,2
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,8
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,3
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			if (rndval == 1)                // entire sequence has been completed
				goto exitfunc;
		}
		frame++;
//		while (TimeCount<frame)         // don't go too fast
//		;
	} while (1);

exitfunc:;
	EGABITMASK(255);
	EGAMAPMASK(15);
	return;
}

//-------------------------------------------------------------------------
// mprintf()
//-------------------------------------------------------------------------
void mprintf(char *msg, ...)
{
	static char x=0;
	static char y=0;
	static char far *video = MK_FP(0xb000,0x0000);
	char buffer[100],*ptr;

	va_list(ap);

	va_start(ap,msg);

	vsprintf(buffer,msg,ap);

	ptr = buffer;
	while (*ptr)
	{
		switch (*ptr)
		{
			case '\n':
				if (y >= 23)
				{
					video -= (x<<1);
					_fmemcpy(MK_FP(0xb000,0x0000),MK_FP(0xb000,0x00a0),3840);
				}
				else
				{
					y++;
					video += ((80-x)<<1);
				}
				x=0;
			break;

			default:
				*video = *ptr;
				video[1] = 15;
				video += 2;
				x++;
			break;
		}
		ptr++;
	}

	va_end(ap);
}

#if 0

//--------------------------------------------------------------------------
//								 FULL SCREEN REFRESH/ANIM MANAGERS
//--------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////
//
//  InitLatchRefresh() -- Loads an ILBM (JAMPAK'd) into the Master Latch
//								  to be used as the background refresh screen...
//
void InitLatchRefresh(char *filename)
{
	struct Shape RefreshShp;
	short yofs;

	VW_ClearVideo(0);

	if (LoadShape(filename,&RefreshShp))
		TrashProg("Can't load %s",filename);

	VW_SetLineWidth(RefreshShp.BPR);

	yofs = masterswap/SCREENWIDTH;
	MoveGfxDst(0,yofs);								// Handle title screen
	UnpackEGAShapeToScreen(&RefreshShp,0,0);
	FreeShape(&RefreshShp);

	MoveScreen(0,0);
	VW_InitDoubleBuffer();

	RF_NewPosition(0,0);

	RF_Refresh();

	SetUpdateBlock(0,0,RefreshShp.bmHdr.w,RefreshShp.bmHdr.h,3);
}


////////////////////////////////////////////////////////////////////////////
//
// InitFullScreenAnim() -- Initialize ALL necessary functions for full screen
//					  				animation types.
//
// filename - filename for background lbm.
// SpawnAll - spawn function to call to spawn all inital objects..
//
void InitFullScreenAnim(char *filename, void (*SpawnAll)())
{
	unsigned old_flags;

	old_flags = GE_SystemFlags.flags;
	GE_SystemFlags.flags &= ~(GEsf_Tiles | GEsf_Panning | GEsf_RefreshVec);

	CA_ClearMarks();

	RFL_InitSpriteList();
	InitObjArray();

	if (SpawnAll)
		SpawnAll();

	CA_CacheMarks(NULL);

	CalcInactivate();
	CalcSprites();

	InitLatchRefresh(filename);

	RF_ForceRefresh();
	RF_ForceRefresh();

	GE_SystemFlags.flags = old_flags;
}



///////////////////////////////////////////////////////////////////////////
//
// DoFullScreenAnim()	-- a General "Do-Every-Thing" function that
//									displays a full screen animation...
//
// filename - Filename of background lbm
//	SpawnAll	- Function to call to spawn all inital objects (REQUIRED)
//	CheckKey - Function to call every cycle - The function called must
//				  return a value of greater than zero (0) to terminate the
//				  animation cycle.									  (REQUIRED)
//	CleanUp	- Function to call upon exiting the animation. (OPTIONAL)
//
void DoFullScreenAnim(char *filename, void (*SpawnAll)(), short (*CheckKey)(),void (*CleanUp)())
{
	unsigned old_flags;
	boolean ExitAnim = false;

	// Save off the current system flags....

	old_flags = GE_SystemFlags.flags;
	GE_SystemFlags.flags &= ~(GEsf_Tiles | GEsf_Panning);

//	randomize();

	// Init refresh latch screen, initalize all object, and setup video mode.

	InitFullScreenAnim(filename,SpawnAll);

	VW_FadeIn();

	while (!ExitAnim)
	{
		CalcInactivate();
		CalcSprites();
		RF_Refresh();

		ExitAnim = (boolean)CheckKey();
	}

//	RemoveBOBList(player);
//	CalcInactivate();

	if (CleanUp)
		CleanUp();

	// Restore old system flags...

	GE_SystemFlags.flags = old_flags;
}

#endif

//--------------------------------------------------------------------------
// FindFile()
//--------------------------------------------------------------------------
boolean FindFile(char *filename,char *disktext,char disknum)
{
	extern boolean splitscreen;

	char command[100];
	char choices[]={sc_Escape,sc_Space,0},drive[2];
	boolean fadeitout=false,rt_code=2;

	if (!disktext)
		disktext = GAMENAME;
	drive[0] = getdisk() + 'A';
	drive[1] = 0;
	while (rt_code == 2)
	{
		if (Verify(filename))
			rt_code = true;
		else
		{
			if (ge_textmode)
			{
				clrscr();
				gotoxy(1,1);
				printf("\nInsert %s disk %d into drive %s.\n",disktext,disknum,drive);
				printf("Press SPACE to continue, ESC to abort.\n");
			}
			else
			{
				if (splitscreen)
				{
					bufferofs = displayofs = screenloc[screenpage];
					CenterWindow(38,5);
				}
				else
				{
					bufferofs = displayofs = 0;
					US_CenterWindow(38,5);
				}

				strcpy(command,"\nInsert ");
				strcat(command,disktext);
				strcat(command," disk");
				if (disknum != -1)
				{
					strcat(command," ");
					itoa(disknum,command+strlen(command),10);
				}
				strcat(command," into drive ");
				strcat(command,drive);
				strcat(command,".");
				US_CPrint(command);
				US_CPrint("Press SPACE to continue, ESC to abort.");
			}

			sound(300);
			VW_WaitVBL(20);
			nosound();

			if (!ge_textmode)
			{
				if (screenfaded)
				{
					VW_FadeIn();
					fadeitout=true;
				}
			}
			if (GetKeyChoice(choices,true) == sc_Escape)
				rt_code = false;
		}
	}

	if (!ge_textmode)
		if (fadeitout)
			VW_FadeOut();

	return(rt_code);
}

#if 0
//--------------------------------------------------------------------------
// CacheAll()
//--------------------------------------------------------------------------
void CacheAV(char *title)
{
	if (Verify("EGAGRAPH."EXT))
	{
		CA_CacheMarks(title);
		if (!FindFile("EGAGRAPH."EXT,AUDIO_DISK))
			TrashProg("Can't find graphics files.");

// cache in audio

		current_disk = AUDIO_DISK;
	}
	else
	{

// cache in audio

		if (!FindFile("EGAGRAPH."EXT,VIDEO_DISK))
			TrashProg("Can't find audio files.");
		CA_CacheMarks(title);

		current_disk = VIDEO_DISK;
	}
}

#endif














#if 0

/////////////////////////////////////////////////////////////////////////////
//
//  GE_DecompressToRAM() -- This Decompression routine uses normal memory
//									 allocation conventions..
//
unsigned char huge *GE_DecompressToRAM(char *SourceFile, unsigned long *DataSize)  /* Just the reverse of Encode(). */
{
	FILE *infile;  /* input & output files */

	unsigned long i, j, k, r, c;
	unsigned char huge *DataPtr;
	unsigned char huge *CurPtr;
	unsigned char Buffer[8];
	unsigned long DstLen;

	if (!(infile = fopen(SourceFile, "rb")))
		return(0);

	// Read Header....

	fread(Buffer,1,4,infile);

	if (strncmp(Buffer,COMP,4))
	{
		fclose(infile);
		printf("NOT a JAM Compressed FILE!\n");
		return(0);
	}

	fread((void *)&DstLen,1,4,infile);

	if (!(DataPtr = farmalloc(DstLen)))
		 return(0);

	 fclose(infile);

	*DataSize = DstLen;

	DecompressToRAMLocation(SourceFile,(unsigned char far *)DataPtr);

	return(DataPtr);
}

#endif
