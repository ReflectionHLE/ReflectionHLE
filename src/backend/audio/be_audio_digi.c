/* Copyright (C) 2014-2020 NY00123
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
#include "be_audio_mixer.h"
#include <string.h>

static void (*g_sdlDigiAudioIntFuncPtr)(void);

static void *g_sdlSoundEffectCurrPtr;
static uint32_t g_sdlSoundEffectSamplesLeft;
static int g_sdlSoundEffectBits;

// Used for changing input sample rate
static BE_ST_AudioMixerSource *g_sdlDigiMixerSource;

void BE_ST_PlaySoundEffect(void *data, int numOfSamples, int bits)
{
	BE_ST_LockAudioRecursively();

	g_sdlSoundEffectCurrPtr = data;
	g_sdlSoundEffectSamplesLeft = numOfSamples;
	g_sdlSoundEffectBits = bits;

	BE_ST_UnlockAudioRecursively();
}

void BE_ST_StopSoundEffect(void)
{
	BE_ST_LockAudioRecursively();

	g_sdlSoundEffectSamplesLeft = 0;

	BE_ST_UnlockAudioRecursively();
}

// Helper function
#ifdef MIXER_SAMPLE_FORMAT_FLOAT
static void BEL_ST_ConvertU8SamplesToOutputFormat(uint8_t *inPtr, float *outPtr, int samplesToCopy)
{
	for (int i = 0; i < samplesToCopy; ++i, ++inPtr, ++outPtr)
		*outPtr = ((float)(*inPtr - 128))/127.0f;
}
static void BEL_ST_ConvertS16SamplesToOutputFormat(int16_t *inPtr, float *outPtr, int samplesToCopy)
{
	for (int i = 0; i < samplesToCopy; ++i, ++inPtr, ++outPtr)
		*outPtr = ((float)(*inPtr))/32767.0f;
}
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
static void BEL_ST_ConvertU8SamplesToOutputFormat(uint8_t *inPtr, int16_t *outPtr, int samplesToCopy)
{
	for (int i = 0; i < samplesToCopy; ++i, ++inPtr, ++outPtr)
		*outPtr = (*inPtr - 128)*256;
}
static void BEL_ST_ConvertS16SamplesToOutputFormat(int16_t *inPtr, int16_t *outPtr, int samplesToCopy)
{
	memcpy(outPtr, inPtr, 2*samplesToCopy);
}
#endif

void BE_ST_StartDigiAudioInt(void (*funcPtr)(void))
{
	BE_ST_LockAudioRecursively();
	g_sdlDigiAudioIntFuncPtr = funcPtr;
	BE_ST_UnlockAudioRecursively();
}

void BE_ST_StopDigiAudioInt(void)
{
	BE_ST_LockAudioRecursively();
	g_sdlDigiAudioIntFuncPtr = 0;
	BE_ST_UnlockAudioRecursively();
}

void BEL_ST_SetDigiMixerSource(BE_ST_AudioMixerSource *src)
{
	g_sdlDigiMixerSource = src;
}

void BE_ST_SetDigiSoundFreq(int freq)
{
	BE_ST_AudioMixerSetSourceFreq(g_sdlDigiMixerSource, freq);
}

void BEL_ST_GenDigiSamples(BE_ST_SndSample_T *stream, int length)
{
	if ((uint32_t)length >= g_sdlSoundEffectSamplesLeft)
	{
		memset(stream + g_sdlSoundEffectSamplesLeft, 0, sizeof(BE_ST_SndSample_T) * (length - g_sdlSoundEffectSamplesLeft));
		if (g_sdlSoundEffectSamplesLeft > 0)
		{
			if (g_sdlSoundEffectBits == 16)
				BEL_ST_ConvertS16SamplesToOutputFormat((int16_t *)g_sdlSoundEffectCurrPtr, stream, g_sdlSoundEffectSamplesLeft);
			else
				BEL_ST_ConvertU8SamplesToOutputFormat((uint8_t *)g_sdlSoundEffectCurrPtr, stream, g_sdlSoundEffectSamplesLeft);
			g_sdlSoundEffectSamplesLeft = 0;
			if (g_sdlDigiAudioIntFuncPtr)
				g_sdlDigiAudioIntFuncPtr();
		}
	}
	else
	{
		if (g_sdlSoundEffectBits == 16)
		{
			BEL_ST_ConvertS16SamplesToOutputFormat((int16_t *)g_sdlSoundEffectCurrPtr, stream, length);
			g_sdlSoundEffectCurrPtr = (int16_t *)g_sdlSoundEffectCurrPtr + length;
		}
		else
		{
			BEL_ST_ConvertU8SamplesToOutputFormat((uint8_t *)g_sdlSoundEffectCurrPtr, stream, length);
			g_sdlSoundEffectCurrPtr = (uint8_t *)g_sdlSoundEffectCurrPtr + length;
		}
		g_sdlSoundEffectSamplesLeft -= length;
	}
}
