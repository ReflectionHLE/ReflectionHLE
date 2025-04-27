/* Reconstructed BioMenace Source Code
 * Copyright (C) 2017-2025 K1n9_Duk3
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

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "version.h"

/////////////////////////////////////////////////
//
// MUSE Header for .BM? (all 3 episodes)
//
/////////////////////////////////////////////////

#define NUMSOUNDS     LASTSOUND
#define NUMSNDCHUNKS  ((3*LASTSOUND)+LASTMUSIC)

REFKEEN_NS_B

//
// Sound names & indexes
//
typedef enum {
	JUMPBUGSND,        // 0
	SNAKEHITSND,       // 1
	CHOMPWALKSND,      // 2
	GORPATTACKSND,     // 3
	GORPSPLATSND,      // 4
	SNAKEDIESND,       // 5
	SNAKESHOOTSND,     // 6
	GRENADEXPLODESND,  // 7
	CHUNKSPLATSND,     // 8
	COBRASHOOTSND,     // 9
	BUGSCREAMSND,      //10
	SNAKEJUMPSND,      //11
	MACHINEGUNSND,     //12
	RICOCHET1SND,      //13
	POWERUPSND,        //14
	GETKEYSND,         //15
	EXTRASNAKESND,     //16
	ACCESSDOORSND,     //17
	ACCESSDENIEDSND,   //18
	OPENDOORSND,       //19
	CLICKSND,          //20
	LASERSND,          //21
	CRYSTALSND,        //22
	GROWLSND,          //23
	MANGLESND,         //24
	APOGEESND,         //25
	BLOBSND,           //26
	YUMSND,            //27
	LIFEPOINTSND,      //28
	STUNRAYSND,        //29
	RAYHITSND,         //30
	EGGSND,            //31
	INVISOSND,         //32
	ICON5000SND,       //33
	LASERHITSND,       //34
	CRUSHERSND,        //35
	FALLBLOCKSND,      //36
	ROCKSLIDESND,      //37
// Note: All of the sound names above were taken directly from the MUSEINFO.BH1
// file that comes with the BioHazard beta. The respective sounds are identical
// to the sounds in the final game episodes.
#ifndef BETA
#if (EPISODE == 1)
	SEWERMANATTACKSND, //38
	ACIDSND,           //39
	BEACONSND,         //40
	TIMERTICKSND,      //41
#elif (EPISODE == 2)
	ACIDSND,           //38
	BEACONSND,         //39
#elif (EPISODE == 3)
	BEACONSND,         //38
	ACIDSND,           //39
#endif
#endif
	LASTSOUND
} soundnames;

//
// Base offsets
//
#define STARTPCSOUNDS     0
#define STARTADLIBSOUNDS  (STARTPCSOUNDS+NUMSOUNDS)
#define STARTDIGISOUNDS   (STARTADLIBSOUNDS+NUMSOUNDS)
#define STARTMUSIC        (STARTDIGISOUNDS+NUMSOUNDS)

//
// Music names & indexes
// (based on the file names found at the end of the IMF data in the AUDIOT file)
//
typedef enum {
	CRUISING_MUS, // 0
#if EPISODE == 1
	WEASEL_MUS,   // 1
	BIOTHEM1_MUS, // 2
	SNAKSAVE_MUS, // 3
	DIRTYH2O_MUS, // 4
	ROCKINIT_MUS, // 5
	BAYOU_MUS,    // 6
	ROBOTY_MUS,   // 7
	PRISONER_MUS, // 8
	DRSHOCK_MUS,  // 9
	CHASING_MUS,  //10
	LIKITWAS_MUS, //11
	APOGFANF_MUS, //12
	SAVED_MUS,    //13
	RESTING_MUS,  //14
	CANTGET_MUS,  //15
	NONVINCE_MUS, //16
	XCITY_MUS,    //17
#elif EPISODE == 2
	ANTSPANT_MUS, // 1
	SQUEAKY_MUS,  // 2
	BITBYTE_MUS,  // 3
	SNAKSAVE_MUS, // 4
	DRSHOCK_MUS,  // 5
	BIOTHEME_MUS, // 6
	PRISONER_MUS, // 7
	ROBOTY_MUS,   // 8
	BAYOU_MUS,    // 9
	DIRTYH2O_MUS, //10
	MUTCMPUT_MUS, //11
	WHODOL_MUS,   //12
	CHASING_MUS,  //13
	GUESS_MUS,    //14
	SAVED_MUS,    //15
	RESTING_MUS,  //16
#ifndef BETA
	APOGFANF_MUS, //17
	CANTGET_MUS,  //18
	NONVINCE_MUS, //19
#endif
#elif EPISODE == 3
	XSEWER_MUS,   // 1
	XUNCRTN_MUS,  // 2
	BITBYTE_MUS,  // 3
	XSLUDGE_MUS,  // 4
	SNAKSAVE_MUS, // 5
	BIOTHEME_MUS, // 6
	ROCKINIT_MUS, // 7
	BAYOU_MUS,    // 8
	XCIRCLES_MUS, // 9
	XSNEAKY_MUS,  //10
	XTHEEVIL_MUS, //11
	APOGFANF_MUS, //12
	SAVED_MUS,    //13
	RESTING_MUS,  //14
	CANTGET_MUS,  //15
	NONVINCE_MUS, //16
#endif
	LASTMUSIC
} musicnames;

/////////////////////////////////////////////////
//
// Thanks for playing with MUSE!
//
/////////////////////////////////////////////////

//arguments for 'StartMusic()': (mapping to 'musicnames' is defined in BM_PLAY.C)

typedef enum {
	MUS_LEVEL1,
	MUS_LEVEL2,
	MUS_LEVEL3,
	MUS_LEVEL4,
	MUS_LEVEL5,
	MUS_LEVEL6,
	MUS_LEVEL7,
	MUS_LEVEL8,
	MUS_LEVEL9,
	MUS_LEVEL10,
	MUS_LEVEL11,
	MUS_LEVEL12,
	MUS_HIGHSCORELEVEL,
#ifdef BETA
	MUS_VICTORY,
	MUS_HOSTAGE,
	MUS_NO_MUSIC
#else
	MUS_INTROLEVEL,
	MUS_VICTORY,
	MUS_HOSTAGE,
	MUS_MENU,
	MUS_INVINCIBLE,
	MUS_NO_MUSIC
#endif
} musicnum;

REFKEEN_NS_E

#endif
