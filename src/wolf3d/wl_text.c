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

// WL_TEXT.C

#include "wl_def.h"
//#pragma	hdrstop

#ifndef SPEAR // REFKEEN: Disable all code for what doesn't need it

/*
=============================================================================

TEXT FORMATTING COMMANDS
------------------------
^C<hex digit>  			Change text color
^E[enter]				End of layout (all pages)
^G<y>,<x>,<pic>[enter]	Draw a graphic and push margins
^P[enter]				start new page, must be the first chars in a layout
^L<x>,<y>[ENTER]		Locate to a specific spot, x in pixels, y in lines

=============================================================================
*/

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define BACKCOLOR		4
#else
#define BACKCOLOR		0x11
#endif


#define WORDLIMIT		80
#define FONTHEIGHT		10
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#define	TOPMARGIN		10
#define BOTTOMMARGIN		10
#define LEFTMARGIN		10
#define RIGHTMARGIN		10
#else
#define	TOPMARGIN		16
#define BOTTOMMARGIN	32
#define LEFTMARGIN		16
#define RIGHTMARGIN		16
#endif
#define PICMARGIN		8
#define TEXTROWS		((200-TOPMARGIN-BOTTOMMARGIN)/FONTHEIGHT)
#define	SPACEWIDTH		7
#define SCREENPIXWIDTH	320
#define SCREENMID		(SCREENPIXWIDTH/2)

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

id0_int_t			pagenum,numpages;

id0_unsigned_t	leftmargin[TEXTROWS],rightmargin[TEXTROWS];
id0_char_t		id0_far *text;
id0_unsigned_t	rowon;

id0_int_t			picx,picy,picnum,picdelay;
id0_boolean_t		layoutdone;

//===========================================================================

#ifndef JAPAN
/*
=====================
=
= RipToEOL
=
=====================
*/

void RipToEOL (void)
{
	while (*text++ != '\n')		// scan to end of line
	;
}


/*
=====================
=
= ParseNumber
=
=====================
*/

id0_int_t	ParseNumber (void)
{
	id0_char_t	ch;
	id0_char_t	num[80],*numptr;

//
// scan until a number is found
//
	ch = *text;
	while (ch < '0' || ch >'9')
		ch = *++text;

//
// copy the number out
//
	numptr = num;
	do
	{
		*numptr++ = ch;
		ch = *++text;
	} while (ch >= '0' && ch <= '9');
	*numptr = 0;

	return atoi (num);
}



/*
=====================
=
= ParsePicCommand
=
= Call with text pointing just after a ^P
= Upon exit text points to the start of next line
=
=====================
*/

void	ParsePicCommand (void)
{
	picy=ParseNumber();
	picx=ParseNumber();
	picnum=ParseNumber();
	RipToEOL ();
}


void	ParseTimedCommand (void)
{
	picy=ParseNumber();
	picx=ParseNumber();
	picnum=ParseNumber();
	picdelay=ParseNumber();
	RipToEOL ();
}


/*
=====================
=
= TimedPicCommand
=
= Call with text pointing just after a ^P
= Upon exit text points to the start of next line
=
=====================
*/

void	TimedPicCommand (void)
{
	ParseTimedCommand ();

//
// update the screen, and wait for time delay
//
	VW_UpdateScreen ();

//
// wait for time
//
	SD_AddToTimeCount(-SD_GetTimeCount());
	SD_TimeCountWaitForDest(picdelay);
#if 0
	TimeCount = 0;
	while (TimeCount < picdelay)
	;
#endif

//
// draw pic
//
	VWB_DrawPic (picx&~7,picy,picnum);
}


/*
=====================
=
= HandleCommand
=
=====================
*/

