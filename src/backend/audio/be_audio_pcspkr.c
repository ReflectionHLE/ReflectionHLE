/* Copyright (C) 2014-2021 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#include "refkeen.h"
#include "be_audio_private.h"

// Changing between 0 and max. possible value - too loud
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
#define MIXER_SAMPLE_PCSPEAKER_TOP_VAL 0.75f
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
#define MIXER_SAMPLE_PCSPEAKER_TOP_VAL 24575
#endif

static int g_be_pcSpeakerSampleFreq;
// Current PC Speaker status
static bool g_sdlPCSpeakerConnected = false;
static BE_ST_SndSample_T g_sdlCurrentBeepSample;
static uint32_t g_sdlBeepHalfCycleCounter, g_sdlBeepHalfCycleCounterUpperBound;

static inline void BEL_ST_PCSpeakerSetBeepSample(bool isUp)
{
	// Changing between 0 and max. possible value - too loud
	g_sdlCurrentBeepSample = isUp ? MIXER_SAMPLE_PCSPEAKER_TOP_VAL : 0;
}

// Frequency is about 1193182Hz/spkVal
void BE_ST_PCSpeakerSetInvFreq(uint16_t spkInvFreq)
{
	g_sdlPCSpeakerConnected = true;
	BEL_ST_PCSpeakerSetBeepSample(0);
	g_sdlBeepHalfCycleCounter = 0;
	g_sdlBeepHalfCycleCounterUpperBound = g_be_pcSpeakerSampleFreq * spkInvFreq;
}

void BE_ST_PCSpeakerSetConstVal(bool isUp)
{
	g_sdlPCSpeakerConnected = false;
	BEL_ST_PCSpeakerSetBeepSample(isUp);
}

void BE_ST_BSound(uint16_t frequency)
{
	BE_ST_LockAudioRecursively();
	BE_ST_PCSpeakerSetInvFreq(PC_PIT_RATE/(uint32_t)frequency);
	BE_ST_UnlockAudioRecursively();
}

void BE_ST_BNoSound(void)
{
	BE_ST_LockAudioRecursively();
	BE_ST_PCSpeakerSetConstVal(0);
	BE_ST_UnlockAudioRecursively();
}

void BEL_ST_SetPCSpeakerSampleRate(int rate)
{
	g_be_pcSpeakerSampleFreq = rate;
}

/*** PC Speaker: Add audio data to the stream ***/
void BEL_ST_GenPCSpeakerSamples(BE_ST_SndSample_T *stream, int length)
{
	if (!g_sdlPCSpeakerConnected)
	{
		for (int loopVar = 0; loopVar < length; loopVar++, *stream++ = g_sdlCurrentBeepSample)
			;
		return;
	}

	for (int loopVar = 0; loopVar < length; loopVar++, stream++)
	{
		*stream = g_sdlCurrentBeepSample;
		g_sdlBeepHalfCycleCounter += 2 * PC_PIT_RATE;
		if (g_sdlBeepHalfCycleCounter >= g_sdlBeepHalfCycleCounterUpperBound)
		{
			g_sdlBeepHalfCycleCounter %= g_sdlBeepHalfCycleCounterUpperBound;
			BEL_ST_PCSpeakerSetBeepSample(g_sdlCurrentBeepSample < MIXER_SAMPLE_PCSPEAKER_TOP_VAL/2);
		}
	}
}
