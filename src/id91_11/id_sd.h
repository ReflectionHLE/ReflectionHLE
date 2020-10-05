/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
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

//
//	ID Engine
//	ID_SD.h - Sound Manager Header
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__TYPES__
#include "id_types.h"
#endif

#ifndef	__ID_SD__
#define	__ID_SD__

REFKEEN_NS_B

#ifdef	__DEBUG__
#define	__DEBUG_SoundMgr__
#endif

#define	TickBase	70		// 70Hz per tick - used as a base for timer 0

typedef	enum	{
					sdm_Off,
					sdm_PC,sdm_AdLib,
				}	SDMode;
typedef	enum	{
					smm_Off,smm_AdLib
				}	SMMode;

#pragma pack(push, 1)

typedef	struct
		{
			id0_longword_t	length;
			id0_word_t		priority;
		} SoundCommon;

//	PC Sound stuff
#define	pcTimer		0x42
#define	pcTAccess	0x43
#define	pcSpeaker	0x61

#define	pcSpkBits	3

typedef	struct
		{
			SoundCommon	common;
			id0_byte_t		data[1];
		} PCSound;

// 	Registers for the Sound Blaster card - needs to be offset by n0
#define	sbReset		0x206
#define	sbReadData	0x20a
#define	sbWriteCmd	0x20c
#define	sbWriteData	0x20c
#define	sbWriteStat	0x20c
#define	sbDataAvail	0x20e

typedef	struct
		{
			SoundCommon	common;
			id0_word_t		hertz;
			id0_byte_t		bits,
						reference,
						data[1];
		} SampledSound;

// 	Registers for the AdLib card
// Operator stuff
#define	alChar		0x20
#define	alScale		0x40
#define	alAttack	0x60
#define	alSus		0x80
#define	alWave		0xe0
// Channel stuff
#define	alFreqL		0xa0
#define	alFreqH		0xb0
#define	alFeedCon	0xc0
// Global stuff
#define	alEffects	0xbd

typedef	struct
		{
			id0_byte_t	mChar,cChar,
					mScale,cScale,
					mAttack,cAttack,
					mSus,cSus,
					mWave,cWave,
					nConn,

					// These are only for Muse - these bytes are really unused
					voice,
					mode,
					unused[3];
		} Instrument;

typedef	struct
		{
			SoundCommon	common;
			Instrument	inst;
			id0_byte_t		block,
						data[1];
		} AdLibSound;

//
//	Sequencing stuff
//
#define	sqMaxTracks	10
#define	sqMaxMoods	1	// DEBUG

#define	sev_Null		0	// Does nothing
#define	sev_NoteOff		1	// Turns a note off
#define	sev_NoteOn		2	// Turns a note on
#define	sev_NotePitch	3	// Sets the pitch of a currently playing note
#define	sev_NewInst		4	// Installs a new instrument
#define	sev_NewPerc		5	// Installs a new percussive instrument
#define	sev_PercOn		6	// Turns a percussive note on
#define	sev_PercOff		7	// Turns a percussive note off
#define	sev_SeqEnd		-1	// Terminates a sequence

// 	Flags for MusicGroup.flags
#define	sf_Melodic		0
#define	sf_Percussive	1

#if 1
typedef	struct
		{
			id0_word_t	length,
					values[1];
		} MusicGroup;
#else
typedef	struct
		{
			id0_word_t	flags,
					count,
					offsets[1];
		} MusicGroup;
#endif

typedef	struct
		{
			/* This part needs to be set up by the user */
			id0_word_t        mood,id0_far *moods[sqMaxMoods];

			/* The rest is set up by the code */
			Instrument	inst;
			id0_boolean_t		percussive;
			id0_word_t		id0_far *seq;
			id0_longword_t	nextevent;
		} ActiveTrack;

#pragma pack(pop)

#define	sqmode_Normal		0
#define	sqmode_FadeIn		1
#define	sqmode_FadeOut		2

#define	sqMaxFade		64	// DEBUG


// Global variables
extern	id0_boolean_t		AdLibPresent,
					NeedsMusic;	// For Caching Mgr
extern	SDMode		SoundMode;
extern	SMMode		MusicMode;
//NOT DECLARED HERE - USE WRAPPERS LIKE SD_GetTimeCount
//extern	id0_longword_t	TimeCount;					// Global time in ticks
#ifdef REFKEEN_VER_CATADVENTURES
extern 	SDMode		oldsoundmode;
#endif

// Function prototypes
extern	void	SD_Startup(void),
				SD_Shutdown(void),
				SD_Default(id0_boolean_t gotit,SDMode sd,SMMode sm),
				SD_PlaySound(soundnames sound),
				SD_StopSound(void),
				SD_WaitSoundDone(void),
				SD_StartMusic(MusicGroup id0_far *music),
				SD_MusicOn(void),
				SD_MusicOff(void),
				SD_FadeOutMusic(void),
				SD_SetUserHook(void (*hook)(void));
extern	id0_boolean_t	SD_MusicPlaying(void),
				SD_SetSoundMode(SDMode mode),
				SD_SetMusicMode(SMMode mode);
extern	id0_word_t	SD_SoundPlaying(void);

#ifdef	_MUSE_	// MUSE Goes directly to the lower level routines
extern	void	SDL_PCPlaySound(PCSound id0_far *sound),
				SDL_PCStopSound(void),
				SDL_ALPlaySound(AdLibSound id0_far *sound),
				SDL_ALStopSound(void);
#endif

// Replacements for direct accesses to TimeCount variable
id0_longword_t SD_GetTimeCount(void);
void SD_SetTimeCount(id0_longword_t newcount);
void SD_AddToTimeCount(id0_longword_t count);
// Use this as a replacement for bosy loops waiting for TimeCount
// to reach destination, as in "while (TimeCount<dst)
void SD_TimeCountWaitForDest(id0_longword_t dst);
// Use this as a replacement for busy loops waiting for some ticks
// to pass, as in "while (TimeCount-src<ticks)"
static inline void SD_TimeCountWaitFromSrc(id0_longword_t src, id0_int_t ticks)
{
	SD_TimeCountWaitForDest(src + ticks);
}
// Essentially SD_TimeCountWaitFromSrc with (src == SD_GetTimeCount())
static inline void SD_TimeCountWaitTicks(id0_int_t ticks)
{
	SD_TimeCountWaitForDest(SD_GetTimeCount() + ticks);
}

REFKEEN_NS_E

#endif