void HandleCommand (void)
{
	id0_int_t	i,margin,top,bottom;
	id0_int_t	picwidth,picheight,picmid;

	switch (toupper(*++text))
	{
	case 'B':
		picy=ParseNumber();
		picx=ParseNumber();
		picwidth=ParseNumber();
		picheight=ParseNumber();
		VWB_Bar(picx,picy,picwidth,picheight,BACKCOLOR);
		RipToEOL();
		break;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case ';':		// comment
		RipToEOL();
		break;
#endif
	case 'P':		// ^P is start of next page, ^E is end of file
	case 'E':
		layoutdone = true;
		text--;    	// back up to the '^'
		break;

	case 'C':		// ^c<hex digit> changes text color
		i = toupper(*++text);
		if (i>='0' && i<='9')
			fontcolor = i-'0';
		else if (i>='A' && i<='F')
			fontcolor = i-'A'+10;

		// *** ALPHA RESTORATION ***
		// Is this EGA-era?
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		fontcolor ^= 4;
#else
		fontcolor *= 16;
		i = toupper(*++text);
		if (i>='0' && i<='9')
			fontcolor += i-'0';
		else if (i>='A' && i<='F')
			fontcolor += i-'A'+10;
#endif
		text++;
		break;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	case '>':
		px = 160;
		text++;
		break;
#endif

	case 'L':
		py=ParseNumber();
		rowon = (py-TOPMARGIN)/FONTHEIGHT;
		py = TOPMARGIN+rowon*FONTHEIGHT;
		px=ParseNumber();
		while (*text++ != '\n')		// scan to end of line
		;
		break;

	case 'T':		// ^Tyyy,xxx,ppp,ttt waits ttt tics, then draws pic
		TimedPicCommand ();
		break;

	case 'G':		// ^Gyyy,xxx,ppp draws graphic
		ParsePicCommand ();
		VWB_DrawPic (picx&~7,picy,picnum);
		// *** ALPHA RESTORATION ***
		// Another EGA thing
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		picwidth = 8*pictable[picnum-STARTPICS].width;
#else
		picwidth = pictable[picnum-STARTPICS].width;
#endif
		picheight = pictable[picnum-STARTPICS].height;
		//
		// adjust margins
		//
		picmid = picx + picwidth/2;
		if (picmid > SCREENMID)
			margin = picx-PICMARGIN;			// new right margin
		else
			margin = picx+picwidth+PICMARGIN;	// new left margin

		top = (picy-TOPMARGIN)/FONTHEIGHT;
		if (top<0)
			top = 0;
		bottom = (picy+picheight-TOPMARGIN)/FONTHEIGHT;
		if (bottom>=TEXTROWS)
			bottom = TEXTROWS-1;

		for (i=top;i<=bottom;i++)
			if (picmid > SCREENMID)
				rightmargin[i] = margin;
			else
				leftmargin[i] = margin;

		//
		// adjust this line if needed
		//
		if (px < leftmargin[rowon])
			px = leftmargin[rowon];
		break;
	}
}


/*
=====================
=
= NewLine
=
=====================
*/

void NewLine (void)
{
	id0_char_t	ch;

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
				ch = toupper(*(text+1));
				if (ch == 'E' || ch == 'P')
				{
					layoutdone = true;
					return;
				}
			}
			text++;

		} while (1);

	}
	px = leftmargin[rowon];
	py+= FONTHEIGHT;
}



/*
=====================
=
= HandleCtrls
=
=====================
*/

void HandleCtrls (void)
{
	id0_char_t	ch;

	ch = *text++;			// get the character and advance

	if (ch == '\n')
	{
		NewLine ();
		return;
	}

}


/*
=====================
=
= HandleWord
=
=====================
*/

