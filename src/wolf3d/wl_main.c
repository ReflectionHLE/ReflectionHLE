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

// WL_MAIN.C

//#include <conio.h>
#include "wl_def.h"
//#pragma hdrstop

REFKEEN_NS_B

/*
=============================================================================

						   WOLFENSTEIN 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


#define FOCALLENGTH     (0x5700l)               // in global coordinates
#define VIEWGLOBAL      0x10000                 // globals visable flush to wall

#define VIEWWIDTH       256                     // size of view window
#define VIEWHEIGHT      144

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

id0_char_t            str[80],str2[20];
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
id0_unsigned_t				tedlevelnum;
#else
id0_int_t				tedlevelnum;
#endif
id0_boolean_t         tedlevel;
id0_boolean_t         nospr;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_boolean_t		noquestions, nofloors;
id0_char_t		logon[] = "SUPER 3D NOAH'S ARK (v1.0)";
#endif
//id0_boolean_t         IsA386; // REFKEEN - Not needed
id0_int_t                     dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,
	5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
id0_int_t	questionnum = 0;
#endif

//
// proejection variables
//
fixed           focallength;
id0_unsigned_t        screenofs;
id0_int_t             viewwidth;
id0_int_t             viewheight;
id0_int_t             centerx;
id0_int_t             shootdelta;                     // pixels away from centerx a target can be
fixed           scale,maxslope;
id0_long_t            heightnumerator;
id0_int_t                     minheightdiv;


void            Quit (const id0_char_t *error);

		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
id0_boolean_t         startgame,loadedgame
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
		,virtualreality
#endif
		;
// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_int_t             mouseadjustment;

id0_char_t	configname[13]="CONFIG.";
#endif


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(SDMode)
BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(SMMode)
BE_CROSS_IMPLEMENT_FP_READWRITE_16LE_FUNCS(SDSMode)

/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig(void)
{
	BE_FILE_T                     file;
	SDMode          sd;
	SMMode          sm;
	SDSMode         sds;


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	if (BE_Cross_IsFileValid(file = BE_Cross_open_rewritable_for_reading("CONFIG."EXTENSION)))
//	if ( (file = open("CONFIG."EXTENSION,O_BINARY | O_RDONLY)) != -1)
#else
	if (BE_Cross_IsFileValid(file = BE_Cross_open_rewritable_for_reading(configname)))
//	if ( (file = open(configname,O_BINARY | O_RDONLY)) != -1)
#endif
	{
	//
	// valid config file
	//
		for (int i = 0; i < MaxScores; ++i)
		{
			BE_Cross_readInt8LEBuffer(file, Scores[i].name, sizeof(Scores[i].name));
			BE_Cross_readInt32LE(file, &Scores[i].score);
			BE_Cross_readInt16LE(file, &Scores[i].completed);
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) // REFKEEN: This depends on id_us.h definition
			BE_Cross_readInt16LE(file, &Scores[i].episode);
#endif
		}
		BE_Cross_read_SDMode_From16LE(file, &sd);
		BE_Cross_read_SMMode_From16LE(file, &sm);
		BE_Cross_read_SDSMode_From16LE(file, &sds);

		BE_Cross_read_boolean_From16LE(file, &mouseenabled);
		BE_Cross_read_boolean_From16LE(file, &joystickenabled);
		BE_Cross_read_boolean_From16LE(file, &joypadenabled);
		BE_Cross_read_boolean_From16LE(file, &joystickprogressive);
		BE_Cross_readInt16LE(file, &joystickport);

		BE_Cross_readInt16LEBuffer(file, dirscan, sizeof(dirscan));
		BE_Cross_readInt16LEBuffer(file, buttonscan, sizeof(buttonscan));
		BE_Cross_readInt16LEBuffer(file, buttonmouse, sizeof(buttonmouse));
		BE_Cross_readInt16LEBuffer(file, buttonjoy, sizeof(buttonjoy));

		BE_Cross_readInt16LE(file, &viewsize);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		BE_Cross_readInt16LE(file, &mouseadjustment);
#endif
#ifdef GAMEVER_NOAH3D
		BE_Cross_readInt16LE(file, &questionnum);
#endif
#if 0
		read(file,Scores,sizeof(HighScore) * MaxScores);

		read(file,&sd,sizeof(sd));
		read(file,&sm,sizeof(sm));
		read(file,&sds,sizeof(sds));

		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joypadenabled,sizeof(joypadenabled));
		read(file,&joystickprogressive,sizeof(joystickprogressive));
		read(file,&joystickport,sizeof(joystickport));

		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));

		read(file,&viewsize,sizeof(viewsize));
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		read(file,&mouseadjustment,sizeof(mouseadjustment));
#endif
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		read(file,&questionnum,sizeof(questionnum));
#endif
#endif

		BE_Cross_close(file);

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
		if (sd == sdm_AdLib && (!AdLibPresent || !SoundBlasterPresent))
#else
		if (sd == sdm_AdLib && !AdLibPresent && !SoundBlasterPresent)
#endif
		{
			// REFKEEN: Vanilla bug
//			sd = sdm_PC;
			sd = sdm_Off/*smm_Off*/;
		}

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (sds ==
		    sds_SoundBlaster && !SoundBlasterPresent)
#else
		if ((sds == sds_SoundBlaster && !SoundBlasterPresent) ||
			(sds == sds_SoundSource && !SoundSourcePresent))
#endif
			sds = sds_Off;
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

		if (!MousePresent)
			mouseenabled = false;
		if (!JoysPresent[joystickport])
			joystickenabled = false;

		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		MainMenu[6].active=1;
		MainItems.curpos=0;
#endif
	}
	else
	{
	//
	// no config file, so select by hardware
	//
		if (SoundBlasterPresent || AdLibPresent)
		{
			sd = sdm_AdLib;
			sm = smm_AdLib;
		}
		else
		{
			sd = sdm_PC;
			sm = smm_Off;
		}

		if (SoundBlasterPresent)
			sds = sds_SoundBlaster;
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
		else if (SoundSourcePresent)
			sds = sds_SoundSource;
#endif
		else
			sds = sds_Off;

		if (MousePresent)
			mouseenabled = true;

		joystickenabled = false;
		joypadenabled = false;
		joystickport = 0;
		joystickprogressive = false;

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		viewsize = 18;
#else
		viewsize = 15;
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		mouseadjustment=5;
#endif
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		questionnum=0;
#endif
	}

	SD_SetMusicMode (sm);
	SD_SetSoundMode (sd);
	SD_SetDigiDevice (sds);

}


/*
====================
=
= WriteConfig
=
====================
*/

