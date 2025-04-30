/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
 * Copyright (C) 2025 NY00123
 *
 * This file is primarily based on:
 * Wolfenstein 3-D Source Code
 * Copyright (C) 1992 id Software
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
// local defines:
/////////////////////////////////////////////////////////////////////////////

#define BACKCOLOR BE_CGA_BLUE

#define WORDLIMIT 80
#define FONTHEIGHT 10
#define TOPMARGIN 10
#define BOTTOMMARGIN 10
#define LEFTMARGIN 10
#define RIGHTMARGIN 10
#define PICMARGIN 8
#define SPACEWIDTH 7
#define TEXTROWS ((200-TOPMARGIN-BOTTOMMARGIN)/FONTHEIGHT)
#define SCREENPIXWIDTH 320
#define SCREENMID (SCREENPIXWIDTH/2)

/////////////////////////////////////////////////////////////////////////////
// initialized variables:
/////////////////////////////////////////////////////////////////////////////

#ifdef BETA
static const id0_char_t *textfiles[] =
{
	"HELPART.BH1",
	"CONTRART.BH1",
	"STORYART.BH1",
	"ORDERART.BH1",
	"ABOUT.BH1"
};
#else
static boolean timerdone = false;
static Uint16 textchunks[] =
{
	ORDERTEXT,
	MENUHELPTEXT,
	STORYTEXT,
	GAMEHELPTEXT,
	HIGHSCORETEXT
#if (EPISODE == 1)
	,ANNOYINGTEXT
#endif
};
#endif

/////////////////////////////////////////////////////////////////////////////
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

static Sint16 pagenum;
static Sint16 numpages;
static Uint16 leftmargin[TEXTROWS];
static Uint16 rightmargin[TEXTROWS];
static const id0_char_t id0_far *text;
static Sint16 rowon;
static Sint16 picx;
static Sint16 picy;
static Sint16 picnum;
static Sint16 picdelay;
static boolean layoutdone;
static Sint16 helpselection;

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
=================
=
= HelpMessage
=
=================
*/

#ifndef BETA
void HelpMessage(const id0_char_t id0_far *message)
{
	if (!helpmessages || DemoMode)
	{
		return;
	}
	VW_FixRefreshBuffer();
	US_CenterWindow(35, 2);
	PrintY += 2;
	US_CPrint(message);
	VW_UpdateScreen();
	VW_WaitVBL(50);
	IN_ClearKeysDown();
	IN_Ack();
	RF_ForceRefresh();
}
#endif

//===========================================================================

/*
=================
=
= RipToEOL
=
=================
*/

void RipToEOL(void)
{
	while (*text++ != '\n');
}

/*
=================
=
= ParseNumber
=
=================
*/

static Sint16 ParseNumber(void)
{
	id0_char_t c, buffer[80];
	id0_char_t *bufptr;

//
// scan until a number is found
//
	c = *text;
	while (c < '0' || c > '9')
	{
		text++;
		c = *text;
	}
	
//
// copy the number out
//
	bufptr = buffer;
	do
	{
		*bufptr = c;
		bufptr++;
		text++;
		c = *text;
	} while (c >= '0' && c <= '9');
	*bufptr = '\0';
	return atoi(buffer);
}

/*
=================
=
= ParsePicCommand
=
= Call with text pointing just after a ^G
= Upon exit text points to the start of next line
=
=================
*/

static void ParsePicCommand(void)
{
	picy = ParseNumber();
	picx = ParseNumber();
	picnum = ParseNumber();
	RipToEOL();
}

/*
=================
=
= ParseTimedCommand
=
=================
*/

static void ParseTimedCommand(void)
{
	picy = ParseNumber();
	picx = ParseNumber();
	picnum = ParseNumber();
	picdelay = ParseNumber();
	RipToEOL();
}

/*
=================
=
= TimedPicCommand
=
= Call with text pointing just after a ^T
= Upon exit text points to the start of next line
=
=================
*/

static void TimedPicCommand(void)
{
	ParseTimedCommand();
	VW_WaitVBL(1);
	VW_ScreenToScreen(bufferofs, displayofs, 40, 200);
	SD_SetTimeCount(0);
	// Comparison was a 16-bit unsigned one in practice
	SD_TimeCountWaitForDest((Uint16)picdelay);
#if 0
	TimeCount = 0;
	while (picdelay > TimeCount);
#endif
	VWB_DrawPic(picx & ~7, picy, picnum);
}