void HandleWord (void)
{
	id0_char_t		word[WORDLIMIT];
	id0_int_t			i,wordindex;
	id0_unsigned_t	wwidth,wheight,newpos;


	//
	// copy the next word into [word]
	//
	word[0] = *text++;
	wordindex = 1;
	while (*text>32)
	{
		word[wordindex] = *text++;
		if (++wordindex == WORDLIMIT)
			Quit ("PageLayout: Word limit exceeded");
	}
	word[wordindex] = 0;		// stick a null at end for C

	//
	// see if it fits on this line
	//
	VW_MeasurePropString (word,NULL,&wwidth,&wheight);

	while (px+wwidth > rightmargin[rowon])
	{
		NewLine ();
		if (layoutdone)
			return;		// overflowed page
	}

	//
	// print it
	//
	newpos = px+wwidth;
	VWB_DrawPropString (word,NULL);
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
=====================
=
= PageLayout
=
= Clears the screen, draws the pics on the page, and word wraps the text.
= Returns a pointer to the terminating command
=
=====================
*/

void PageLayout (id0_boolean_t shownumber)
{
	id0_int_t		i,oldfontcolor;
	id0_char_t	ch;

	oldfontcolor = fontcolor;

	// *** ALPHA RESTORATION *** 
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	fontcolor = 0x0a;
#else
	fontcolor = 0;
#endif

//
// clear the screen
//
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Bar (0,0,320,200,0x24);
#else
	VWB_Bar (0,0,320,200,BACKCOLOR);
#endif
	// *** SOD V1.4 ACTIVISION + ALPHA RESTORATION ***
	// Interestingly this was commented out for the SOD Activision
	// release, instead of omitting WL_TEXT.C from the project
	// (also unused in the Wolf3D GT and Activision builds).
#if ((GAMEVER_WOLFREV <= GV_WR_WL6GT14B) || (!defined SPEAR)) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	VWB_DrawPic (0,0,H_TOPWINDOWPIC);
	VWB_DrawPic (0,8,H_LEFTWINDOWPIC);
	VWB_DrawPic (312,8,H_RIGHTWINDOWPIC);
	VWB_DrawPic (8,176,H_BOTTOMINFOPIC);
#endif

	for (i=0;i<TEXTROWS;i++)
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
	while (*text <= 32)
		text++;

	if (*text != '^' || toupper(*++text) != 'P')
		Quit ("PageLayout: Text not headed with ^P");

	while (*text++ != '\n')
	;


//
// process text stream
//
	do
	{
		ch = *text;

		if (ch == '^')
			HandleCommand ();
		// *** ALPHA RESTIRATION *** 
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		else
		if (ch == 9)
		{
		 px = (px+8)&0xf8;
		 text++;
		}
#endif
		else if (ch <= 32)
			HandleCtrls ();
		else
			HandleWord ();

	} while (!layoutdone);

	pagenum++;

	if (shownumber)
	{
		// *** APOGEE + EARLY GOODTIMES + ID RELEASES + S3DNA RESTORATION ***
		// Micro difference - where/when to set px and py...
		#ifdef SPANISH
		strcpy (str,"Hoja ");
		BE_Cross_itoa_dec (pagenum,str2);
		strcat (str,str2);
		strcat (str," de ");
		// *** APOGEE + EARLY GOODTIMES + ID RELEASES + S3DNA RESTORATION ***
		// Pick location based on version
		#if (GAMEVER_WOLFREV > GV_WR_N3DWT10)
		py = 183;
		px = 208;
		#endif
		#else
		strcpy (str,"pg ");
		BE_Cross_itoa_dec (pagenum,str2);
		strcat (str,str2);
		strcat (str," of ");
		// *** APOGEE + EARLY GOODTIMES + ID RELEASES + S3DNA RESTORATION ***
		// Pick location based on version
		#if (GAMEVER_WOLFREV > GV_WR_N3DWT10)
		py = 183;
		px = 213;
		#endif
		#endif
		BE_Cross_itoa_dec (numpages,str2);
		strcat (str,str2);
		// *** ALPHA RESTORATION *** 
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		fontcolor = 8;
		py = 186;
		px = 218;
#else
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		fontcolor = 0x10;
#else
		fontcolor = 0x4f; 			   //12^BACKCOLOR;
#endif
		// *** APOGEE + EARLY GOODTIMES + ID RELEASES + S3DNA RESTORATION ***
		// Pick location based on version
		#if (GAMEVER_WOLFREV <= GV_WR_N3DWT10)
		#ifdef SPANISH
		py = 183;
		px = 208;
		#else
		py = 183;
		px = 213;
		#endif
		#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312

		VWB_DrawPropString (str,NULL);
	}

	fontcolor = oldfontcolor;
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

void BackPage (void)
{
	pagenum--;
	do
	{
		text--;
		if (*text == '^' && toupper(*(text+1)) == 'P')
			return;
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
void CacheLayoutGraphics (void)
{
	id0_char_t	id0_far *bombpoint, id0_far *textstart;
	id0_char_t	ch;

	textstart = text;
	bombpoint = text+30000;
	numpages = pagenum = 0;

	do
	{
		if (*text == '^')
		{
			ch = toupper(*++text);
			if (ch == 'P')		// start of a page
				numpages++;
			if (ch == 'E')		// end of file, so load graphics and return
			{
				// *** SOD 1.4 ACTIVISION + ALPHA RESTORATION ***
				// Again, none of WL_TEXT.C should even be used in the Activision EXEs...
#if ((GAMEVER_WOLFREV <= GV_WR_WL6GT14B) || (!defined SPEAR)) && (GAMEVER_WOLFREV > GV_WR_WL920312)
				CA_MarkGrChunk(H_TOPWINDOWPIC);
				CA_MarkGrChunk(H_LEFTWINDOWPIC);
				CA_MarkGrChunk(H_RIGHTWINDOWPIC);
				CA_MarkGrChunk(H_BOTTOMINFOPIC);
#endif
				CA_CacheMarks ();
				text = textstart;
				return;
			}
			if (ch == 'G')		// draw graphic command, so mark graphics
			{
				ParsePicCommand ();
				CA_MarkGrChunk (picnum);
			}
			if (ch == 'T')		// timed draw graphic command, so mark graphics
			{
				ParseTimedCommand ();
				CA_MarkGrChunk (picnum);
			}
		}
		else
			text++;

	} while (text<bombpoint);

	Quit ("CacheLayoutGraphics: No ^E to terminate file!");
}
#endif


/*
=====================
=
= ShowArticle
=
=====================
*/

#ifdef JAPAN
void ShowArticle (id0_int_t which)
#else
void ShowArticle (id0_char_t id0_far *article)
#endif
{
	#ifdef JAPAN
	id0_int_t		snames[10] = {	H_HELP1PIC,
							H_HELP2PIC,
							H_HELP3PIC,
							H_HELP4PIC,
							H_HELP5PIC,
							H_HELP6PIC,
							H_HELP7PIC,
							H_HELP8PIC,
							H_HELP9PIC,
							H_HELP10PIC};
	id0_int_t		enames[14] = {
							0,0,
							#ifndef JAPDEMO
							C_ENDGAME1APIC,
							C_ENDGAME1BPIC,
							C_ENDGAME2APIC,
							C_ENDGAME2BPIC,
							C_ENDGAME3APIC,
							C_ENDGAME3BPIC,
							C_ENDGAME4APIC,
							C_ENDGAME4BPIC,
							C_ENDGAME5APIC,
							C_ENDGAME5BPIC,
							C_ENDGAME6APIC,
							C_ENDGAME6BPIC
							#endif
							};
	#endif
	id0_unsigned_t	oldfontnumber;
	id0_unsigned_t	temp;
	id0_boolean_t 	newpage,firstpage;

	#ifdef JAPAN
	pagenum = 1;
	if (!which)
		numpages = 10;
	else
		numpages = 2;

	#else

	text = article;
	oldfontnumber = fontnumber;
	fontnumber = 0;
	// *** ALPHA RESTIRATION *** 
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	CA_UpLevel ();
	CA_SetGrPurge ();
#else
	CA_MarkGrChunk(STARTFONT);
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_Bar (0,0,320,200,0x24);
#else
	VWB_Bar (0,0,320,200,BACKCOLOR);
#endif
	CacheLayoutGraphics ();
	#endif

	newpage = true;
	firstpage = true;

	do
	{
		if (newpage)
		{
			newpage = false;
			#ifdef JAPAN
			if (!which)
				CA_CacheScreen(snames[pagenum - 1]);
			else
				CA_CacheScreen(enames[which*2 + pagenum - 1]);
			#else
			PageLayout (true);
			#endif
			VW_UpdateScreen ();
			if (firstpage)
			{
				// *** ALPHA RESTORATION *** 
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
				VL_FadeIn(0,255,gamepal,30);
#else
				VL_FadeIn(0,255,gamepal,10);
#endif
				// VW_FadeIn ()
				firstpage = false;
			}
		}

		LastScan = 0;
		while (!LastScan)
			BE_ST_ShortSleep();

		switch (LastScan)
		{
		case sc_UpArrow:
		case sc_PgUp:
		case sc_LeftArrow:
			if (pagenum>1)
			{
				#ifndef JAPAN
				BackPage ();
				BackPage ();
				#else
				pagenum--;
				#endif
				newpage = true;
			}
			break;

		// *** ALPHA RESTIRATION *** 
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		case sc_Enter:
#endif
		case sc_DownArrow:
		case sc_PgDn:
		case sc_RightArrow:		// the text allready points at next page
			if (pagenum<numpages)
			{
				newpage = true;
				#ifdef JAPAN
				pagenum++;
				#endif
			}
			break;
		}

		// *** APOGEE VERSIONS + S3NA RESTORATION ***
		// This is also skipped in these versions
		#if (!defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A) && (!defined GAMEVER_NOAH3D)
		//#ifndef SPEAR
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
			PicturePause();
		#endif

	} while (LastScan != sc_Escape);

	// *** ALPHA RESTIRATION *** 
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	CA_DownLevel ();
#endif
	IN_ClearKeysDown ();
	fontnumber = oldfontnumber;
}


//===========================================================================

#ifndef JAPAN
#ifdef ARTSEXTERN
// *** PRE-V1.4 APOGEE RESTORATION *** - Change the the order of appearances endextern and helpextern in the v1.2 EXE's layout (and SPEAR wasn't ready for that)
// *** S3DNA RESTORATION *** - No T_ENDART1
#ifdef GAMEVER_NOAH3D
id0_int_t		helpextern = T_HELPART;
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
id0_int_t		helpextern = T_HELPART;
id0_int_t 	endextern = T_ENDART1;
#else
id0_int_t 	endextern = T_ENDART1;
#ifndef SPEAR
id0_int_t		helpextern = T_HELPART;
#endif
#endif // VERSIONS RESTORATION
#endif
// *** S3DNA RESTORATION + ALPHA *** - No T_ENDART1,
// and embedding extension + using char * in alpha (instead of char..[])
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
id0_char_t *helpfilename = "HELPART.WL1",
	 *orderfilename = "ORDERART.WL1",
	 *storyfilename = "STORYART.WL1",
	 *orderfilename2 = "ORDERART.WL1", // Weird one
	 *ckartfilename = "CKART.WL1"; // Commander Keen?
#elif (!defined GAMEVER_NOAH3D)
id0_char_t helpfilename[13] = "HELPART.",
	 endfilename[13] = "ENDART1.";
#endif
#endif

/*
=================
=
= HelpScreens
=
=================
*/
#ifndef SPEAR
void HelpScreens (void)
{
	id0_int_t			artnum;
	id0_char_t id0_far 	*text;
	memptr		layout;


	// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	CA_UpLevel ();
	MM_SortMem ();
#endif
#ifdef JAPAN
	ShowArticle (0);
	VW_FadeOut();
	FreeMusic ();
	CA_DownLevel ();
	MM_SortMem ();
#else




#ifdef ARTSEXTERN
	artnum = helpextern;
	CA_CacheGrChunk (artnum);
	text = (id0_char_t id0_seg *)grsegs[artnum];
	MM_SetLock (&grsegs[artnum], true);
#else
	CA_LoadFile (helpfilename,&layout);
	text = (id0_char_t id0_seg *)layout;
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	MM_SetLock (&layout, true);
#endif
#endif

	ShowArticle (text);

#ifdef ARTSEXTERN
	MM_FreePtr (&grsegs[artnum]);
#else
	MM_FreePtr (&layout);
#endif



	// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	MenuFadeOut();
#else
	VW_FadeOut();
#endif

	// *** S3DNA + ALPHA RESTORATION ***
#if (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
	FreeMusic ();
	CA_DownLevel ();
	MM_SortMem ();
#endif
#endif
}
#endif

// *** ALPHA RESTORATION ***
// This is almost identical to HelpScreens; Only the ART file differs.
// It looks like the function named survived in WL_DEF.H
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void OrderingInfo (void)
{
	id0_char_t id0_far 	*text;
	memptr		layout;

	CA_LoadFile (orderfilename,&layout);
	text = (id0_char_t id0_seg *)layout;

	ShowArticle (text);

	MM_FreePtr (&layout);

	VW_FadeOut();
}

// *** S3DNA (AND ALPHA) RESTORATION ***
#elif (!defined GAMEVER_NOAH3D)
//
// END ARTICLES
//
void EndText (void)
{
	id0_int_t			artnum;
	id0_char_t id0_far 	*text;
	memptr		layout;


	ClearMemory ();

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	ResetSplitScreen ();
#endif
	CA_UpLevel ();
	MM_SortMem ();
#ifdef JAPAN
	ShowArticle(gamestate.episode + 1);

	VW_FadeOut();

	SETFONTCOLOR(0,15);
	IN_ClearKeysDown();
	if (MousePresent)
		BE_ST_GetEmuAccuMouseMotion(NULL, NULL); // Clear accumulated mouse movement
//		Mouse(MDelta);	// Clear accumulated mouse movement

	FreeMusic ();
	CA_DownLevel ();
	MM_SortMem ();
#else



#ifdef ARTSEXTERN
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	artnum = endextern;
#else
	artnum = endextern+gamestate.episode;
#endif
	CA_CacheGrChunk (artnum);
	text = (id0_char_t id0_seg *)grsegs[artnum];
	MM_SetLock (&grsegs[artnum], true);
#else
	endfilename[6] = '1'+gamestate.episode;
	CA_LoadFile (endfilename,&layout);
	text = (id0_char_t id0_seg *)layout;
	MM_SetLock (&layout, true);
#endif

	ShowArticle (text);

#ifdef ARTSEXTERN
	MM_FreePtr (&grsegs[artnum]);
#else
	MM_FreePtr (&layout);
#endif


	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	MenuFadeOut();
#else
	VW_FadeOut();
#endif
	SETFONTCOLOR(0,15);
	IN_ClearKeysDown();
	if (MousePresent)
		BE_ST_GetEmuAccuMouseMotion(NULL, NULL); // Clear accumulated mouse movement
//		Mouse(MDelta);	// Clear accumulated mouse movement

	FreeMusic ();
	CA_DownLevel ();
	MM_SortMem ();
#endif
}
#endif

#endif // REFKEEN: Disable code when not required
