/*** S3DNA RESTORATION - Recreated quiz source file ***/

#include "WL_DEF.H"
#pragma hdrstop

extern int gotgatgun;

CP_itemtype
QuizMenu[]=
{
	1,"1",NULL,
	1,"2",NULL,
	1,"3",NULL,
	1,"4",NULL
};

CP_iteminfo QuizItems = {30,96,4,0,24};


void CleanupQuiz (void)
{
	int i;
	unsigned temp;

	temp = bufferofs;
	CA_CacheGrChunk (STATUSBARPIC);
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
		VWB_DrawPic (0,160,STATUSBARPIC);
	}
	UNCACHEGRCHUNK (STATUSBARPIC);
	bufferofs = temp;

	DrawFruit ();
	DrawFace ();
	DrawHealth ();
	DrawLives ();
	DrawLevel ();
	DrawAmmo ();
	DrawKeys ();
	DrawScore ();
}

// S3DNA RESTORATION - This one is a bit tricky, since each question is stored
// in the EXE in a NULL-terminated form, followed by a NULL-terminated Bible
// reference, and then followed by NON-null terminated possible answers.
// In addition, that data is preceded by an array of far pointers to the
// questions. Finally, all data and pointers are stored in single segment.
//
// Maybe the raw data was stored in an external file as-is, and pointers to
// pieces of the data were defined. Still, both the data and the pointers
// should be in the same segment.

// TODO How to store (also, chunks were split based on '4' separator,
// but it looks like this isn't necessarily the right way)

extern char	far * far quizdata[];

int AskQuestion (int number)
{
	int which, numitems;
	char far *s;
	char *se, *sq;

	if (number >= 99)
	{
		sprintf (str,"AskQuestion(): Invalid question %d!",number);
		Quit (str);
	}
	FinishPaletteShifts ();
	ClearMemory ();
	ClearSplitVWB ();
	VWB_Bar (0,0,320,160,VIEWCOLOR);
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	DrawWindow (14,21,292,134,BKGDCOLOR);
	CA_CacheGrChunk (STARTFONT+1);
	fontnumber = 1;

	WindowX = PrintX = 0;
	WindowY = PrintY = 4;
	WindowW = 320;
	WindowH = 160;

	sprintf (str,"Question #%d",number+1);
	US_CPrint (str);

	WindowX = PrintX = 26;
	WindowY = PrintY = 26;
	WindowW = 276;
	WindowH = 132;

	US_Print (quizdata[number]);
	s = quizdata[number];
	while (*s)
		s++;
	s++;

	if (gamestate.difficulty <= gd_easy)
	{
		// Print reference
		US_Print ("\n(");
		US_Print (s);
		US_Print (")");
	}

	while (*s)
		s++;
	s++;

	// At this stage s points to the first answer
	which = 0;
	numitems = 4;
	se = QuizMenu[0].string;
	while (which < 4)
	{
		sq = se;
		while (*s > 4)
			*sq++ = *s++;
		*sq = '\0';
		if (*s & 1)
			numitems = which;

		if (*s >= 3)
			break;

		if (quizdata[number+1] <= s)
		{
			sprintf (str,"AskQuestion(): Question %d has no end!",number);
			Quit (str);
		}
		s++;
		se += sizeof(CP_itemtype);
		which++;
	}
	if (which == 4)
	{
		sprintf (str,"AskQuestion(): Question %d has too many answers!",number);
		Quit (str);
	}
	if (numitems == 4)
	{
		sprintf (str,"AskQuestion(): Question %d has no correct answer!",number);
		Quit (str);
	}
	// So the correct answer is the one followed by an odd number. 3 or 4 is used to terminate the list.
	QuizItems.amount = which+1;
	QuizItems.curpos = 0;
	CacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
	DrawMenu (&QuizItems,&QuizMenu);
	VW_UpdateScreen ();
	WaitKeyUp ();
	which = HandleMenu (&QuizItems,&QuizMenu,NULL);

	DrawWindow (14,21,292,134,BKGDCOLOR);
	WindowX = PrintX = 0;
	WindowY = PrintY = 88;

	if (which == numitems)
	{
		US_CPrint ("Correct!");
		VW_UpdateScreen ();
		GivePoints (1000);
		if (gamestate.health < 100)
		{
			HealSelf (25);
			GiveAmmo (10);
		}
		else
		{
			GiveAmmo (25);
		}
		StatusDrawPic (19,4,GOTGATLINGPIC);
		facecount = 0;
		gotgatgun = 1;
		SD_PlaySound (ENDBONUS2SND);
	}
	else if (which != -1)
	{
		US_CPrint ("Incorrect!");
		VW_UpdateScreen ();
		SD_PlaySound (NOBONUSSND);
		HealSelf (4);
	}

	while (SD_SoundPlaying())
		;
	VL_WaitVBL(30); // VBLWAIT in WL_INTER.C

	IN_ClearKeysDown ();
	UnCacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
	CleanupQuiz ();
	ClearMemory ();
	PM_CheckMainMem ();
	lasttimecount = TimeCount;

	if (MousePresent)
		Mouse(MDelta);	// Clear accumulated mouse movement

	if (which == -1)
		return which;
	if (which == numitems)
		return 1;
	return 0;
}