void WriteConfig(void)
{
	BE_FILE_T                     file;

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	file = BE_Cross_open_rewritable_for_overwriting("CONFIG."EXTENSION);
//	file = open("CONFIG."EXTENSION,O_CREAT | O_BINARY | O_WRONLY,
//				S_IREAD | S_IWRITE | S_IFREG);
#else
	file = BE_Cross_open_rewritable_for_overwriting(configname);
//	file = open(configname,O_CREAT | O_BINARY | O_WRONLY,
//				S_IREAD | S_IWRITE | S_IFREG);
#endif

	if (BE_Cross_IsFileValid(file))
	{
		for (int i = 0; i < MaxScores; ++i)
		{
			BE_Cross_writeInt8LEBuffer(file, Scores[i].name, sizeof(Scores[i].name));
			BE_Cross_writeInt32LE(file, &Scores[i].score);
			BE_Cross_writeInt16LE(file, &Scores[i].completed);
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10) // REFKEEN: This depends on id_us.h definition
			BE_Cross_writeInt16LE(file, &Scores[i].episode);
#endif
		}
		BE_Cross_write_SDMode_To16LE(file, &SoundMode);
		BE_Cross_write_SMMode_To16LE(file, &MusicMode);
		BE_Cross_write_SDSMode_To16LE(file, &DigiMode);

		BE_Cross_write_boolean_To16LE(file, &mouseenabled);
		BE_Cross_write_boolean_To16LE(file, &joystickenabled);
		BE_Cross_write_boolean_To16LE(file, &joypadenabled);
		BE_Cross_write_boolean_To16LE(file, &joystickprogressive);
		BE_Cross_writeInt16LE(file, &joystickport);

		BE_Cross_writeInt16LEBuffer(file, dirscan, sizeof(dirscan));
		BE_Cross_writeInt16LEBuffer(file, buttonscan, sizeof(buttonscan));
		BE_Cross_writeInt16LEBuffer(file, buttonmouse, sizeof(buttonmouse));
		BE_Cross_writeInt16LEBuffer(file, buttonjoy, sizeof(buttonjoy));

		BE_Cross_writeInt16LE(file, &viewsize);
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		BE_Cross_writeInt16LE(file, &mouseadjustment);
#endif
#ifdef GAMEVER_NOAH3D
		BE_Cross_writeInt16LE(file, &questionnum);
#endif
#if 0
		write(file,Scores,sizeof(HighScore) * MaxScores);

		write(file,&SoundMode,sizeof(SoundMode));
		write(file,&MusicMode,sizeof(MusicMode));
		write(file,&DigiMode,sizeof(DigiMode));

		write(file,&mouseenabled,sizeof(mouseenabled));
		write(file,&joystickenabled,sizeof(joystickenabled));
		write(file,&joypadenabled,sizeof(joypadenabled));
		write(file,&joystickprogressive,sizeof(joystickprogressive));
		write(file,&joystickport,sizeof(joystickport));

		write(file,&dirscan,sizeof(dirscan));
		write(file,&buttonscan,sizeof(buttonscan));
		write(file,&buttonmouse,sizeof(buttonmouse));
		write(file,&buttonjoy,sizeof(buttonjoy));

		write(file,&viewsize,sizeof(viewsize));
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		write(file,&mouseadjustment,sizeof(mouseadjustment));
#endif
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		write(file,&questionnum,sizeof(questionnum));
#endif
#endif

		BE_Cross_close(file);
	}
}


//===========================================================================


#if 0 // REFKEEN - Not needed
/*
========================
=
= Patch386
=
= Patch ldiv to use 32 bit instructions
=
========================
*/

id0_char_t    *JHParmStrings[] = {"no386",id0_nil_t};
void Patch386 (void)
{
extern void id0_far jabhack2(void);
extern id0_int_t id0_far  CheckIs386(void);

	id0_int_t     i;

	for (i = 1;i < id0_argc;i++)
		if (US_CheckParm(id0_argv[i],JHParmStrings) == 0)
		{
			IsA386 = false;
			return;
		}

	if (CheckIs386())
	{
		IsA386 = true;
		jabhack2();
	}
	else
		IsA386 = false;
}
#endif // REFKEEN - Disable

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

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void NewGame (id0_int_t difficulty)
#else
void NewGame (id0_int_t difficulty,id0_int_t episode)
#endif
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	memset (&LevelRatios,0,sizeof(LRstruct)*30);
#endif
	memset (&gamestate,0,sizeof(gamestate));
	gamestate.difficulty = difficulty;
	gamestate.weapon = gamestate.bestweapon
		= gamestate.chosenweapon = wp_pistol;
	gamestate.health = 100;
	gamestate.ammo = STARTAMMO;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;
	// *** S3DNA + ALPHA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	gamestate.mapon=episode;
	gamestate.maxammo=99;
	gamestate.gas = gamestate.missiles = 0;
	gamestate.machinegun = 0;
	gamestate.chaingun = 0;
	gamestate.flamethrower = 0;
	gamestate.missile = 0;
	gamestate.automap = false;
#elif (GAMEVER_WOLFREV > GV_WR_WL920312)
	gamestate.episode=episode;
#endif

	startgame = true;
}

//===========================================================================

void DiskFlopAnim(id0_int_t x,id0_int_t y)
{
 static id0_char_t which=0;
 // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
 if (!x && !y)
   return;
#endif
 VWB_DrawPic(x,y,C_DISKLOADING1PIC+which);
 VW_UpdateScreen();
 // *** S3DNA RESTORATION ***
 // WARNING: The following statement may technically lead to undefined behaviors
#ifdef GAMEVER_NOAH3D
 which=which+1; which=which&3;
#else
 which^=1;
#endif
}


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
// This isn't found in the v1.0 EXE
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
id0_long_t DoChecksum(id0_byte_t id0_far *source,id0_unsigned_t size,id0_long_t checksum)
{
 id0_unsigned_t i;

 for (i=0;i<size-1;i++)
   checksum += source[i]^source[i+1];

 return checksum;
}
#endif


/*
==================
=
= SaveTheGame
=
==================
*/

id0_boolean_t SaveTheGame(BE_FILE_T file,id0_int_t x,id0_int_t y)
{
	return false; // TODO IMPLEMENT
#if 0
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// Comment out anything to do with checksumming and free size verifications, plus a bit more

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	// REFKEEN (TODO) Let's disable the available disk space check for now
	id0_long_t checksum;
//	struct diskfree_t dfree;
//	id0_long_t avail,size,checksum;
#endif
	objtype *ob,nullobj;


#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#if 0 // REFKEEN: Disable for now
	if (_dos_getdiskfree(0,&dfree))
	  Quit("Error in _dos_getdiskfree call");

	avail = (id0_long_t)dfree.avail_clusters *
			dfree.bytes_per_sector *
			dfree.sectors_per_cluster;

	size = 0;
	for (ob = player; ob ; ob=ob->next)
	  size += sizeof(*ob);
	size += sizeof(nullobj);

	size += sizeof(gamestate) +
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			sizeof(LRstruct)*30 +
#else
			sizeof(LRstruct)*8 +
#endif
			sizeof(tilemap) +
			sizeof(actorat) +
			sizeof(laststatobj) +
			sizeof(statobjlist) +
			sizeof(doorposition) +
			sizeof(pwallstate) +
			sizeof(pwallx) +
			sizeof(pwally) +
			sizeof(pwalldir) +
			sizeof(pwallpos);

	if (avail < size)
	{
	 Message(STR_NOSPACE1"\n"
			 STR_NOSPACE2);
	 return false;
	}
#endif

	checksum = 0;
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10


	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)&gamestate,sizeof(gamestate));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&gamestate,sizeof(gamestate),checksum);
#endif

	DiskFlopAnim(x,y);
	// *** SHAREWARE V1.0 APOGEE + SOD (DEMO) V1.0+V1.4 FORMGEN + S3DNA RESTORATION ***
	// LevelRatios should have 8 entries in these versions of SOD (like WL1/WL6) but don't write anything in Wolf3D v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#if (defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A)
