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

#include "id_heads.h"


//       ID_SD_A.ASM
//       Id Sound Manager assembly stuff

// Ported from ASM, while re-using services from id_sd.c with minor changes

REFKEEN_NS_B

#define DEBUG 0

void SDL_PCService(void);
void SDL_ALSoundService(void);
#ifdef GAMEVER_NOAH3D // *** S3DNA RESTORATION ***
void MIDI_IRQService(void);
#else
void SDL_ALService(void);
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
void SDL_SSService(void);
#endif
#endif

void SDL_PCStopSound(void);
void SDL_DigitizedDone(void);

void SDL_t0FastAsmService(void);

// ==========================================================================

//extern id0_long_t LocalTime;
extern id0_boolean_t sqActive;
extern volatile id0_byte_t *pcSound;
extern id0_longword_t pcLengthLeft;
extern id0_byte_t *alSound;
#ifndef GAMEVER_NOAH3D // *** S3DNA RESTORATION ***
extern volatile id0_byte_t *ssSample;
extern volatile id0_longword_t ssLengthLeft;
extern id0_word_t ssControl, ssStatus, ssData;
extern id0_byte_t ssOn, ssOff;
#if (GAMEVER_WOLFREV > GV_WR_WL920312)
id0_boolean_t pcindicate;
#endif
#endif

// REFKEEN - We may use uint_fast16_t instead of id0_word_t
//static uint_fast16_t count_time;
static uint_fast16_t count_fx;
static uint_fast16_t extreme;

// REFKEEN NOTES: Let's omit COMMONSTART debug code setting overscan color.
// Also omit COMMONEND and the variables it changes; We don't need them now.

//
//	DOFX
//	Macro used to do the sound effects code. Ported from ASM to a function.
//
static void DOFX(void)
{
	SDL_PCService();
	SDL_ALSoundService();
}

#if 0 // REFKEEN: LocalTime has no use and TimeCount is updated differently
static void TIME(void)
{
	if (count_time >= 2)
	{
//		++LocalTime;
		++TimeCount;
		count_time = 0;
	}
}
#endif

#ifndef GAMEVER_NOAH3D // *** S3DNA + ALPHA RESTORATION ***
#if (GAMEVER_WOLFREV > GV_WR_WL920312)

//
//	SDL_IndicatePC
//
void SDL_IndicatePC(id0_boolean_t on) { pcindicate = on; }

//
//	SDL_t0ExtremeAsmService
//	Timer 0 ISR 7000Hz interrupts
//
void SDL_t0ExtremeAsmService(void)
{
	if (!pcindicate)
		return;

	if (pcSound)
	{
		id0_byte_t s = *pcSound++;
		// Nuke some of the precision
		BE_ST_PCSpeakerSetVal(s & 0x80 != 0);

		if (!(--pcLengthLeft))
		{
			SDL_PCStopSound();
			SDL_DigitizedDone();
		}
	}

	if (++extreme < 10)
		return;
	extreme = 0;
	SDL_t0FastAsmService();
}

#endif // GAMEVER_WOLFREV
#endif // GAMEVER_NOAH3D

//
//	SDL_t0FastAsmService
//	Timer 0 ISR for 700Hz interrupts
//
void SDL_t0FastAsmService(void)
{
	++count_fx;
#ifndef GAMEVER_NOAH3D // *** S3DNA RESTORATION ***
	if ((count_fx < 5) && !sqActive
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	    && !ssSample
#endif
	)
		return;
#endif

	if (count_fx >= 5)
	{
		count_fx = 0;
		DOFX();
#if 0
		++count_time;
		TIME();
#endif
	}

#ifdef GAMEVER_NOAH3D // *** S3DNA RESTORATION ***
	MIDI_IRQService();
#else
	SDL_ALService();
#if REFKEEN_SD_ENABLE_SOUNDSOURCE
	SDL_SSService();
#endif
#endif
}

//
//	SDL_t0SlowAsmService
//	Timer 0 ISR for 140Hz interrupts
//
//
void SDL_t0SlowAsmService(void)
{
#if 0
	++count_time;
	TIME();
#endif
	if (pcSound || alSound)
		DOFX();
}

REFKEEN_NS_E