/*
=================
=
= HandleCommand
=
=================
*/

static void HandleCommand(void)
{
	Sint16 i,margin,top,bottom;
	Sint16 picwidth,picheight,picmid;

	switch (toupper(*(++text)))
	{
	case 'B':
		picy = ParseNumber();
		picx = ParseNumber();
		picwidth = ParseNumber();
		picheight = ParseNumber();
		VWB_Bar(picx, picy, picwidth, picheight, 1);
		RipToEOL();
		break;
	case 'E':		// ^P is start of next page, ^E is end of file
	case 'P':
		layoutdone = true;
		text--;
		break;
	case 'C':		// ^c<hex digit> changes text color
		i = toupper(*(++text));
		if (i >= '0' && i <= '9')
		{
			fontcolor = i + 0 - '0';
		}
		else if (i >= 'A' && i <= 'F')
		{
			fontcolor = i + 10 - 'A';
		}
		fontcolor ^= BACKCOLOR;
		text++;
		break;
	case 'L':
		py = ParseNumber();
		rowon = (py - 10)/10;
		py = rowon * 10 + 10;
		px = ParseNumber();
		while (*(text++) != '\n');
		break;
	case 'T':		// ^Tyyy,xxx,ppp,ttt waits ttt tics, then draws 
		TimedPicCommand();
		break;
	case 'G':		// ^Gyyy,xxx,ppp draws graphic
		ParsePicCommand();
		VWB_DrawPic(picx & ~7, picy, picnum);
		picwidth = pictable[picnum-STARTPICS].width * 8;
		picheight = pictable[picnum-STARTPICS].height;
		picmid = picx + picwidth/2;
		//
		// adjust margins
		//
		if (picmid > SCREENMID)
		{
			margin = picx-PICMARGIN;			// new right margin
		}
		else
		{
			margin = picx+picwidth+PICMARGIN;	// new left margin
		}
		top = (picy-TOPMARGIN)/FONTHEIGHT;
		if (top < 0)
		{
			top = 0;
		}
		bottom = (picy+picheight-TOPMARGIN)/FONTHEIGHT;
		if (bottom >= TEXTROWS)
		{
			bottom = TEXTROWS-1;
		}
		for (i=top; i<=bottom; i++)
		{
			if (picmid > SCREENMID)
			{
				rightmargin[i] = margin;
			}
			else
			{
				leftmargin[i] = margin;
			}
		}
		//
		// adjust this line if needed
		//
		if (leftmargin[rowon] > px)
		{
			px = leftmargin[rowon];
		}
		break;
	}
}

/*
=================
=
= NewLine
=
=================
*/

static void NewLine(void)
{
	id0_char_t c;

	if (++rowon == TEXTROWS)
	{
	//
	// overflowed the page, so skip until next page break
	//
		layoutdone = true;
		do
		{
			if (*text == '^')
			{
				c = toupper(text[1]);
				if (c == 'E' || c == 'P')
				{
					layoutdone = true;
					return;
				}
			}
			text++;
		} while (1);
	}
	px = leftmargin[rowon];
	py += FONTHEIGHT;
}

/*
=================
=
= HandleCtrls
=
=================
*/

static void HandleCtrls(void)
{
	id0_char_t c;

	c = *(text++);			// get the character and advance
	if (c == '\n')
	{
		NewLine();
		return;
	}
}

/*
=================
=
= HandleWord
=
=================
*/

static void HandleWord(void)
{
	Uint16 wwidth, wheight, newpos, wordindex;
	id0_char_t word[WORDLIMIT];

	//
	// copy the next word into [word]
	//
	word[0] = *(text++);
	wordindex = 1;
	while (*text > ' ')
	{
		word[wordindex] = *(text++);
		if (++wordindex == WORDLIMIT)
		{
			Quit("PageLayout: Word limit exceeded");
		}
	}
	word[wordindex] = '\0';		// stick a null at end for C

	//
	// see if it fits on this line
	//
	VW_MeasurePropString(word, NULL, &wwidth, &wheight);
	while (rightmargin[rowon] < px+wwidth)
	{
		NewLine();
		if (layoutdone)
		{
			return;		// overflowed page
		}
	}

	//
	// print it
	//
	newpos = px+wwidth;
	VWB_DrawPropString(word, NULL);
	px = newpos;

	//
	// suck up any extra spaces
	//
	while (*text == ' ')
	{
		px += SPACEWIDTH;
		text++;
	}
}