//#ifdef SPEAR
	CA_FarWrite (file,(void id0_far *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((id0_byte_t id0_far *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#elif (defined GAMEVER_NOAH3D)
	CA_FarWrite (file,(void id0_far *)&LevelRatios[0],sizeof(LRstruct)*30);
	checksum = DoChecksum((id0_byte_t id0_far *)&LevelRatios[0],sizeof(LRstruct)*30,checksum);
#else
	CA_FarWrite (file,(void id0_far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((id0_byte_t id0_far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
#endif
	CA_FarWrite (file,(void id0_far *)tilemap,sizeof(tilemap));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)tilemap,sizeof(tilemap),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)actorat,sizeof(actorat));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)actorat,sizeof(actorat),checksum);
#endif

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	CA_FarWrite (file,(void id0_far *)areaconnect,sizeof(areaconnect));
	CA_FarWrite (file,(void id0_far *)areabyplayer,sizeof(areabyplayer));
#endif

	for (ob = player ; ob ; ob=ob->next)
	{
	 DiskFlopAnim(x,y);
	 CA_FarWrite (file,(void id0_far *)ob,sizeof(*ob));
	}
	nullobj.active = ac_badobject;          // end of file marker
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)&nullobj,sizeof(nullobj));



	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)&laststatobj,sizeof(laststatobj));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&laststatobj,sizeof(laststatobj),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)statobjlist,sizeof(statobjlist));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)statobjlist,sizeof(statobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)doorposition,sizeof(doorposition));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)doorposition,sizeof(doorposition),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void id0_far *)doorobjlist,sizeof(doorobjlist));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)doorobjlist,sizeof(doorobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	CA_FarWrite (file,(void id0_far *)&pwallstate,sizeof(pwallstate));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwallstate,sizeof(pwallstate),checksum);
#endif
	CA_FarWrite (file,(void id0_far *)&pwallx,sizeof(pwallx));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwallx,sizeof(pwallx),checksum);
#endif
	CA_FarWrite (file,(void id0_far *)&pwally,sizeof(pwally));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwally,sizeof(pwally),checksum);
#endif
	CA_FarWrite (file,(void id0_far *)&pwalldir,sizeof(pwalldir));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwalldir,sizeof(pwalldir),checksum);
#endif
	CA_FarWrite (file,(void id0_far *)&pwallpos,sizeof(pwallpos));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwallpos,sizeof(pwallpos),checksum);
#endif

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	//
	// WRITE OUT CHECKSUM
	//
	CA_FarWrite (file,(void id0_far *)&checksum,sizeof(checksum));
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312

	return(true);
#endif
}

//===========================================================================

/*
==================
=
= LoadTheGame
=
==================
*/

id0_boolean_t LoadTheGame(BE_FILE_T file,id0_int_t x,id0_int_t y)
{
	return false; // TODO IMPLEMENT
#if 0
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// Comment out anything to do with checksumming, plus a bit more

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	id0_long_t checksum,oldchecksum;
#endif
	objtype *ob,nullobj;


#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = 0;
#endif

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)&gamestate,sizeof(gamestate));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&gamestate,sizeof(gamestate),checksum);
#endif

	DiskFlopAnim(x,y);
	// *** SHAREWARE V1.0 APOGEE + SOD (DEMO) V1.0+V1.4 FORMGEN + S3DNA RESTORATION ***
	// LevelRatios should have 8 entries in these versions of SOD (like WL1/WL6) but don't read anything in Wolf3D v1.0
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
#if (defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A)
//#ifdef SPEAR
	CA_FarRead (file,(void id0_far *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((id0_byte_t id0_far *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#elif (defined GAMEVER_NOAH3D)
	CA_FarRead (file,(void id0_far *)&LevelRatios[0],sizeof(LRstruct)*30);
	checksum = DoChecksum((id0_byte_t id0_far *)&LevelRatios[0],sizeof(LRstruct)*30,checksum);
#else
	CA_FarRead (file,(void id0_far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((id0_byte_t id0_far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
#endif
	SetupGameLevel ();

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)tilemap,sizeof(tilemap));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)tilemap,sizeof(tilemap),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)actorat,sizeof(actorat));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)actorat,sizeof(actorat),checksum);
#endif

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	CA_FarRead (file,(void id0_far *)areaconnect,sizeof(areaconnect));
	CA_FarRead (file,(void id0_far *)areabyplayer,sizeof(areabyplayer));
#endif



	InitActorList ();
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)player,sizeof(*player));

	while (1)
	{
	 DiskFlopAnim(x,y);
		CA_FarRead (file,(void id0_far *)&nullobj,sizeof(nullobj));
		if (nullobj.active == ac_badobject)
			break;
		GetNewActor ();
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP11)
		memcpy (newobj,&nullobj,sizeof(nullobj));
#else
	 // don't copy over the links
		memcpy (newobj,&nullobj,sizeof(nullobj)-4);
#endif
	}



	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)&laststatobj,sizeof(laststatobj));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&laststatobj,sizeof(laststatobj),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)statobjlist,sizeof(statobjlist));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)statobjlist,sizeof(statobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)doorposition,sizeof(doorposition));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)doorposition,sizeof(doorposition),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void id0_far *)doorobjlist,sizeof(doorobjlist));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)doorobjlist,sizeof(doorobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	CA_FarRead (file,(void id0_far *)&pwallstate,sizeof(pwallstate));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwallstate,sizeof(pwallstate),checksum);
#endif
	CA_FarRead (file,(void id0_far *)&pwallx,sizeof(pwallx));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwallx,sizeof(pwallx),checksum);
#endif
	CA_FarRead (file,(void id0_far *)&pwally,sizeof(pwally));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwally,sizeof(pwally),checksum);
#endif
	CA_FarRead (file,(void id0_far *)&pwalldir,sizeof(pwalldir));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwalldir,sizeof(pwalldir),checksum);
#endif
	CA_FarRead (file,(void id0_far *)&pwallpos,sizeof(pwallpos));
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	checksum = DoChecksum((id0_byte_t id0_far *)&pwallpos,sizeof(pwallpos),checksum);
#endif

#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	CA_FarRead (file,(void id0_far *)&oldchecksum,sizeof(oldchecksum));

	if (oldchecksum != checksum)
	{
	 Message(STR_SAVECHT1"\n"
			 STR_SAVECHT2"\n"
			 STR_SAVECHT3"\n"
			 STR_SAVECHT4);

	 IN_ClearKeysDown();
	 IN_Ack();

	 gamestate.score = 0;
	 gamestate.lives = 1;
	 gamestate.weapon =
	   gamestate.chosenweapon =
	   gamestate.bestweapon = wp_pistol;
	 gamestate.ammo = 8;
	 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	 gamestate.gas = gamestate.missiles = 0;
	 gamestate.machinegun = 0;
	 gamestate.chaingun = 0;
	 gamestate.flamethrower = 0;
	 gamestate.missile = 0;
	 gamestate.automap = false;
#endif
	}
#endif
#endif

	return true;
#endif
}

//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
	US_Shutdown ();
	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	VW_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ();
}


//===========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const   float   radtoint = (float)FINEANGLES/2/PI;

void BuildTables (void)
{
  id0_int_t           i;
#if !REFKEEN_USE_PRECALC_SINTABLE
  float         angle,anglestep;
#endif
  double        tang;
#if !REFKEEN_USE_PRECALC_SINTABLE
  fixed         value;
#endif


//
// calculate fine tangents
//

	for (i=0;i<FINEANGLES/8;i++)
	{
		tang = tan( (i+0.5)/radtoint);
		finetangent[i] = tang*TILEGLOBAL;
		finetangent[FINEANGLES/4-1-i] = 1/tang*TILEGLOBAL;
	}

#if !REFKEEN_USE_PRECALC_SINTABLE
//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//
// The low word of the value is the fraction, the high bit is the sign bit,
// bits 16-30 should be 0
//

  angle = 0;
  anglestep = PI/2/ANGLEQUAD;
  for (i=0;i<=ANGLEQUAD;i++)
  {
	value=GLOBAL1*sin(angle);
	sintable[i]=
	  sintable[i+ANGLES]=
	  sintable[ANGLES/2-i] = value;
	sintable[ANGLES-i]=
	  sintable[ANGLES/2+i] = value | 0x80000000l;
	angle += anglestep;
  }
#endif // !REFKEEN_USE_PRECALC_SINTABLE

}

