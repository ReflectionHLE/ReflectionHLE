/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
 * Copyright (C) 2025 NY00123
 *
 * This file is loosely based on:
 * Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
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
// uninitialized variables:
/////////////////////////////////////////////////////////////////////////////

boolean scorescreenkludge;

/////////////////////////////////////////////////////////////////////////////
// functions:
/////////////////////////////////////////////////////////////////////////////

/*
============================
=
= CheckLastScan
=
============================
*/

void CheckLastScan(void)
{
	if (LastScan)
	{
		if (storedemo)
		{
			playstate = ex_resetgame;
			restartgame = gd_Normal;
			IN_ClearKeysDown();
			NewGame();
			return;
		}
#ifdef BETA
		if (LastScan == sc_F1)
		{
			HelpScreens();
			StartMusic(MUS_HIGHSCORELEVEL);
			return;
		}
#endif
		US_ControlPanel(0);
#ifdef BETA
		if (restartgame)
		{
			playstate = ex_resetgame;
		}
		else if (loadedgame)
		{
			playstate = ex_loadedgame;
		}
#else
		ResetScoreObj();
#endif
	}
}

//===========================================================================

/*
============================
=
= TitleScreen
=
============================
*/

#ifdef BETA
void TitleScreen(void)
{
	VW_FadeOut();
	RF_FixOfs();
	panadjust = 0;
	CA_CacheGrChunk(TITLEPIC);
	VWB_DrawPic(0, 0, TITLEPIC);
	VW_SetScreen(displayofs, 0);
	VW_ScreenToScreen(bufferofs, displayofs, 42, 224);
	VW_FadeIn();
	IN_UserInput(6*TickBase, false);
	CA_ClearMarks();
	CheckLastScan();
}
#else
void TitleScreen(Uint16 seconds)
{
	VW_FadeOut();
	RF_FixOfs();
	panadjust = 0;
#if ((EPISODE == 1) && (!SHAREWARE))
	CA_CacheGrChunk(REGTITLEPIC);
	VWB_DrawPic(0, 0, REGTITLEPIC);
#else
	CA_CacheGrChunk(TITLEPIC);
	VWB_DrawPic(0, 0, TITLEPIC);
#endif
	VW_SetScreen(displayofs, 0);
	VW_ScreenToScreen(bufferofs, displayofs, 42, 224);
	VW_FadeIn();
	IN_UserInput(seconds*TickBase, false);
	CA_ClearMarks();
	CheckLastScan();
}
#endif	// ifdef BETA ... else ...

//===========================================================================

/*
============================
=
= MiscScreen
=
============================
*/

#ifdef BETA
void MiscScreen(void)
{
	VW_FadeOut();
	RF_FixOfs();
	panadjust = 0;
	CA_CacheGrChunk(PIRACYPIC);
	VWB_DrawPic(0, 0, PIRACYPIC);
	VW_SetScreen(displayofs, 0);
	VW_ScreenToScreen(bufferofs, displayofs, 42, 224);
	VW_FadeIn();
	IN_UserInput(6*TickBase, false);
	CA_ClearMarks();
	CheckLastScan();
}
#else
void MiscScreen(Sint16 num)
{
	VW_FadeOut();
	RF_FixOfs();
	panadjust = 0;
	if (num == 0)
	{
#if (EPISODE == 1)
		CA_CacheGrChunk(PREVIEWPIC);
		VWB_DrawPic(0, 0, PREVIEWPIC);
#endif
	}
	else if (num == 1)
	{
		CA_CacheGrChunk(CREDITSPIC);
		VWB_DrawPic(0, 0, CREDITSPIC);
	}
	else if (num == 2)
	{
		CA_CacheGrChunk(PIRACYPIC);
		VWB_DrawPic(0, 0, PIRACYPIC);
	}
#if (EPISODE == 1 && (defined VERSION_1_1))
	else if (num == 3)
	{
		CA_CacheGrChunk(TECHHELPPIC);
		VWB_DrawPic(0, 0, TECHHELPPIC);
	}
#ifdef GMS_VERSION
	else if (num == 4)
	{
		CA_CacheGrChunk(GMSPIC);
		VWB_DrawPic(0, 0, GMSPIC);
	}
#endif
#endif
	VW_SetScreen(displayofs, 0);
	VW_ScreenToScreen(bufferofs, displayofs, 42, 224);
	VW_FadeIn();
#ifdef VERSION_1_1
#ifdef GMS_VERSION
	if (num != 3)
	{
		IN_UserInput(6*TickBase, false);
	}
	else
	{
		IN_ClearKeysDown();
		IN_Ack();
	}
#else
	if (num < 3)
	{
		IN_UserInput(6*TickBase, false);
	}
	else
	{
		IN_ClearKeysDown();
		IN_Ack();
	}
#endif
#else
	IN_UserInput(6*TickBase, false);
#endif
	CA_ClearMarks();
	CheckLastScan();
}
#endif	// ifdef BETA ... else ...

//===========================================================================

/*
============================
=
= RunDemo
=
============================
*/