/*
=================
=
= DrawScores
=
=================
*/

#ifndef BETA
static void DrawScores(void)
{
	Uint16 oldcolor, i;
	Uint16 wwidth, wheight;
	id0_char_t numbuf[16], *ptr;
	HighScore *entry;

	oldcolor = fontcolor;
	fontcolor = BE_CGA_YELLOW^BACKCOLOR;
	for (i=0, entry=&Scores[0]; i<8; i++, entry++)
	{
		PrintY = i*16 + 35;
		PrintX = 40;
		US_Print(entry->name);
		BE_Cross_ultoa_dec(entry->score, numbuf);
		for (ptr = numbuf; *ptr; ptr++)
		{
			*ptr = *ptr + 81;
		}
		USL_MeasureString(numbuf, NULL, &wwidth, &wheight);
		PrintX = 280-wwidth;
		US_Print(numbuf);
	}
	fontcolor = oldcolor;
}

/*
=================
=
= UpdateTimer
=
=================
*/

static void UpdateTimer(void)
{
	Sint16 oldcolor;

	oldcolor = fontcolor;
	VWB_DrawPic(16, 176, TEXTBORDERBOTTOMPIC);
	strcpy(str, "Timer= ");
	if (timeleft <= 0)
	{
		timeleft = -1;
		strcat(str, "GO!");
		timerdone = true;
		SD_PlaySound(BEACONSND);
	}
	else
	{
		BE_Cross_itoa_dec(timeleft, str2);
		strcat(str, str2);
#if (EPISODE == 1)
		SD_PlaySound(TIMERTICKSND);
#endif
	}
	fontcolor = BE_CGA_LIGHTRED^BACKCOLOR;
	py = 186;
	px = 218;
	VWB_DrawPropString(str, NULL);
	fontcolor = oldcolor;
}
#endif	// ifndef BETA

/*
=================
=
= PageLayout
=
= Clears the screen, draws the pics on the page, and word wraps the text.
= Returns a pointer to the terminating command
=
=================
*/

static void PageLayout(boolean shownumber)
{
	Sint16 oldcolor, i;
	id0_char_t c;

	oldcolor = fontcolor;
	fontcolor = BE_CGA_YELLOW^BACKCOLOR;
//
// clear the screen
//
	VWB_Bar(0, 0, 320, 200, BACKCOLOR);
	VWB_DrawPic( 16, 0, TEXTBORDERTOPPIC);
	VWB_DrawPic(  0, 0, TEXTBORDERLEFTPIC);
	VWB_DrawPic(304, 0, TEXTBORDERRIGHTPIC);
	if (shownumber)
	{
		VWB_DrawPic(16, 176, TEXTBORDERBOTTOMPIC);
	}
	else
	{
		VWB_DrawPic(16, 192, TEXTBORDERTOPPIC);
	}
	for (i=0; i<TEXTROWS; i++)
	{
		leftmargin[i] = LEFTMARGIN;
		rightmargin[i] = SCREENPIXWIDTH-RIGHTMARGIN;
	}
	px = LEFTMARGIN;
	py = TOPMARGIN;
	rowon = 0;
	layoutdone = false;

//
// make sure we are starting layout text (^P first command)
//
	while (*text <= ' ')
	{
		text++;
	}
	if (*text != '^' || toupper(*(++text)) != 'P')
	{
		Quit("PageLayout: Text not headed with ^P");
	}
	while (*(text++) != '\n');

//
// process text stream
//
	do
	{
		c = *text;
		if (c == '^')
		{
			HandleCommand();
		}
		else if (c <= ' ')
		{
			HandleCtrls();
		}
		else
		{
			HandleWord();
		}
	} while (!layoutdone);
	pagenum++;
	if (shownumber)
	{
#ifdef BETA
		strcpy(str, "pg ");
#else
		strcpy(str, "Pg ");
#endif
		BE_Cross_itoa_dec(pagenum, str2);
		strcat(str, str2);
		strcat(str, " of ");
		BE_Cross_itoa_dec(numpages, str2);
		strcat(str, str2);
		fontcolor = BE_CGA_LIGHTRED^BACKCOLOR;
		py = 186;
		px = 218;
		VWB_DrawPropString(str, NULL);
	}
	fontcolor = oldcolor;
}

//===========================================================================