//===========================================================================


/*
====================
=
= CalcProjection
=
= Uses focallength
=
====================
*/

void CalcProjection (id0_long_t focal)
{
	id0_int_t             i;
	id0_long_t            intang;
	float   angle;
	double  tang;
	double  planedist;
	double  globinhalf;
	id0_int_t             halfview;
	double  halfangle,facedist;


	focallength = focal;
	facedist = focal+MINDIST;
	halfview = viewwidth/2;                                 // half view in pixels

//
// calculate scale value for vertical height calculations
// and sprite x calculations
//
	scale = halfview*facedist/(VIEWGLOBAL/2);

//
// divide heightnumerator by a posts distance to get the posts height for
// the heightbuffer.  The pixel height is height>>2
//
	heightnumerator = (TILEGLOBAL*scale)>>6;
	minheightdiv = heightnumerator/0x7fff +1;

//
// calculate the angle offset from view angle of each pixel's ray
//

	for (i=0;i<halfview;i++)
	{
	// start 1/2 pixel over, so viewangle bisects two middle pixels
		tang = (id0_long_t)i*VIEWGLOBAL/viewwidth/facedist;
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[halfview-1-i] = intang;
		pixelangle[halfview+i] = -intang;
	}

//
// if a point's abs(y/x) is greater than maxslope, the point is outside
// the view area
//
	maxslope = finetangent[pixelangle[0]];
	maxslope >>= 8;
}



//===========================================================================

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls (void)
{
	id0_int_t     i;

	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}

//===========================================================================

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
/*
==========================
=
= SignonScreen
=
==========================
*/

void SignonScreen (void)                        // VGA version
{
	id0_unsigned_t        segstart,seglength;

	VL_SetVGAPlaneMode ();
	VL_TestPaletteSet ();
	VL_SetPalette (gamepal);

	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (!virtualreality)
#endif
	{
		VW_SetScreen(0x8000,0);
		VL_MungePic ((id0_byte_t *)introscn,320,200);
		VL_MemToScreen ((id0_byte_t *)introscn,320,200,0,0);
		VW_SetScreen(0,0);
	}

//
// reclaim the memory from the linked in signon screen
//
	// REFKEEN DIFFERENCE (FIXME: Should we "fix" this at all?)
	// - Don't handle this, a bit more complicated with our setup and the
	// difference is (probably) insignificant with well-defined behaviors
	// anyway...
#if 0
	segstart = FP_SEG(&introscn);
	seglength = 64000/16;
	if (FP_OFF(&introscn))
	{
		segstart++;
		seglength--;
	}
	MML_UseSpace (segstart,seglength);
#endif
}


/*
==========================
=
= FinishSignon
=
==========================
*/

void FinishSignon (void)
{

#ifndef SPEAR
	// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	VW_Bar (0,185,320,15,BE_ST_VGAFetchGFXByteFromPlane(0,0));
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	VW_Bar (0,185,300,15,BE_ST_VGAFetchGFXByteFromPlane(0,0));
#else
	VW_Bar (0,189,300,11,BE_ST_VGAFetchGFXByteFromPlane(0,0));
#endif
	WindowX = 0;
	WindowW = 320;
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	PrintY = 188;
#else
	PrintY = 190;
#endif

	#ifndef JAPAN
	SETFONTCOLOR(14,4);

	#ifdef SPANISH
	US_CPrint ("Oprima una tecla");
	#else
	US_CPrint ("Press a key");
	#endif

	#endif

	if (!NoWait)
		IN_Ack ();

	#ifndef JAPAN
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	VW_Bar (0,185,320,15,BE_ST_VGAFetchGFXByteFromPlane(0,0));

	PrintY = 188;
#else
	VW_Bar (0,189,300,11,BE_ST_VGAFetchGFXByteFromPlane(0,0));

	PrintY = 190;
#endif
	SETFONTCOLOR(10,4);

	#ifdef SPANISH
	US_CPrint ("pensando...");
	#else
	US_CPrint ("Working...");
	#endif

	#endif

	SETFONTCOLOR(0,15);
#else
	if (!NoWait)
		VW_WaitVBL(3*70);
#endif
}
#endif // S3DNA RESTORATION

//===========================================================================

/*
=================
=
= MS_CheckParm
=
=================
*/

id0_boolean_t MS_CheckParm (const id0_char_t id0_far *check)
{
	id0_int_t             i;
	const id0_char_t    *parm;

	for (i = 1;i<id0_argc;i++)
	{
		parm = id0_argv[i];

		while ( !isalpha(*parm) )       // skip - / \ etc.. in front of parm
			if (!*parm++)
				break;                          // hit end of string without an alphanum

		if (!BE_Cross_strcasecmp(check,parm))
//		if ( !_fstricmp(check,parm) )
			return true;
	}

	return false;
}

//===========================================================================

/*
=====================
=
= InitDigiMap
=
=====================
*/

