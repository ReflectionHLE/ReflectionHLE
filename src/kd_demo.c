/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// KD_DEMO.C

#include "kd_def.h"

#pragma hdrstop

#define RLETAG  0xABCD

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

//===========================================================================

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (void)
{
	id0_word_t    i;

	gamestate.worldx = 0;           // spawn keen at starting spot

	gamestate.mapon = 0;
	gamestate.score = 0;
	gamestate.nextextra = 20000;
	gamestate.lives = 3;
	gamestate.flowerpowers = gamestate.boobusbombs = 0;
	for (i = 0;i < GAMELEVELS;i++)
		gamestate.leveldone[i] = false;
}

//===========================================================================

/*
=====================
=
= WaitOrKey
=
=====================
*/

id0_int_t WaitOrKey (id0_int_t vbls)
{
	while (vbls--)
	{
		IN_ReadControl(0,&c);           // get player input
		if (LastScan || c.button0 || c.button1)
		{
			IN_ClearKeysDown ();
			return 1;
		}
		VW_WaitVBL(1);
	}
	return 0;
}

//===========================================================================

/*
=====================
=
= GameOver
=
=====================
*/

void
GameOver (void)
{
	VW_InitDoubleBuffer ();
	US_CenterWindow (16,3);

	US_PrintCentered("Game Over!");

	VW_UpdateScreen ();
	IN_ClearKeysDown ();
	IN_Ack ();

}


//===========================================================================

/*
==================
=
= StatusWindow
=
==================
*/

void StatusWindow (void)
{
	id0_word_t    x;

	// DEBUG - make this look better

	US_CenterWindow(22,7);
	US_CPrint("Status Window");

	WindowX += 8;
	WindowW -= 8;
	WindowY += 20;
	WindowH -= 20;
	PrintX = WindowX;
	PrintY = WindowY;

	VWB_DrawTile8(PrintX,PrintY,26);
	VWB_DrawTile8(PrintX + 8,PrintY,27);
	PrintX += 24;
	US_PrintUnsigned(gamestate.lives);
	US_Print("\n");

	VWB_DrawTile8(PrintX,PrintY,32);
	VWB_DrawTile8(PrintX + 8,PrintY,33);
	VWB_DrawTile8(PrintX,PrintY + 8,34);
	VWB_DrawTile8(PrintX + 8,PrintY + 8,35);
	PrintX += 24;
	US_PrintUnsigned(gamestate.boobusbombs);
	US_Print("\n");

	WindowX += 50;
	WindowW -= 50;
	PrintX = WindowX;
	PrintY = WindowY;

	fontcolor = F_FIRSTCOLOR;
	US_Print("Next ");
	fontcolor = F_BLACK;
	x = PrintX;
	VWB_DrawTile8(PrintX,PrintY,26);
	VWB_DrawTile8(PrintX + 8,PrintY,27);
	PrintX += 24;
	US_PrintUnsigned(gamestate.nextextra);
	US_Print("\n");

	PrintX = x;
	VWB_DrawTile8(PrintX,PrintY,24);
	VWB_DrawTile8(PrintX + 8,PrintY,25);
	PrintX += 24;
	US_PrintUnsigned(gamestate.keys);
	US_Print("\n");

	// DEBUG - add flower powers (#36)

	VW_UpdateScreen();
	IN_Ack();
}

id0_boolean_t
SaveGame(id0_int_t file)
{
	id0_word_t    i,size,compressed,expanded;
	objtype *o;
	memptr  bigbuffer;

	if (!CA_FarWrite(file,(void id0_far *)&gamestate,sizeof(gamestate)))
		return(false);

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i++)   // Write all three planes of the map
	{
//
// leave a id0_word_t at start of compressed data for compressed length
//
		compressed = CA_RLEWCompress ((id0_unsigned_t id0_huge *)mapsegs[i]
			,expanded,((id0_unsigned_t id0_huge *)bigbuffer)+1,RLETAG);

		*(id0_unsigned_t id0_huge *)bigbuffer = compressed;

		if (!CA_FarWrite(file,(void id0_far *)bigbuffer,compressed+2) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}
	}

	for (o = player;o;o = o->next)
		if (!CA_FarWrite(file,(void id0_far *)o,sizeof(objtype)))
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

	MM_FreePtr (&bigbuffer);
	return(true);
}