/*
=====================
=
= BackPage
=
= Scans for a previous ^P
=
=====================
*/

static void BackPage(void)
{
	pagenum--;
	do
	{
		text--;
		if (text[0] == '^' && toupper(text[1]) == 'P')
		{
			return;
		}
	} while (1);
}

//===========================================================================

/*
=====================
=
= CacheLayoutGraphics
=
= Scans an entire layout file (until a ^E) marking all graphics used, and
= counting pages, then caches the graphics in
=
=====================
*/

static void CacheLayoutGraphics(void)
{
	const id0_char_t	id0_far *bombpoint, id0_far *textstart;
	id0_char_t	ch;

	textstart = text;
	bombpoint = text+30000;
	numpages = pagenum = 0;
	CA_MarkGrChunk(TEXTBORDERTOPPIC);
	CA_MarkGrChunk(TEXTBORDERLEFTPIC);
	CA_MarkGrChunk(TEXTBORDERRIGHTPIC);
	CA_MarkGrChunk(TEXTBORDERBOTTOMPIC);
	do
	{
		if (*text == '^')
		{
			ch = toupper(*(++text));
			if (ch == 'P')		// start of a page
			{
				numpages++;
			}
			if (ch == 'E')		// end of file, so load graphics and return
			{
				CA_CacheMarks(NULL);
				text = textstart;
				return;
			}
			if (ch == 'G')		// draw graphic command, so mark graphics
			{
				ParsePicCommand();
				CA_MarkGrChunk(picnum);
			}
			if (ch == 'T')		// timed draw graphic command, so mark graphics
			{
				ParseTimedCommand();
				CA_MarkGrChunk(picnum);
			}
		}
		else
		{
			text++;
		}
	} while (text < bombpoint);
	Quit("CacheLayoutGraphics: No ^E to terminate file!");
}

//===========================================================================

/*
=================
=
= HelpMenu
=
=================
*/

static Sint16 HelpMenu(void)
{
	CursorInfo cursor;
	ControlInfo control;
	Sint16 ydelta;
	Uint16 key;

	VWB_Bar(0, 0, 320, 200, BACKCOLOR);
	CA_CacheGrChunk(HELPMENUPIC);
	CA_CacheGrChunk(HELPCURSORPIC);
	CA_CacheGrChunk(TEXTBORDERTOPPIC);
	CA_CacheGrChunk(TEXTBORDERLEFTPIC);
	CA_CacheGrChunk(TEXTBORDERRIGHTPIC);
	VWB_DrawPic( 16,   0, TEXTBORDERTOPPIC);
	VWB_DrawPic(  0,   0, TEXTBORDERLEFTPIC);
	VWB_DrawPic(304,   0, TEXTBORDERRIGHTPIC);
	VWB_DrawPic( 16, 192, TEXTBORDERTOPPIC);
	VWB_DrawPic( 88,   8, HELPMENUPIC);
	ydelta = 0;
	IN_ClearKeysDown();
	do
	{
		if (helpselection < 0)
		{
			helpselection = 0;
		}
		else if (helpselection > 4)
		{
			helpselection = 4;
		}
		VWB_DrawPic(56, 24*helpselection+56, HELPCURSORPIC);
		VW_UpdateScreen();
		VWB_Bar(56, 24*helpselection+56, 31, 24, BACKCOLOR);
		IN_ReadControl(0, &control);
		IN_ReadCursor(&cursor);
		if (LastScan)
		{
			key = LastScan;
			IN_ClearKeysDown();
			switch (key)
			{
			case sc_UpArrow:
				helpselection--;
				break;
			case sc_DownArrow:
				helpselection++;
				break;
			case sc_Enter:
				VW_ClearVideo(BACKCOLOR);
				return helpselection;
			case sc_Escape:
				VW_ClearVideo(BACKCOLOR);
				return -1;
			}
		}
		ydelta += cursor.y;
		if (cursor.button0 || cursor.button1 || control.button0 || control.button1)
		{
			VW_ClearVideo(BACKCOLOR);
			return helpselection;
		}
		if (ydelta < -40)
		{
			ydelta += 40;
			helpselection--;
		}
		else if (ydelta > 40)
		{
			ydelta -= 40;
			helpselection++;
		}
		BE_ST_ShortSleep();
	} while (1);
}

/*
=================
=
= HelpScreens
=
=================
*/