static  id0_int_t     wolfdigimap[] =
		{
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			D_ANTLPESND,            0,
			D_BEARSND,              1,
			D_BONUSSND,             2,
			D_CAMELSND,             3,
			D_CLOSDRSND,            4,
			D_COCHITSND,            5,
			D_COCTHRSND,            6,
			D_DBITESND,             7,
			D_EATSND,               8,
			D_EDIESND,              9,
			D_ELPHNTSND,            10,
			D_EXTRASND,             11,
			D_GATLINSND,            12,
			D_GIRAFESND,            13,
			D_GOATSND,              14,
			D_GUNSHTSND,            15,
			D_INCSND,               16,
			D_KANGROSND,            17,
			D_KEYSND,               18,
			D_KNIFESND,             19,
			D_MONKEYSND,            20,
			D_MSTEPSND,             21,
			D_OPENDRSND,            22,
			D_OSTRCHSND,            23,
			D_OXSND,                24,
			D_PAINSND,              25,
			D_PDIESND,              26,

			D_PWALLSND,             27,
			D_SHEEPSND,             28,
			D_SLING2SND,            29,
			D_SPITSND,              30,
			D_WATHITSND,            31,
			D_WATTHRSND,            32,
			/// *** ALPHA RESTORATION ***
#elif (GAMEVER_WOLFREV <= GV_WR_WL920312)
			HALTSND,                0,
			DEATHSCREAM1SND,        1,
			DEATHSCREAM2SND,        2,
			DEATHSCREAM3SND,        2,
			DOGBARKSND,             3,
			CLOSEDOORSND,           4,
			OPENDOORSND,            5,
			HITENEMYSND,            6,
			TAKEDAMAGESND,          7,
			ATKGATLINGSND,          8,
			ATKMACHINEGUNSND,       8,
			ATKPISTOLSND,           9,
#elif (!defined SPEAR) // S3DNA + ALPHA
			// These first sounds are in the upload version
//#ifndef SPEAR
			HALTSND,                0,
			DOGBARKSND,             1,
			CLOSEDOORSND,           2,
			OPENDOORSND,            3,
			ATKMACHINEGUNSND,       4,
			ATKPISTOLSND,           5,
			ATKGATLINGSND,          6,
			SCHUTZADSND,            7,
			GUTENTAGSND,            8,
			MUTTISND,               9,
			BOSSFIRESND,            10,
			SSFIRESND,              11,
			DEATHSCREAM1SND,        12,
			DEATHSCREAM2SND,        13,
			DEATHSCREAM3SND,        13,
			TAKEDAMAGESND,          14,
			PUSHWALLSND,            15,

			LEBENSND,               20,
			NAZIFIRESND,            21,
			SLURPIESND,             22,

			YEAHSND,				32,

#ifndef UPLOAD
			// These are in all other episodes
			DOGDEATHSND,            16,
			AHHHGSND,               17,
			DIESND,                 18,
			EVASND,                 19,

			TOT_HUNDSND,            23,
			MEINGOTTSND,            24,
			SCHABBSHASND,           25,
			HITLERHASND,            26,
			SPIONSND,               27,
			NEINSOVASSND,           28,
			DOGATTACKSND,           29,
			LEVELDONESND,           30,
			MECHSTEPSND,			31,

			SCHEISTSND,				33,
			DEATHSCREAM4SND,		34,		// AIIEEE
			DEATHSCREAM5SND,		35,		// DEE-DEE
			DONNERSND,				36,		// EPISODE 4 BOSS DIE
			EINESND,				37,		// EPISODE 4 BOSS SIGHTING
			ERLAUBENSND,			38,		// EPISODE 6 BOSS SIGHTING
			DEATHSCREAM6SND,		39,		// FART
			DEATHSCREAM7SND,		40,		// GASP
			DEATHSCREAM8SND,		41,		// GUH-BOY!
			DEATHSCREAM9SND,		42,		// AH GEEZ!
			KEINSND,				43,		// EPISODE 5 BOSS SIGHTING
			MEINSND,				44,		// EPISODE 6 BOSS DIE
			ROSESND,				45,		// EPISODE 5 BOSS DIE

#endif
#else
//
// SPEAR OF DESTINY DIGISOUNDS
//
			HALTSND,                0,
			CLOSEDOORSND,           2,
			OPENDOORSND,            3,
			ATKMACHINEGUNSND,       4,
			ATKPISTOLSND,           5,
			ATKGATLINGSND,          6,
			SCHUTZADSND,            7,
			BOSSFIRESND,            8,
			SSFIRESND,              9,
			DEATHSCREAM1SND,        10,
			DEATHSCREAM2SND,        11,
			TAKEDAMAGESND,          12,
			PUSHWALLSND,            13,
			AHHHGSND,               15,
			LEBENSND,               16,
			NAZIFIRESND,            17,
			SLURPIESND,             18,
			LEVELDONESND,           22,
			DEATHSCREAM4SND,		23,		// AIIEEE
			DEATHSCREAM3SND,        23,		// DOUBLY-MAPPED!!!
			DEATHSCREAM5SND,		24,		// DEE-DEE
			DEATHSCREAM6SND,		25,		// FART
			DEATHSCREAM7SND,		26,		// GASP
			DEATHSCREAM8SND,		27,		// GUH-BOY!
			DEATHSCREAM9SND,		28,		// AH GEEZ!
			GETGATLINGSND,			38,		// Got Gat replacement

#ifndef SPEARDEMO
			DOGBARKSND,             1,
			DOGDEATHSND,            14,
			SPIONSND,               19,
			NEINSOVASSND,           20,
			DOGATTACKSND,           21,
			TRANSSIGHTSND,			29,		// Trans Sight
			TRANSDEATHSND,			30,		// Trans Death
			WILHELMSIGHTSND,		31,		// Wilhelm Sight
			WILHELMDEATHSND,		32,		// Wilhelm Death
			UBERDEATHSND,			33,		// Uber Death
			KNIGHTSIGHTSND,			34,		// Death Knight Sight
			KNIGHTDEATHSND,			35,		// Death Knight Death
			ANGELSIGHTSND,			36,		// Angel Sight
			ANGELDEATHSND,			37,		// Angel Death
			GETSPEARSND,			39,		// Got Spear replacement
#endif
#endif
			LASTSOUND
		};


void InitDigiMap (void)
{
	id0_int_t                     *map;

	for (map = wolfdigimap;*map != LASTSOUND;map += 2)
		DigiMap[map[0]] = map[1];


}


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
CP_iteminfo	MusicItems={CTL_X,40,11,0,32};
CP_itemtype id0_far MusicMenu[]=
	{
		{1,"Song 1",0},
		{1,"It's All Good!",0},
		{1,"Song 3",0},
		{1,"Song 4",0},
		{1,"Feed-time Shuffle",0},
		{1,"Song 6",0},
		{1,"Song 7",0},
		{1,"Song 8",0},
		{1,"Song 9",0},
		{1,"The Happy Song",0},
		{1,"Song 11",0},
	};
#elif (!defined SPEAR)
//#ifndef SPEAR
CP_iteminfo	MusicItems={CTL_X,CTL_Y,6,0,32};
CP_itemtype id0_far MusicMenu[]=
	{
		{1,"Get Them!",0},
		{1,"Searching",0},
		{1,"P.O.W.",0},
		{1,"Suspense",0},
		{1,"War March",0},
		{1,"Around The Corner!",0},

		{1,"Nazi Anthem",0},
		{1,"Lurking...",0},
		{1,"Going After Hitler",0},
		{1,"Pounding Headache",0},
		{1,"Into the Dungeons",0},
		{1,"Ultimate Conquest",0},

		{1,"Kill the S.O.B.",0},
		{1,"The Nazi Rap",0},
		{1,"Twelfth Hour",0},
		{1,"Zero Hour",0},
		{1,"Ultimate Conquest",0},
		{1,"Wolfpack",0}
	};
#else
CP_iteminfo MusicItems={CTL_X,CTL_Y-20,9,0,32};
CP_itemtype id0_far MusicMenu[]=
   {
		{1,"Funky Colonel Bill",0},
		{1,"Death To The Nazis",0},
		{1,"Tiptoeing Around",0},
		{1,"Is This THE END?",0},
		{1,"Evil Incarnate",0},
		{1,"Jazzin' Them Nazis",0},
		{1,"Puttin' It To The Enemy",0},
		{1,"The SS Gonna Get You",0},
		{1,"Towering Above",0}
	};
#endif