id0_boolean_t
LoadGame(id0_int_t file)
{
	id0_word_t    i,j,size;
	objtype *o;
	id0_int_t orgx,orgy;
	objtype         *prev,*next,*followed;
	id0_unsigned_t        compressed,expanded;
	memptr  bigbuffer;

	if (!CA_FarRead(file,(void id0_far *)&gamestate,sizeof(gamestate)))
		return(false);

// drop down a cache level and mark everything, so when the option screen
// is exited it will be cached

	ca_levelbit >>= 1;
	ca_levelnum--;

	SetupGameLevel (false);         // load in and cache the base old level
	titleptr[ca_levelnum] = levelnames[mapon];

	ca_levelbit <<= 1;
	ca_levelnum ++;

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i++)   // Read all three planes of the map
	{
		if (!CA_FarRead(file,(void id0_far *)&compressed,sizeof(compressed)) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		if (!CA_FarRead(file,(void id0_far *)bigbuffer,compressed) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		CA_RLEWexpand ((id0_unsigned_t id0_huge *)bigbuffer,
			(id0_unsigned_t id0_huge *)mapsegs[i],compressed,RLETAG);
	}

	MM_FreePtr (&bigbuffer);

	// Read the object list back in - assumes at least one object in list

	InitObjArray ();
	new = player;
	prev = new->prev;
	next = new->next;
	if (!CA_FarRead(file,(void id0_far *)new,sizeof(objtype)))
		return(false);
	new->prev = prev;
	new->next = next;
	new->needtoreact = true;
	new->sprite = NULL;
	new = scoreobj;
	while (true)
	{
		prev = new->prev;
		next = new->next;
		if (!CA_FarRead(file,(void id0_far *)new,sizeof(objtype)))
			return(false);
		followed = new->next;
		new->prev = prev;
		new->next = next;
		new->needtoreact = true;
		new->sprite = NULL;

		if (followed)
			GetNewObj (false);
		else
			break;
	}

	*((id0_long_t *)&(scoreobj->temp1)) = -1;             // force score to be updated
	scoreobj->temp3 = -1;                   // and flower power
	scoreobj->temp4 = -1;                   // and lives

	return(true);
}

void
ResetGame(void)
{
	NewGame ();

	ca_levelnum--;
	CA_ClearMarks();
	titleptr[ca_levelnum] = NULL;           // don't reload old level
	ca_levelnum++;
}

#if FRILLS
void
TEDDeath(void)
{
	ShutdownId();
	execlp("TED5.EXE","TED5.EXE","/LAUNCH","KDREAMS",NULL);
}
#endif

static id0_boolean_t
MoveTitleTo(id0_int_t offset)
{
	id0_boolean_t         done;
	id0_int_t                     dir,
				chunk,
				move;
	id0_longword_t        lasttime,delay;

	if (offset < originxglobal)
		dir = -1;
	else
		dir = +1;

	chunk = dir * PIXGLOBAL;

	done = false;
	delay = 1;
	while (!done)
	{
		lasttime = SD_GetTimeCount();
		move = delay * chunk;
		if (chunk < 0)
			done = originxglobal + move <= offset;
		else
			done = originxglobal + move >= offset;
		if (!done)
		{
			RF_Scroll(move,0);
			RF_Refresh();
		}
		if (IN_IsUserInput())
			return(true);
		delay = SD_GetTimeCount() - lasttime;
	}
	if (originxglobal != offset)
	{
		RF_Scroll(offset - originxglobal,0);
		RF_Refresh();
	}
	return(false);
}

static id0_boolean_t
Wait(id0_longword_t time)
{
	time += SD_GetTimeCount();
	while ((SD_GetTimeCount() < time) && (!IN_IsUserInput()))
	{
		if (!(SD_GetTimeCount() % MINTICS))
			RF_Refresh();
	}
	return(IN_IsUserInput());
}

static id0_boolean_t
ShowText(id0_int_t offset,WindowRec *wr,id0_char_t *s)
{
	if (MoveTitleTo(offset))
		return(true);

	US_RestoreWindow(wr);
	US_CPrint(s);
	VW_UpdateScreen();

	if (Wait(TickBase * 5))
		return(true);

	US_RestoreWindow(wr);
	US_CPrint(s);
	VW_UpdateScreen();
	return(false);
}

/*
=====================
=
= DemoLoop
=
=====================
*/

void
DemoLoop (void)
{
	id0_char_t            *s;
	id0_word_t            move;
	id0_longword_t        lasttime;
	WindowRec       mywin;

#if FRILLS
//
// check for launch from ted
//
	if (tedlevel)
	{
		NewGame();
		gamestate.mapon = tedlevelnum;
		GameLoop();
		TEDDeath();
	}
#endif

//
// demo loop
//
	US_SetLoadSaveHooks(LoadGame,SaveGame,ResetGame);
	restartgame = gd_Continue;
	while (true)
	{
		// Load the Title map
		gamestate.mapon = 20;           // title map number
		loadedgame = false;
		SetupGameLevel(true);

		while (!restartgame && !loadedgame)
		{
			VW_InitDoubleBuffer();
			IN_ClearKeysDown();

			while (true)
			{
				// Display the Title map
				RF_NewPosition((5 * TILEGLOBAL) + (TILEGLOBAL / 2),
								(TILEGLOBAL * 2) + (TILEGLOBAL / 2)
								+ (TILEGLOBAL / 4));
				RF_ForceRefresh();
				RF_Refresh();
				RF_Refresh();

				if (Wait(TickBase * 2))
					break;

				mywin.x = (16 * 13) + 4;
				mywin.y = 0;
				mywin.w = 16 * 7;
				mywin.h = 200;
				mywin.px = mywin.x + 0;
				mywin.py = mywin.y + 10;
				s =             "Game\n"
						"John Carmack\n"
						"\n"
						"Utilities\n"
						"John Romero\n"
						"\n"
						"Interface/Sound\n"
						"Jason Blochowiak\n"
						"\n"
						"Creative Director\n"
						"Tom Hall\n"
						"\n"
						"Art\n"
						"Adrian Carmack\n";
				if (ShowText((9 * TILEGLOBAL) - (PIXGLOBAL * 2),&mywin,s))
					break;

				mywin.x = 4;
				mywin.y = 0;
				mywin.w = 16 * 7;
				mywin.h = 200;
				mywin.px = mywin.x + 0;
				mywin.py = mywin.y + 10;
				s =             "\n"
						"\"Keen Dreams\"\n"
						"Copyright 1991-93\n"
						"Softdisk, Inc.\n"
						"\n"
						"\n"
						"\n"
						"\n"
						"Commander Keen\n"
						"Copyright 1990-91\n"
						"Id Software, Inc.\n"
						"\n"
						"Press F1 for Help\n"
						"SPACE to Start\n";
				if (ShowText((2 * TILEGLOBAL) + (PIXGLOBAL * 2),&mywin,s))
					break;

				if (MoveTitleTo((5 * TILEGLOBAL) + (TILEGLOBAL / 2)))
					break;
				if (Wait(TickBase * 3))
					break;

				VWB_Bar(0,0,320,200,FIRSTCOLOR);
				US_DisplayHighScores(-1);

				if (IN_UserInput(TickBase * 8,false))
					break;
			}

			US_ControlPanel ();
		}

		if (!loadedgame)
			NewGame();
		GameLoop();
	}
}
