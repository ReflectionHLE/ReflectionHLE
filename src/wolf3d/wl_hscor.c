/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2021 NY00123
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

// *** ALPHA RESTORATION *** - #include this file
// from WL_MAIN.C or WL_INTER.C, depending on version

//==========================================================================

/*
==================
=
= DrawHighScores
=
==================
*/

void	DrawHighScores(void)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	id0_char_t		buffer[16],*str;
	id0_word_t		i,
				w,h;

	HighScore	*s;

	CA_CacheGrChunk (HIGHSCORESPIC);
	VWB_DrawPic(0,0,HIGHSCORESPIC);
	UNCACHEGRCHUNK (HIGHSCORESPIC);
#else
	id0_char_t		buffer[16],*str,buffer1[5];
	id0_byte_t		temp,temp1,temp2,temp3;
	id0_word_t		i,j,
				w,h,
				x,y;
	HighScore	*s;


	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	MM_SortMem ();
#endif

#ifndef SPEAR
	// *** SHAREWARE/REGISTERED APOGEE + EARLY GOODTIMES/ID RESTORATION ***
	// Uncomment line for Shareware/Registered 1.4 Apogee and early GT/ID,
	// and relocate it for pre-1.4 Apogee
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (GAMEVER_WOLFREV <= GV_WR_WL6GT14A)
	CA_CacheGrChunk (C_CODEPIC);
#endif
	CA_CacheGrChunk (HIGHSCORESPIC);
	CA_CacheGrChunk (STARTFONT);
	// *** SHAREWARE/REGISTERED APOGEE + EARLY GOODTIMES/ID RESTORATION ***
	// Relocated line for pre-1.4 Apogee
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	CA_CacheGrChunk (C_CODEPIC);
#endif
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	CA_CacheGrChunk (C_LEVELPIC);
	CA_CacheGrChunk (C_SCOREPIC);
	CA_CacheGrChunk (C_NAMEPIC);
#endif

	ClearMScreen();
	DrawStripes(10);

	VWB_DrawPic(48,0,HIGHSCORESPIC);
	// *** PRE-V1.4 APOGEE RESTORATION ***
	// Relocate line for pre-1.4 Apogee
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	UNCACHEGRCHUNK (HIGHSCORESPIC);
#endif

	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_DrawPic(4*8,68,C_NAMEPIC);
	VWB_DrawPic(21*8,68,C_LEVELPIC);
	VWB_DrawPic(32*8,68,C_SCOREPIC);
#elif (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	VWB_DrawPic(4*8,68,C_NAMEPIC);
	VWB_DrawPic(20*8,68,C_LEVELPIC);
	VWB_DrawPic(28*8,68,C_SCOREPIC);
#endif
	// *** SHAREWARE V1.0+REGISTERED APOGEE + EARLY GOODTIMES/ID RESTORATION ***
	// Uncomment line for Shareware 1.0 and any Registered Apogee release, and early Goodtimes/Id
//#if (GAMEVER_WOLFREV <= GV_WR_WL6GT14A) && ((!defined UPLOAD) || (GAMEVER_WOLFREV <= GV_WR_WL1AP10))
// REFKEEN: UPLOAD is now a variable, but we still need the first WOLFREV check
// to prevent a compilation error (C_CODEPIC isn't defined for S3DNA)
#if (GAMEVER_WOLFREV <= GV_WR_WL6GT14A)
if (!UPLOAD || (GAMEVER_WOLFREV <= GV_WR_WL1AP10))
//#ifndef UPLOAD
	VWB_DrawPic(35*8,68,C_CODEPIC);
#endif
	fontnumber=0;

#else
	CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
	ClearMScreen();
	DrawStripes(10);
	UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);

	CacheLump (HIGHSCORES_LUMP_START,HIGHSCORES_LUMP_END);
	CA_CacheGrChunk (STARTFONT+1);
	VWB_DrawPic (0,0,HIGHSCORESPIC);

	fontnumber = 1;
#endif


#ifndef SPEAR
	SETFONTCOLOR(15,0x29);
#else
	SETFONTCOLOR(HIGHLIGHT,0x29);
#endif