void RunDemo(Sint16 num)
{
#ifdef BETA
	Sint16 handle;
	char demofile[] = "DEMO?."EXTENSION;
	
	demofile[4] = num+'0';
	handle = open(demofile, O_BINARY|O_RDONLY, S_IFREG|S_IREAD|S_IWRITE);
	if (handle == -1)
	{
		strcpy(str, "RunDemo:  Cannot open ");
		strcat(str, demofile);
		Quit(str);
	}
	NewGame();
	read(handle, &gamestate.mapon, sizeof(gamestate.mapon));
	read(handle, &DemoSize, sizeof(DemoSize));
	MM_GetPtr((memptr *)&DemoBuffer, DemoSize);
	MM_SetLock((memptr *)&DemoBuffer, true);
	CA_FarRead(handle, DemoBuffer, DemoSize);
	close(handle);
	IN_StartDemoPlayback(DemoBuffer, DemoSize);
	SetupGameLevel(true);
	if (scorescreenkludge)
	{
		DrawHighScores();
	}
	PlayLoop();
	IN_StopDemo();
	MM_SetLock((memptr *)&DemoBuffer, false);	// not required, MM_FreePtr() can free locked buffers
	MM_FreePtr((memptr *)&DemoBuffer);
	CA_ClearMarks();
	CheckLastScan();
#else
	Uint16 demonum;
	Uint16 id0_far *demodata;
	gametype oldgamestate;
	
	demonum = num;
	if (demonum != DEMO_LEVELWARP)
	{
		NewGame();
	}
	else
	{
		memcpy(&oldgamestate, &gamestate, sizeof(gamestate));
	}
	num += HIGHSCOREDEMO;
	CA_CacheGrChunk(num);
	demodata = (Uint16 id0_seg*)grsegs[num];
	gamestate.mapon = demodata[0];
	DemoSize = demodata[1];
	MM_GetPtr((memptr *)&DemoBuffer, DemoSize);
	MM_SetLock((memptr *)&DemoBuffer, true);
	memcpy(DemoBuffer, ((id0_char_t id0_seg *)(grsegs[num]))+4, DemoSize);
//	_fmemcpy(DemoBuffer, ((id0_char_t id0_seg *)(grsegs[num]))+4, DemoSize);
	MM_FreePtr(&grsegs[num]);
	IN_StartDemoPlayback(DemoBuffer, DemoSize);
	SetupGameLevel(true);
	if (scorescreenkludge)
	{
		DrawHighScores();
	}
	PlayLoop();
	IN_StopDemo();
	MM_FreePtr((memptr *)&DemoBuffer);
	VW_FixRefreshBuffer();
	CA_ClearMarks();
	if (demonum != DEMO_APOGEE && demonum != DEMO_LEVELWARP)
	{
		CheckLastScan();
	}
	if (demonum == DEMO_LEVELWARP)
	{
		memcpy(&gamestate, &oldgamestate, sizeof(gamestate));
	}
#endif	// ifdef BETA ... else ...
}

//===========================================================================

/*
============================
=
= DrawHighScores
=
============================
*/

void DrawHighScores(void)
{
	Uint16 i;
	char *bufptr;
	Uint16 width, height;
	HighScore *entry;
	Uint16 oldbufferofs;
	char buf[16];
	
	RF_NewPosition(0, 0);
	oldbufferofs = bufferofs;
	bufferofs = masterofs;
	for (i=0, entry=&Scores[0]; i<8; i++, entry++)
	{
		PrintY = i*16+35;
		PrintX = 40;
		US_Print(entry->name);
		BE_Cross_ultoa_dec(entry->score, buf);
		for (bufptr=buf; *bufptr; bufptr++)
		{
			*bufptr = *bufptr + 81;
		}
		USL_MeasureString(buf, NULL, &width, &height);
		PrintX = 280-width;
		US_Print(buf);
	}
	fontcolor = WHITE;
	bufferofs = oldbufferofs;
}

//===========================================================================

/*
============================
=
= CheckHighScore
=
============================
*/

void CheckHighScore(Sint32 score, Sint16 completed)
{
	Uint16 i, n;
	Sint16 index;
	HighScore entry;
	
	strcpy(entry.name, "");	//Note: 'entry.name[0] = 0;' would be more efficient
	entry.score = score;
	entry.completed = completed;
	for (i=0, index=-1; i<MaxScores; i++)
	{
		if (Scores[i].score < entry.score ||
			(Scores[i].score == entry.score && Scores[i].completed < entry.completed))
		{
			for (n=MaxScores; --n > i;)
			{
				Scores[n] = Scores[n-1];
			}
			Scores[i] = entry;
			index = i;
			HighScoresDirty = true;
			break;
		}
	}
	if (index != -1)
	{
		scorescreenkludge = true;
		gamestate.mapon = 12;
		SetupGameLevel(true);
		DrawHighScores();
		RF_Refresh();
		RF_Refresh();
		PrintY = i*16+35;
		PrintX = 40;
		US_LineInput(PrintX, PrintY, Scores[index].name, NULL, true, MaxHighName, 112);
		scorescreenkludge = false;
	}
}

//===========================================================================

/*
============================
=
= ShowHighScores
=
============================
*/

void ShowHighScores(void)
{
	scorescreenkludge = true;
	IN_ClearKeysDown();
	RunDemo(DEMO_HIGHSCORE);
	scorescreenkludge = false;
}

REFKEEN_NS_E