#ifndef SPEARDEMO
void DoJukebox(void)
{
	id0_int_t which,lastsong=-1;
	id0_unsigned_t start;
//	id0_unsigned_t start,songs[]=
	musicnames songs[]=
		{
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			NOAH01_MUS,
			NOAH02_MUS,
			NOAH03_MUS,
			NOAH04_MUS,
			NOAH05_MUS,
			NOAH06_MUS,
			NOAH07_MUS,
			NOAH08_MUS,
			NOAH09_MUS,
			NOAH10_MUS,
			NOAH11_MUS,
#elif (!defined SPEAR)
//#ifndef SPEAR
			GETTHEM_MUS,
			SEARCHN_MUS,
			POW_MUS,
			SUSPENSE_MUS,
			WARMARCH_MUS,
			CORNER_MUS,

			NAZI_OMI_MUS,
			PREGNANT_MUS,
			GOINGAFT_MUS,
			HEADACHE_MUS,
			DUNGEON_MUS,
			ULTIMATE_MUS,

			INTROCW3_MUS,
			NAZI_RAP_MUS,
			TWELFTH_MUS,
			ZEROHOUR_MUS,
			ULTIMATE_MUS,
			PACMAN_MUS
#else
			XFUNKIE_MUS,             // 0
			XDEATH_MUS,              // 2
			XTIPTOE_MUS,             // 4
			XTHEEND_MUS,             // 7
			XEVIL_MUS,               // 17
			XJAZNAZI_MUS,            // 18
			XPUTIT_MUS,              // 21
			XGETYOU_MUS,             // 22
			XTOWER2_MUS              // 23
#endif
		};
//	struct dostime_t time;



	IN_ClearKeysDown();
	if (!AdLibPresent && !SoundBlasterPresent)
		return;


	MenuFadeOut();

#ifndef SPEAR
// *** S3DNA RESTORATION ***
#if (!defined UPLOAD) && (!defined GAMEVER_NOAH3D)
//#ifndef UPLOAD
	// REFKEEN: Use second isn't of hundredth of a sec
	int hour, min, sec;
	BE_Cross_GetLocalTime_UNSAFE(&hour, &min, &sec);
	start = (sec%3)*6;
//	_dos_gettime(&time);
//	start = (time.hsecond%3)*6;
#else
	start = 0;
#endif
#else
	start = 0;
#endif


	CA_CacheGrChunk (STARTFONT+1);
#ifdef SPEAR
	CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#else
	CacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#endif
	CA_LoadAllSounds ();

	fontnumber=1;
	ClearMScreen ();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VWB_DrawPic(112,192,C_MOUSELBACKPIC);
#else
	VWB_DrawPic(112,184,C_MOUSELBACKPIC);
	DrawStripes (10);
#endif
	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	DrawWindow (CTL_X-2,CTL_Y-36,280,13*12,BKGDCOLOR);
#elif (!defined SPEAR)
//#ifndef SPEAR
	DrawWindow (CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);
#else
	DrawWindow (CTL_X-2,CTL_Y-26,280,13*10,BKGDCOLOR);
#endif

	DrawMenu (&MusicItems,&MusicMenu[start]);

	SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
	PrintY=15;
	WindowX = 0;
	WindowY = 320;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	US_CPrint ("Noah Tunes");
#else
	US_CPrint ("Robert's Jukebox");
#endif

	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);
	VW_UpdateScreen();
	MenuFadeIn();

	do
	{
		which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
		if (which>=0)
		{
			if (lastsong >= 0)
				MusicMenu[start+lastsong].active = 1;

			StartCPMusic(songs[start + which]);
			MusicMenu[start+which].active = 2;
			DrawMenu (&MusicItems,&MusicMenu[start]);
			VW_UpdateScreen();
			lastsong = which;
		}
	} while(which>=0);

	MenuFadeOut();
	IN_ClearKeysDown();
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
#ifdef SPEAR
	UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#else
	UnCacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#endif
#endif
}
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL1AP10


/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

