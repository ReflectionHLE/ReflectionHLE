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
//	ID_US.h - Header file for the User Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_US__
#define	__ID_US__

#ifdef	__DEBUG__
#define	__DEBUG_UserMgr__
#endif

//#define	HELPTEXTLINKED

#define	MaxX	320
#define	MaxY	200

#define	MaxHelpLines	500

#define	MaxHighName	57
#define	MaxScores	7
typedef	struct
		{
			id0_char_t	name[MaxHighName + 1];
			id0_long_t	score;
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			id0_word_t	completed;
#else
			id0_word_t	completed,episode;
#endif
		} HighScore;

#define	MaxGameName		32
#define	MaxSaveGames	6
typedef	struct
		{
			id0_char_t	signature[4];
			id0_word_t	*oldtest;
			id0_boolean_t	present;
			id0_char_t	name[MaxGameName + 1];
		} SaveGame;

#define	MaxString	128	// Maximum input string size

typedef	struct
		{
			id0_int_t	x,y,
				w,h,
				px,py;
		} WindowRec;	// Record used to save & restore screen windows

typedef	enum
		{
			gd_Continue,
			gd_Easy,
			gd_Normal,
			gd_Hard
		} GameDiff;

//	Hack import for TED launch support
extern	id0_boolean_t		tedlevel;
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
extern	id0_word_t			tedlevelnum;
#else
extern	id0_int_t			tedlevelnum;
#endif
extern	void		TEDDeath(void);

extern	id0_boolean_t		ingame,		// Set by game code if a game is in progress
					abortgame,	// Set if a game load failed
					loadedgame,	// Set if the current game was loaded
					NoWait,
					HighScoresDirty;
extern	id0_char_t		*abortprogram;	// Set to error msg if program is dying
extern	GameDiff	restartgame;	// Normally gd_Continue, else starts game
extern	id0_word_t		PrintX,PrintY;	// Current printing location in the window
extern	id0_word_t		WindowX,WindowY,// Current location of window
					WindowW,WindowH;// Current size of window

extern	id0_boolean_t		Button0,Button1,
					CursorBad;
extern	id0_int_t			CursorX,CursorY;

extern	void		(*USL_MeasureString)(const id0_char_t id0_far *,const id0_char_t id0_far *,id0_word_t *,id0_word_t *),
					(*USL_DrawString)(const id0_char_t id0_far *,const id0_char_t id0_far *);

extern	id0_boolean_t		(*USL_SaveGame)(id0_int_t),(*USL_LoadGame)(id0_int_t);
extern	void		(*USL_ResetGame)(void);
extern	SaveGame	Games[MaxSaveGames];
extern	HighScore	Scores[];

#define	US_HomeWindow()	{PrintX = WindowX; PrintY = WindowY;}

extern	void	US_Startup(void),
				US_Setup(void),
				US_Shutdown(void),
				US_InitRndT(id0_boolean_t randomize),
				US_SetLoadSaveHooks(id0_boolean_t (*load)(id0_int_t),
									id0_boolean_t (*save)(id0_int_t),
									void (*reset)(void)),
				US_TextScreen(void),
				US_UpdateTextScreen(void),
				US_FinishTextScreen(void),
				US_DrawWindow(id0_word_t x,id0_word_t y,id0_word_t w,id0_word_t h),
				US_CenterWindow(id0_word_t,id0_word_t),
				US_SaveWindow(WindowRec *win),
				US_RestoreWindow(WindowRec *win),
				US_ClearWindow(void),
				US_SetPrintRoutines(void (*measure)(const id0_char_t id0_far *,const id0_char_t id0_far *,id0_word_t *,id0_word_t *),
									void (*print)(const id0_char_t id0_far *,const id0_char_t id0_far *)),
				US_PrintCentered(id0_char_t GAMEVER_COND_FARPTR *s),
				US_CPrint(const id0_char_t GAMEVER_COND_FARPTR *s),
				US_CPrintLine(const id0_char_t GAMEVER_COND_FARPTR *s,const id0_char_t GAMEVER_COND_FARPTR *optse),
				US_Print(const id0_char_t GAMEVER_COND_FARPTR *s),
				US_PrintUnsigned(id0_longword_t n),
				US_PrintSigned(id0_long_t n),
				US_StartCursor(void),
				US_ShutCursor(void),
				US_CheckHighScore(id0_long_t score,id0_word_t other),
				US_DisplayHighScores(id0_int_t which);
extern	id0_boolean_t	US_UpdateCursor(void),
				US_LineInput(id0_int_t x,id0_int_t y,id0_char_t *buf,id0_char_t *def,id0_boolean_t escok,
								id0_int_t maxchars,id0_int_t maxwidth);
extern	id0_int_t		US_CheckParm(const id0_char_t *parm,const id0_char_t **strings),
				US_RndT(void);

		void	USL_PrintInCenter(id0_char_t GAMEVER_COND_FARPTR *s,Rect r);
		id0_char_t 	*USL_GiveSaveName(id0_word_t game);
#endif