void HelpScreens(void)
{
	Uint16 olddisplayofs, oldbufferofs, oldfontnumber, temp;
#ifdef BETA
	memptr buffer;
#endif
	Sint16 pos;
#ifndef BETA
	Uint16 oldmusic;
#endif
	boolean newpage;

	oldfontnumber = fontnumber;
	olddisplayofs = displayofs;
	oldbufferofs = bufferofs;
	fontnumber = 0;
#ifndef BETA
	oldmusic = currentmusic;
#endif
	EGAMAPMASK(15);
	CA_UpLevel();
	CA_SetGrPurge();	// redundant! CA_UpLevel already does this in BioMenace
	VW_ClearVideo(BACKCOLOR);
	RF_FixOfs();
	bufferofs = 0;
	displayofs = 0x8000;
	VW_SetScreen(displayofs, 0);
#ifndef BETA
	StartMusic(MUS_LEVEL1);
	SD_SetTimeCount(0);
	timerdone = false;
#endif
	do
	{
#ifdef BETA
		pos = HelpMenu();
#else
		if (timeleft == -1)
		{
			pos = HelpMenu();
		}
		else
		{
			pos = 5;
		}
		if (pos == 4)
		{
			textdrawscores = true;
		}
#endif
		VW_ClearVideo(BACKCOLOR);
		if (pos == -1)
		{
			CA_DownLevel();
			IN_ClearKeysDown();
			bufferofs = oldbufferofs;
			displayofs = olddisplayofs;
			fontnumber = oldfontnumber;
			VW_ClearVideo(BACKCOLOR);
			RF_FixOfs();
#ifdef BETA
			StopMusic();
#else
			SD_FadeOutMusic();
			while (SD_MusicPlaying())
				BE_ST_ShortSleep();
/*
	Note:
	It would be better to call StopMusic() before calling CA_DownLevel()
	to avoid "out of memory" crashes. The only downside of this is that
	there would be no music while the game re-caches the graphics.
	However, that is just a minor inconvenience compared to having the
	game crash.
*/
			StopMusic();
			StartMusic(oldmusic);
#endif
			return;
		}
		// BUG: should call CA_ClearMarks() here to make sure we don't cache more
		// than we need
#ifdef BETA
		CA_LoadFile(textfiles[pos], &buffer);
		text = (id0_char_t id0_far *)buffer;
		// BUG: should lock buffer to make sure it doesn't get moved around when
		// caching the graphics
#else
		pos = textchunks[pos];
		CA_CacheGrChunk(pos);
		text = (const id0_char_t id0_far *)grsegs[pos];
		// BUG: should lock grsegs[pos] to make sure it doesn't get moved around
		// when caching the graphics
#endif
		CacheLayoutGraphics();
		newpage = true;
		do
		{
			if (newpage)
			{
				newpage = false;
				PageLayout(true);
#ifndef BETA
				if (textdrawscores == true)
				{
					DrawScores();
					textdrawscores = false;
				}
#endif
				VW_SetScreen(bufferofs, 0);
				temp = displayofs;
				displayofs = bufferofs;
				bufferofs = temp;
			}
			LastScan = 0;
			while (!LastScan)
			{
				BE_ST_ShortSleep();
#ifndef BETA
				if (SD_GetTimeCount() >= TickBase && timeleft != -1)
				{
					LastScan = 0xFE;
				}
#endif
			}
#ifndef BETA
			if (SD_GetTimeCount() >= TickBase && timeleft > -1)
			{
				temp = displayofs;
				displayofs = bufferofs;
				bufferofs = temp;
				// TODO: Reduce by TickBase instead? Behaviors
				// might differ just a bit from the original
				// executables for DOS.
				SD_SetTimeCount(0);
				UpdateTimer();
				VW_SetScreen(bufferofs, 0);
				temp = displayofs;
				displayofs = bufferofs;
				bufferofs = temp;
				if (timeleft >= 0)
				{
					timeleft--;
				}
			}
#endif
			switch (LastScan)
			{
			case sc_UpArrow:
			case sc_LeftArrow:
			case sc_PgUp:
				if (pagenum > 1)
				{
					BackPage();
					BackPage();
					newpage = true;
				}
				break;
			case sc_DownArrow:
			case sc_RightArrow:
			case sc_PgDn:
				if (pagenum < numpages)
				{
					newpage = true;
				}
				break;
#ifndef BETA
			case sc_Escape:
				if (timeleft == -1)
				{
					goto artdone;
				}
				break;
#endif
			}
#ifdef BETA
#define CONDITION (LastScan != sc_Escape)
#else
#define CONDITION (true)
#endif
		} while (CONDITION);
#undef CONDITION
#ifdef BETA
		MM_FreePtr(&buffer);
		IN_ClearKeysDown();
#else
artdone:
		MM_FreePtr(&grsegs[pos]);
		IN_ClearKeysDown();
		if (timerdone == true)
		{
			timerdone = false;
			CA_DownLevel();
			IN_ClearKeysDown();
			bufferofs = oldbufferofs;
			displayofs = olddisplayofs;
			fontnumber = oldfontnumber;
			VW_ClearVideo(BACKCOLOR);
			RF_FixOfs();
			SD_FadeOutMusic();
			while (SD_MusicPlaying())
				BE_ST_ShortSleep();
/*
	Note:
	It would be better to call StopMusic() before calling CA_DownLevel()
	to avoid "out of memory" crashes. The only downside of this is that
	there would be no music while the game re-caches the graphics.
	However, that is just a minor inconvenience compared to having the
	game crash.
*/
			StopMusic();
			break;
		}
#endif
	} while (1);
#ifndef BETA
	return;
#endif
}