void InitGame (void)
{
	id0_int_t                     i,x,y;
	id0_unsigned_t        *blockstart;

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	noquestions = false;
	if (MS_CheckParm ("noquestions"))
		noquestions = true;
	nofloors = false;
	if (MS_CheckParm ("nofloors"))
		nofloors = true;
#elif (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (MS_CheckParm ("virtual"))
		virtualreality = true;
	else
		virtualreality = false;
#endif

	MM_Startup ();                  // so the signon screen can be freed

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	SignonScreen ();
#endif

	VW_Startup ();
	IN_Startup ();
	PM_Startup ();
	PM_UnlockMainMem ();
	SD_Startup ();
	CA_Startup ();
	US_Startup ();


// *** SHAREWARE V1.0 APOGEE + ALPHA + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	if (mminfo.mainmem < 240000L)
#elif (defined GAMEVER_NOAH3D)
	if (mminfo.mainmem < 275000L && !MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
#elif (!defined SPEAR)
//#ifndef SPEAR
	if (mminfo.mainmem < 235000L)
#else
	if (mminfo.mainmem < 257000L && !MS_CheckParm("debugmode"))
#endif
	{
		id0_byte_t *screen;
//		memptr screen;

		CA_CacheGrChunk (ERRORSCREEN);
		screen = (id0_byte_t *)grsegs[ERRORSCREEN];
		ShutdownId();
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
		memcpy(BE_ST_GetTextModeMemoryPtr(), screen + 7 + 8*160, 15*160);
//		movedata ((id0_unsigned_t)screen,7+8*160,0xb800,0,15*160);
#else
		memcpy(BE_ST_GetTextModeMemoryPtr(), screen + 7 + 7*160, 17*160);
//		movedata ((id0_unsigned_t)screen,7+7*160,0xb800,0,17*160);
#endif
		BE_ST_gotoxy (1,23);
		BE_ST_HandleExit(1);
	}
#endif // GAMEVER_WOLFREV > GV_WR_WL920312


//
// build some tables
//
	InitDigiMap ();

	for (i=0;i<MAPSIZE;i++)
	{
		nearmapylookup[i] = &tilemap[0][0]+MAPSIZE*i;
		farmapylookup[i] = i*64;
	}

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	updateptr = &update[0];
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	*(id0_unsigned_t *)(updateptr+UPDATEWIDE*UPDATEHIGH) = UPDATETERMINATE;
#endif

	bufferofs = 0;
	displayofs = 0;
	ReadConfig ();


	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
//
// HOLDING DOWN 'M' KEY?
//

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (!defined SPEARDEMO) && (GAMEVER_WOLFREV > GV_WR_WL1AP10)
//#ifndef SPEARDEMO
	BE_ST_ShortSleep();
	if (Keyboard[sc_M])
	  DoJukebox();
	else
#endif
//
// draw intro screen stuff
//
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (!virtualreality)
#endif
		IntroScreen ();
#endif // S3DNA RESTORATION

//
// load in and lock down some basic chunks
//

	CA_CacheGrChunk(STARTFONT);
	MM_SetLock (&grsegs[STARTFONT],true);

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	LoadLatchMem ();
#endif
	BuildTables ();          // trig tables
	SetupWalls ();

#if 0
{
id0_int_t temp,i;
temp = viewsize;
	profilehandle = open("SCALERS.TXT", O_CREAT | O_WRONLY | O_TEXT);
for (i=1;i<20;i++)
	NewViewSize(i);
viewsize = temp;
close(profilehandle);
}
#endif

	NewViewSize (viewsize);


//
// initialize variables
//
	InitRedShifts ();
	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (!virtualreality)
#endif
		FinishSignon();
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	displayofs = PAGE1START;
	bufferofs = PAGE2START;
#endif

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	if (MS_CheckParm ("CHECK"))
	{
		BE_ST_printf("Press any key...");
		IN_Ack();
	}
	else
		IN_UserInput (TickBase);

	VL_SetVGAPlaneMode ();
	VL_TestPaletteSet ();
	VL_SetPalette (gamepal);
	VW_FadeOut ();

	LoadLatchMem ();
#elif (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	if (virtualreality)
	{
		NoWait = true;
#if REFKEEN_WL_ENABLE_VR
		geninterrupt(0x60);
#endif
	}
#endif
}

//===========================================================================

/*
==========================
=
= SetViewSize
=
==========================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
void SetViewSize (id0_unsigned_t width, id0_unsigned_t height)
#else
id0_boolean_t SetViewSize (id0_unsigned_t width, id0_unsigned_t height)
#endif
{
	viewwidth = width&~15;                  // must be divisable by 16
	viewheight = height&~1;                 // must be even
	centerx = viewwidth/2-1;
	shootdelta = viewwidth/10;
	screenofs = ((200-STATUSLINES-viewheight)/2*SCREENWIDTH+(320-viewwidth)/8);

//
// calculate trace angles and projection constants
//
	CalcProjection (FOCALLENGTH);

//
// build all needed compiled scalers
//
//	MM_BombOnError (false);
	SetupScaling (viewwidth*1.5);
#if 0
	MM_BombOnError (true);
	if (mmerror)
	{
		Quit ("Can't build scalers!");
		mmerror = false;
		return false;
	}
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	SetPlaneViewSize ();
#endif
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	return true;
#endif
}


void ShowViewSize (id0_int_t width)
{
	id0_int_t     oldwidth,oldheight;

	oldwidth = viewwidth;
	oldheight = viewheight;

	viewwidth = width*16;
	viewheight = width*16*HEIGHTRATIO;
	DrawPlayBorder ();

	viewheight = oldheight;
	viewwidth = oldwidth;
}


void NewViewSize (id0_int_t width)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	CA_UpLevel ();
	MM_SortMem ();
#endif
	viewsize = width;
	// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
resize:
	if (!SetViewSize (width*16,width*16*HEIGHTRATIO))
	{
		--width;
		goto resize;
	}
#else
	SetViewSize (width*16,width*16*HEIGHTRATIO);
#endif
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	CA_DownLevel ();
#endif
}



//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/

void Quit (const id0_char_t *error)
{
	id0_unsigned_t        finscreen;
	id0_byte_t *screen;
//	memptr	screen;

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11) && (!defined GAMEVER_NOAH3D)
#if REFKEEN_WL_ENABLE_VR
	if (virtualreality)
		geninterrupt(0x61);
#endif
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL1AP10)
	ClearMemory ();
#endif
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 if (CA_Started)
 {
#endif
	// REFKEEN: null pointer dereference fix. Even with the
	// original EXEs, these should be the behaviors if nothing's wrong.
	if (!error || !*error)
//	if (!*error)
	{
	 #ifndef JAPAN
	 CA_CacheGrChunk (ORDERSCREEN);
	 screen = (id0_byte_t *)grsegs[ORDERSCREEN];
	 #endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	 WriteConfig ();
#endif
	}
	else
	{
	 CA_CacheGrChunk (ERRORSCREEN);
	 screen = (id0_byte_t *)grsegs[ERRORSCREEN];
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 }
#endif
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
	WriteConfig ();
#endif
	ShutdownId ();

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 if (CA_Started)
 {
	BE_ST_clrscr();
#endif
	if (error && *error)
	{
	  // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	  // *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	  memcpy(BE_ST_GetTextModeMemoryPtr(), screen, 7*160);
//	  movedata ((id0_unsigned_t)screen,0,0xb800,0,7*160);
#else
	  memcpy(BE_ST_GetTextModeMemoryPtr(), screen + 7, 7*160);
//	  movedata ((id0_unsigned_t)screen,7,0xb800,0,7*160);
#endif
	  BE_ST_gotoxy (10,4);
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
	  BE_ST_puts(error);
	  // *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	  BE_ST_gotoxy (1,8);
#endif
	  BE_ST_HandleExit(1);
	}
	// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	else
#endif
	if (!error || !(*error))
	{
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		if (tedlevel)
			execlp("TED5.EXE","TED5.EXE","/LAUNCH",NULL);
#endif
		BE_ST_clrscr();
		// *** ACTIVISION RELEASES + ALPHA RESTORATION ***
		// This should be commented out in the 1.4 Activision releases
		// of Wolf3D and SOD (no copy protection), but compiled in the
		// (shareware/registered) Apogee and SOD (demo) releases,
		// as well as the other "GOODTIMES" releases;
		// Absent from the Alpha build, though.
		#if (!defined JAPAN) && (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL6GT14B)
		//#ifndef JAPAN
		// *** S3DNA RESTORATION ***
		#ifdef GAMEVER_NOAH3D
		memcpy(BE_ST_GetTextModeMemoryPtr(), screen, 4000);
//		movedata ((id0_unsigned_t)screen,0,0xb800,0,4000);
		#else
		memcpy(BE_ST_GetTextModeMemoryPtr(), screen + 7, 4000);
//		movedata ((id0_unsigned_t)screen,7,0xb800,0,4000);
		#endif
		BE_ST_gotoxy(1,24);
		#endif
//asm	mov	bh,0
//asm	mov	dh,23	// row
//asm	mov	dl,0	// collumn
//asm	mov ah,2
//asm	int	0x10
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
 }
 else
 {
	VL_SetTextMode();
	if (error && *error)
		BE_ST_puts(error);
 }
#endif

	BE_ST_HandleExit(0);
}

// *** ALPHA RESTORATION ***
// The functions included here, DrawHighScores and CheckHighScore,
// moved to WL_INTER.C at some point
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
#include "wl_hscor.c"
#endif

//===========================================================================



/*
=====================
=
= DemoLoop
=
=====================
*/

// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
static  const id0_char_t *ParmStrings[] = {"easy","normal","hard",""};
#else
static  const id0_char_t *ParmStrings[] = {"baby","easy","normal","hard",""};
#endif

void	PG13 (void); // REFKEEN: Need this for DemoLoop

void    DemoLoop (void)
{
	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	static id0_int_t LastDemo;
#endif
	id0_int_t     i,level;
	id0_long_t nsize;
	memptr	nullblock;

//
// check for launch from ted
//
	if (tedlevel)
	{
		NoWait = true;
		// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
		NewGame(1,0);
#endif

		for (i = 1;i < id0_argc;i++)
		{
			if ( (level = US_CheckParm(id0_argv[i],ParmStrings)) != -1)
			{
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			 gamestate.difficulty=level;
#endif
			 break;
			}
		}

			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
		NewGame(level);
#endif
		// *** S3DNA + ALPHA RESTORATION ***
#if (defined GAMEVER_NOAH3D) || (GAMEVER_WOLFREV <= GV_WR_WL920312)
		gamestate.mapon = tedlevelnum;
#elif (!defined SPEAR)
//#ifndef SPEAR
		gamestate.episode = tedlevelnum/10;
		gamestate.mapon = tedlevelnum%10;
#else
		gamestate.episode = 0;
		gamestate.mapon = tedlevelnum;
#endif
		GameLoop();
		Quit (NULL);
	}


//
// main game cycle
//


//	nsize = (id0_long_t)40*1024;
//	MM_GetPtr(&nullblock,nsize);

#ifndef DEMOTEST

	#ifndef UPLOAD

		#ifndef GOODTIMES
		#ifndef SPEAR
		#ifndef JAPAN
		// *** S3DNA RESTORATION ***
		#ifndef GAMEVER_NOAH3D
		if (!NoWait)
			NonShareware();
		#endif
		#endif
		#else

			#ifndef GOODTIMES
			#ifndef SPEARDEMO
			CopyProtection();
			#endif
			#endif

		#endif
		#endif
	#endif

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
	displayofs = bufferofs = 0;
#endif
// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
	StartCPMusic(INTROSONG);
#endif

// *** S3DNA + ALPHA RESTORATION ***
#if (!defined JAPAN) && (!defined GAMEVER_NOAH3D) && (GAMEVER_WOLFREV > GV_WR_WL920312)
//#ifndef JAPAN
	if (!NoWait)
		PG13 ();
#endif

#endif

	while (1)
	{
		while (!NoWait)
		{
//
// title page
//
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			CA_CacheGrChunk (TITLEPIC);
#else
			MM_SortMem ();
#endif
			// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
			bufferofs = 19200;
			displayofs = 0;
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			VWB_DrawPic (0,0,TITLEPIC);
			UNCACHEGRCHUNK (TITLEPIC);
#else
			VW_SetCRTC(displayofs);
#endif
#endif // GAMEVER_WOLFREV <= GV_WR_WL920312
#ifndef DEMOTEST

			// *** S3DNA RESTORATION ***
#if (defined SPEAR) || (defined GAMEVER_NOAH3D)
//#ifdef SPEAR
			CA_CacheGrChunk (TITLEPALETTE);

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			CA_CacheScreen(TITLEPIC);
#else
			CA_CacheGrChunk (TITLE1PIC);
			VWB_DrawPic (0,0,TITLE1PIC);
			UNCACHEGRCHUNK (TITLE1PIC);

			CA_CacheGrChunk (TITLE2PIC);
			VWB_DrawPic (0,80,TITLE2PIC);
			UNCACHEGRCHUNK (TITLE2PIC);
#endif
			VW_UpdateScreen ();
			VL_FadeIn(0,255,(id0_byte_t *)grsegs[TITLEPALETTE],30);

			UNCACHEGRCHUNK (TITLEPALETTE);
#else
			// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			CA_CacheScreen (TITLEPIC);
#endif
			// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			if (screenfaded)
			{
				VW_UpdateScreen ();
				VW_FadeIn();
			}
			else
#endif
			{
				if (FizzleFade(bufferofs,displayofs,320,200,20,true))
					break;
			}
#else
			VW_UpdateScreen ();
			VW_FadeIn();
#endif
#endif
			// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			if (IN_UserInput(TickBase))
				break;
#elif (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			if (IN_UserInput(TickBase*3))
				break;
#else
			if (IN_UserInput(TickBase*15))
				break;
			VW_FadeOut();
#endif
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
			CA_CacheGrChunk (COPYRIGHTPIC);
			VWB_DrawPic (0,184,COPYRIGHTPIC);
			VW_UpdateScreen ();
			CA_LoadAllSounds ();
			UNCACHEGRCHUNK (COPYRIGHTPIC);
			if (IN_UserInput(TickBase))
				break;

			CA_CacheGrChunk (FAMILYAPIC);
			VWB_DrawPic (224,112,FAMILYAPIC);
			VW_UpdateScreen ();
			UNCACHEGRCHUNK (FAMILYAPIC);
			if (IN_UserInput(TickBase*13))
				break;

			VW_FadeOut ();
			Briefing (6,true);
#endif
//
// credits page
//
			// *** S3DNA + ALPHA RESTORATION ***
#ifndef GAMEVER_NOAH3D
	#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			CA_CacheGrChunk (CREDITSPIC);
			VWB_DrawPic (0,0,CREDITSPIC);
			UNCACHEGRCHUNK (CREDITSPIC);
	#else
			CA_CacheScreen (CREDITSPIC);
	#endif
#endif
			// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
			// A bit of additional restored code
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
			if (FizzleFade(bufferofs,displayofs,320,200,20,true))
				break;
	#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			if (IN_UserInput(TickBase*3))
	#else
			if (IN_UserInput(TickBase*10))
	#endif
				break;
#endif
			// *** REGISTERED SOD V1.4 ACTIVISION RESTORATION ***
			// Different restored code
#if (defined SPEAR) && (GAMEVER_WOLFREV > GV_WR_SODFG14A)
			VW_ScreenToScreen (bufferofs,bufferofs+ylookup[175],30,17);
			VW_ScreenToScreen (bufferofs+ylookup[168]+30,displayofs,50,30);
			VW_ScreenToScreen (bufferofs+ylookup[42],bufferofs+ylookup[170]+30,20,30);
			VW_ScreenToScreen (bufferofs+ylookup[42],bufferofs+ylookup[165]+53,25,30);
			VW_ScreenToScreen (displayofs,bufferofs+ylookup[168]+18,50,30);
#endif
			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			VW_UpdateScreen();
			VW_FadeIn ();
			if (IN_UserInput(TickBase*10))
				break;
			VW_FadeOut ();
//
// high scores
//
			DrawHighScores ();
			VW_UpdateScreen ();
			VW_FadeIn ();

			if (IN_UserInput(TickBase*10))
				break;
#endif // GAMEVER_WOLFREV > GV_WR_WL6AP11
#endif
//
// demo
//

			// *** S3DNA + ALPHA RESTORATION ***
			#ifdef GAMEVER_NOAH3D
			PlayDemo (LastDemo++%3);
			#elif (!defined SPEARDEMO) && (GAMEVER_WOLFREV > GV_WR_WL920312)
			//#ifndef SPEARDEMO
			PlayDemo (LastDemo++%4);
			#else
			PlayDemo (0);
			#endif

			if (playstate == ex_abort)
				break;
			// *** PRE-V1.4 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL6AP11)
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			StartCPMusic(ROSTER_MUS);
#endif

			DrawHighScores();
			VW_UpdateScreen();
			VW_FadeIn();

#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
			if (IN_UserInput(TickBase*3))
#else
			if (IN_UserInput(TickBase*9))
#endif
				break;
#endif
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
			StartCPMusic(INTROSONG);
#endif

		}

		VW_FadeOut ();

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
		if (Keyboard[sc_M])
			DoJukebox();
#endif
		// *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
#ifndef SPEAR
		if (Keyboard[sc_Tab] && MS_CheckParm(GAMEVER_WOLF3D_DEBUGPARM))
#else
		if (Keyboard[sc_Tab] && MS_CheckParm("debugmode"))
#endif
			RecordDemo ();
		else
#endif // GAMEVER_WOLFREV > GV_WR_WL920312
			US_ControlPanel (0);

		if (startgame || loadedgame)
		{
			GameLoop ();
			// *** PRE-V1.4 APOGEE RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL6AP11)
			VW_FadeOut();
			StartCPMusic(INTROSONG);
#endif
		}
	}
}


//===========================================================================


/*
==========================
=
= main
=
==========================
*/

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_NOAH3D
const id0_char_t    *nosprtxt[] = {"nospr",id0_nil_t};
#endif

void wolf3d_exe_main (void)
{
	id0_int_t     i;


	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV <= GV_WR_WL920312)
	for (i = 1;i<id0_argc;i++)
		if (US_CheckParm(id0_argv[i],nosprtxt) == 0)
			nospr = true;
#endif

#ifdef BETA
	//
	// THIS IS FOR BETA ONLY!
	//
	struct dosdate_t d;

	_dos_getdate(&d);
	if (d.year > YEAR ||
		(d.month >= MONTH && d.day >= DAY))
	{
	 BE_ST_printf("Sorry, BETA-TESTING is over. Thanks for you help.\n");
	 BE_ST_HandleExit(1);
	}
#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	VGAMode = true;
	VL_SetTextMode();
	FillCharAttr(' ',0x5d,80);
	BE_ST_gotoxy((79-strlen(logon))/2,1);
	BE_ST_puts(logon);
#endif

	// *** ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
	CheckForEpisodes();
#endif

	//Patch386 (); // REFKEEN - Not needed

	InitGame ();

	DemoLoop();

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_NOAH3D
	Quit("FatalError: DemoLoop() exited!");
#else
	Quit("Demo loop exited???");
#endif
}

// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Some unused function
#if 0 // REFKEEN: Just ignore it
//#if (GAMEVER_WOLFREV > GV_WR_WL920312) && (GAMEVER_WOLFREV <= GV_WR_WL1AP10)
id0_long_t GetRandomTableSum (void)
{
	extern id0_far id0_byte_t rndtable[];
	id0_long_t result = 0;
	id0_int_t i;
	for (i=0;i<0x100;i++)
		result += rndtable[i];
	return result;
}
#endif

// (REFKEEN) Used for loading data from DOS EXE (instead of hardcoding)
id0_byte_t *gamepal;
id0_char_t *signon;


void RefKeen_Load_Embedded_Resources_From_wolf3d_exe(void)
{
	id0_longword_t unusedsize;

	if (!(gamepal = (id0_byte_t *)BE_Cross_BfarmallocFromEmbeddedData("GAMEPAL.BIN", &unusedsize))
#ifndef GAMEVER_NOAH3D
	    || !(signon = (id0_char_t *)BE_Cross_BfarmallocFromEmbeddedData("SIGNON.BIN", &unusedsize))
#endif
	)
		// Don't use quit, yet
		BE_ST_ExitWithErrorMsg("RefKeen_Load_Embedded_Resources_From_wolf3d_exe - Failed to load\nat least one file.");
}

REFKEEN_NS_E