#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
	for (i = 0,s = Scores;i < MaxScores;i++,s++)
	{
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		PrintY = 68 + (16 * i);
#else
		PrintY = 76 + (16 * i);
#endif

		//
		// name
		//

		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PrintX = 60;
#elif (!defined SPEAR)
//#ifndef SPEAR
		PrintX = 4*8;
#else
		PrintX = 16;
#endif
		US_Print(s->name);

		//
		// level
		//

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (s->completed >= 30)
			s->completed = 30;
		sprintf(buffer,"%d-%d",MapEpisode[s->completed],MapLevel[s->completed]);
#else
		BE_Cross_ultoa_dec(s->completed,buffer);
#endif
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		USL_MeasureString(buffer,NULL,&w,&h);
		PrintX = (24 * 8)-w;
#elif (!defined SPEAR)
//#ifndef SPEAR
		for (str = buffer;*str;str++)
			*str = *str + (129 - '0');	// Used fixed-width numbers (129...)
		USL_MeasureString(buffer,NULL,&w,&h);
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PrintX = (24 * 8)-w;
#else
		PrintX = (22 * 8)-w;
#endif
#else
		USL_MeasureString(buffer,NULL,&w,&h);
		PrintX = 194 - w;
#endif

		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
//#ifndef UPLOAD
// REFKEEN: UPLOAD is now a variable, but we still need
// to check that the episode struct field is present
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
if (!UPLOAD)
{
#ifndef SPEAR
		PrintX -= 6;
		BE_Cross_itoa_dec(s->episode+1,buffer1);
		US_Print("E");
		US_Print(buffer1);
		US_Print("/L");
#endif
}
#endif
#endif

#ifdef SPEAR
		if (s->completed == 21)
			VWB_DrawPic (PrintX+8,PrintY-1,C_WONSPEARPIC);
		else
#endif
		US_Print(buffer);

		//
		// score
		//
		BE_Cross_ultoa_dec(s->score,buffer);
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		USL_MeasureString(buffer,NULL,&w,&h);
		PrintX = 288 - w;
#elif (!defined SPEAR)
//#ifndef SPEAR
		for (str = buffer;*str;str++)
			*str = *str + (129 - '0');	// Used fixed-width numbers (129...)
		USL_MeasureString(buffer,NULL,&w,&h);
		PrintX = (34 * 8) - 8 - w;
#else
		USL_MeasureString(buffer,NULL,&w,&h);
		PrintX = 292 - w;
#endif
		US_Print(buffer);

		// *** SHAREWARE V1.0+REGISTERED APOGEE + EARLY GOODTIMES/ID RESTORATION ***
		// *** REGISTERED V1.4 APOGEE + EARLY GOODTIMES/ID RESTORATION ***
		// Do compile verification block for Shareware 1.0 and any Registered Apogee release, and early Goodtimes/Id. But *not* for the alpha.
		#if (GAMEVER_WOLFREV <= GV_WR_WL6GT14A)
		//#if 0
//#if (!defined UPLOAD) || ((GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL1AP10))
// REFKEEN: UPLOAD is now a variable, but this shouldn't be built for < v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
if (!UPLOAD || (GAMEVER_WOLFREV <= GV_WR_WL1AP10))
{
//#ifndef UPLOAD
#ifndef SPEAR
		//
		// verification #
		//
		if (!i)
		{
		 temp=(((s->score >> 28)& 0xf)^
			  ((s->score >> 24)& 0xf))+'A';
		 temp1=(((s->score >> 20)& 0xf)^
			   ((s->score >> 16)& 0xf))+'A';
		 temp2=(((s->score >> 12)& 0xf)^
			   ((s->score >> 8)& 0xf))+'A';
		 temp3=(((s->score >> 4)& 0xf)^
			   ((s->score >> 0)& 0xf))+'A';

		 SETFONTCOLOR(0x49,0x29);
		 PrintX = 35*8;
		 buffer[0]=temp;
		 buffer[1]=temp1;
		 buffer[2]=temp2;
		 buffer[3]=temp3;
		 buffer[4]=0;
		 US_Print(buffer);
		 SETFONTCOLOR(15,0x29);
		}
#endif
//#endif
}
#endif
		#endif
	}

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	SETFONTCOLOR(0,15);
#else
	// *** PRE-V1.4 APOGEE RESTORATION (INC. SPECIAL CASE FOR V1.0) ***
	// Relocated line for pre-1.4 Apogee, but don't compile at all in v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	UNCACHEGRCHUNK (HIGHSCORESPIC);
#endif
	VW_UpdateScreen ();
#endif

#ifdef SPEAR
	UnCacheLump (HIGHSCORES_LUMP_START,HIGHSCORES_LUMP_END);
	fontnumber = 0;
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
}

//===========================================================================


/*
=======================
=
= CheckHighScore
=
=======================
*/

void	CheckHighScore (id0_long_t score,id0_word_t other)
{
	id0_word_t		i,j;
	id0_int_t			n;
	HighScore	myscore;

	strcpy(myscore.name,"");
	myscore.score = score;
	// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	myscore.episode = gamestate.mapon;
#elif (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	myscore.episode = gamestate.episode;
#endif
	myscore.completed = other;

	for (i = 0,n = -1;i < MaxScores;i++)
	{
		if
		(
			(myscore.score > Scores[i].score)
		||	(
				(myscore.score == Scores[i].score)
			&& 	(myscore.completed > Scores[i].completed)
			)
		)
		{
			for (j = MaxScores;--j > i;)
				Scores[j] = Scores[j - 1];
			Scores[i] = myscore;
			n = i;
			break;
		}
	}

#ifdef SPEAR
	StartCPMusic (XAWARD_MUS);
	// *** S3DNA + ALPHA RESTORATION ***
#elif (defined GAMEVER_NOAH3D)
	StartCPMusic (NOAH11_MUS);
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
//#else
	StartCPMusic (ROSTER_MUS);
#endif
	DrawHighScores ();

	VW_FadeIn ();

	if (n != -1)
	{
	//
	// got a high score
	//
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		PrintY = 68 + (16 * n);
#else
		PrintY = 76 + (16 * n);
#endif
#ifndef SPEAR
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		PrintX = 60;
#else
		PrintX = 4*8;
#endif
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
		backcolor = BORDCOLOR;
		fontcolor = 15;
#endif
		US_LineInput(PrintX,PrintY,Scores[n].name,id0_nil_t,true,MaxHighName,100);
#else
		PrintX = 16;
		fontnumber = 1;
		VWB_Bar (PrintX-2,PrintY-2,145,15,0x9c);
		VW_UpdateScreen ();
		backcolor = 0x9c;
		fontcolor = 15;
		US_LineInput(PrintX,PrintY,Scores[n].name,id0_nil_t,true,MaxHighName,130);
#endif
	}
	else
	{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		VW_UpdateScreen ();
#endif
		IN_ClearKeysDown ();
		IN_UserInput(500);
	}

}