//===========================================================================

/*
=================
=
= FinaleLayout
=
=================
*/

void FinaleLayout(void)
{
	id0_char_t id0_seg *textseg;
	Sint16 i;

	VW_ClearVideo(BACKCOLOR);
	RF_FixOfs();
	CA_UpLevel();
	CA_SetGrPurge();	// redundant! CA_UpLevel already does this in BioMenace
	// BUG: should call CA_ClearMarks() here to make sure we don't cache more than we need
	CA_CacheGrChunk(TEXTARROW2PIC);
	CA_CacheGrChunk(TEXTARROW1PIC);
	// BUG: the text's grsegs pointer should be locked to make sure it doesn't
	// get moved around when caching the graphics
#ifdef BETA
	CA_LoadFile("ENDART.BH1", (memptr *)&textseg);
#elif (EPISODE == 2)
	if (gamestate.nukestate != 2)
	{
		CA_CacheGrChunk(BADENDINGTEXT);
		textseg = (id0_char_t id0_seg *)grsegs[BADENDINGTEXT];
	}
	else
	{
		CA_CacheGrChunk(ENDINGTEXT);
		textseg = (id0_char_t id0_seg *)grsegs[ENDINGTEXT];
	}
#else
	CA_CacheGrChunk(ENDINGTEXT);
	textseg = (id0_char_t id0_seg *)grsegs[ENDINGTEXT];
#endif
	text = textseg;
	CacheLayoutGraphics();
#if (EPISODE == 1)
	StartMusic(MUS_HIGHSCORELEVEL);
#elif (EPISODE == 2)
	StartMusic(MUS_HOSTAGE);
#elif (EPISODE == 3)
	StartMusic(MUS_LEVEL5);
#endif
	while (pagenum < numpages)
	{
		PageLayout(false);
		IN_ClearKeysDown();
		VW_SetScreen(bufferofs, 0);
		do {
			VWB_DrawPic(298, 184, TEXTARROW1PIC);
			for (i=0; i<TickBase; i++)
			{
				if (IN_IsUserInput())
				{
					goto nextpage;
				}
				VW_WaitVBL(1);
			}
			VWB_DrawPic(298, 184, TEXTARROW2PIC);
			for (i=0; i<TickBase; i++)
			{
				if (IN_IsUserInput())
				{
					goto nextpage;
				}
				VW_WaitVBL(1);
			}
		} while (1);
nextpage:;
	}
	StopMusic();
#ifdef BETA
	MM_FreePtr((memptr *)&textseg);
#elif (EPISODE == 2)
	if (gamestate.nukestate != 2)
	{
		MM_FreePtr(&grsegs[BADENDINGTEXT]);
	}
	else
	{
		MM_FreePtr(&grsegs[ENDINGTEXT]);
	}
#else
	MM_FreePtr(&grsegs[ENDINGTEXT]);
#endif
	MM_FreePtr(&grsegs[TEXTARROW1PIC]);
	MM_FreePtr(&grsegs[TEXTARROW2PIC]);
	CA_DownLevel();
	IN_ClearKeysDown();
	VW_ClearVideo(BACKCOLOR);
	RF_FixOfs();
	CA_FreeGraphics();
}

REFKEEN_NS_E
