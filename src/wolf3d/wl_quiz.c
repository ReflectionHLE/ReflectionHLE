/*** S3DNA RESTORATION - Recreated quiz source file ***/

#include "wl_def.h"
//#pragma hdrstop

extern id0_int_t gotgatgun;

static CP_itemtype
QuizMenu[]=
{
	1,"1",NULL,
	1,"2",NULL,
	1,"3",NULL,
	1,"4",NULL
};

static CP_iteminfo QuizItems = {30,96,4,0,24};


void RestorePlayScreen (void)
{
	id0_int_t i;
	id0_unsigned_t temp;

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

	DrawTreasure ();
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
// It turns out all data was defined in it own far segment in a separate ASM
// file, named QUESTION.ASM (based on debugging info embedded in the EXE).

extern id0_char_t	id0_far * id0_far Question[];

id0_int_t AskQuestion (id0_int_t question)
{
	id0_int_t answer, correct;
	id0_char_t id0_far *p;
	id0_char_t *p2;

	if (question >= 99)
	{
		sprintf (str,"AskQuestion(): Invalid question %d!",question);
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

	sprintf (str,"Question #%d",question+1);
	US_CPrint (str);

	WindowX = PrintX = 26;
	WindowY = PrintY = 26;
	WindowW = 276;
	WindowH = 132;

	US_Print (Question[question]);
	p = Question[question];
	while (*p)
		p++;
	p++;

	if (gamestate.difficulty <= gd_easy)
	{
		// Print reference
		US_Print ("\n(");
		US_Print (p);
		US_Print (")");
	}

	while (*p)
		p++;
	p++;

	// At this stage p points to the first answer
	answer = 0;
	correct = 4;

	for (; answer < 4;)
	{
		p2 = QuizMenu[answer].string;
		while (*p > 4)
			*p2++ = *p++;
		*p2 = '\0';
		if (*p & 1)
			correct = answer;

		if (*p >= 3)
			break;

		if (Question[question+1] <= p)
		{
			sprintf (str,"AskQuestion(): Question %d has no end!",question);
			Quit (str);
		}
		p++; answer++;
	}

	if (answer == 4)
	{
		sprintf (str,"AskQuestion(): Question %d has too many answers!",question);
		Quit (str);
	}
	if (correct == 4)
	{
		sprintf (str,"AskQuestion(): Question %d has no correct answer!",question);
		Quit (str);
	}
	// So the correct answer is the one followed by an odd number. 3 or 4 is used to terminate the list.
	QuizItems.amount = answer+1;
	QuizItems.curpos = 0;
	CacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
	DrawMenu (&QuizItems,&QuizMenu);
	VW_UpdateScreen ();
	WaitKeyUp ();
	answer = HandleMenu (&QuizItems,&QuizMenu,NULL);

	DrawWindow (14,21,292,134,BKGDCOLOR);
	WindowX = PrintX = 0;
	WindowY = PrintY = 88;

	if (answer == correct)
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
		SD_PlaySound (PERCENT100SND);
	}
	else if (answer != -1)
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
	RestorePlayScreen ();
	ClearMemory ();
	PM_CheckMainMem ();
	lasttimecount = TimeCount;

	if (MousePresent)
		Mouse(MDelta);	// Clear accumulated mouse movement

	if (answer == -1)
		return answer;
	if (answer == correct)
		return 1;
	return 0;
}
