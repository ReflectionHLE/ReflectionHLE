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
#include "backend/timing/be_timing.h"
#include "be_audio_mixer.h"
#include "be_audio_resampling.h"
#include "be_st_sdl_private.h" // For BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO
#include <stdlib.h>
#include <string.h>

enum { BE_ST_AUDIO_MIXER_MAX_SOURCES = 4 };

static struct
{
	BE_ST_AudioMixerSource sources[BE_ST_AUDIO_MIXER_MAX_SOURCES];
	uint64_t samplesPerPartTimesPITRate;
	uint32_t samplesInCurrentPart;
	uint32_t samplesPartNum;
	uint32_t pendingSamples;
	uint32_t offsetInSound;
	int numSources;
	int freq;
} g_stAudioMixer;

static void BEL_ST_AudioMixerFreeSourceBuffers(BE_ST_AudioMixerSource *src)
{
	if (g_stAudioMixer.freq != src->freq)
		BEL_ST_ShutdownResampling(&src->resamplingContext);
	free(src->out.buffer);
	free(src->in.buffer);
}

void BE_ST_AudioMixerSetSourceFreq(BE_ST_AudioMixerSource *src, int freq)
{
	BE_ST_LockAudioRecursively();
	src->skip = true;
	BE_ST_UnlockAudioRecursively();

	if (src->in.buffer)
		BEL_ST_AudioMixerFreeSourceBuffers(src);

	src->in.num = 0;
	src->out.num = 0;
	src->numScaledSamplesToGenNextTime = 0;
	// src->out.size shall not change, and should've been set earlier
	src->in.size = src->out.size * freq / g_stAudioMixer.freq;
	src->freq = freq;

	src->in.buffer = (BE_ST_SndSample_T *)malloc(src->in.size * sizeof(BE_ST_SndSample_T));
	if (!src->in.buffer)
		BE_ST_ExitWithErrorMsg("BEL_ST_AudioMixerAddSource: Out of memory!");

	if (g_stAudioMixer.freq != freq)
		BEL_ST_InitResampling(&src->resamplingContext,
		                      g_stAudioMixer.freq, freq, src->in.buffer);

	src->out.buffer = (BE_ST_SndSample_T *)malloc(src->out.size * sizeof(BE_ST_SndSample_T));
	if (!src->out.buffer)
		BE_ST_ExitWithErrorMsg("BEL_ST_AudioMixerAddSource: Out of memory!");
finish:
	BE_ST_LockAudioRecursively();
	src->skip = false;
	BE_ST_UnlockAudioRecursively();
}

void BEL_ST_AudioMixerInit(int freq)
{
	g_stAudioMixer.offsetInSound = 0;
	g_stAudioMixer.samplesPartNum = 0;
	g_stAudioMixer.numSources = 0;
	g_stAudioMixer.freq = freq;
}

void BEL_ST_AudioMixerShutdown(void)
{
	for (int i = 0; i < g_stAudioMixer.numSources; ++i)
		BEL_ST_AudioMixerFreeSourceBuffers(&g_stAudioMixer.sources[i]);
}

void BEL_ST_AudioMixerUpdateFromPITRateWord(int_fast32_t rateVal)
{
	g_stAudioMixer.samplesPerPartTimesPITRate = rateVal * g_stAudioMixer.freq;
	// Since the following division may lead to truncation,
	// samplesInCurrentPart can change during playback by +-1
	// (otherwise music may be a bit faster than intended).
	g_stAudioMixer.samplesInCurrentPart = g_stAudioMixer.samplesPerPartTimesPITRate / PC_PIT_RATE;
}

BE_ST_AudioMixerSource *BEL_ST_AudioMixerAddSource(
	int freq, int maxNumOfOutSamples,
	void (*genSamples)(BE_ST_SndSample_T *stream, int len))
{
	if (g_stAudioMixer.numSources == BE_ST_AUDIO_MIXER_MAX_SOURCES)
		BE_ST_ExitWithErrorMsg("BEL_ST_AudioMixerAddSource: Can't add another source!");
	BE_ST_AudioMixerSource *src = &g_stAudioMixer.sources[g_stAudioMixer.numSources++];

	src->genSamples = genSamples;
	src->out.size = maxNumOfOutSamples;
	src->in.buffer = src->out.buffer = 0;

	BE_ST_AudioMixerSetSourceFreq(src, freq);
	return src;
}

void BEL_ST_AudioMixerCallback(BE_ST_SndSample_T *stream, int len)
{
	int samplesToGenerate = len;
	int samplesToGenerateNextTime = g_stAudioMixer.freq / 100; // ~10ms
	int i, j;
	BE_ST_AudioMixerSource *src;

	BE_ST_LockAudioRecursively();

	while (len)
	{
		uint32_t processedInputSamples = g_stAudioMixer.pendingSamples;
		samplesToGenerate -= processedInputSamples;
		while (samplesToGenerate)
		{
			// Optionally make an inner callback back, possibly in game code
			if (!g_stAudioMixer.offsetInSound && g_sdlTimerIntFuncPtr)
			{
				g_sdlTimerIntFuncPtr();
#ifndef BE_ST_MANAGE_INT_CALLS_SEPARATELY_FROM_AUDIO
				BE_ST_SET_TIMER_INT_COUNTER_INC();
#endif
			}
			// Now generate sound
			uint32_t currNumOfSamples =
				BE_Cross_TypedMin(
					uint32_t,
					samplesToGenerate,
					g_stAudioMixer.samplesInCurrentPart-g_stAudioMixer.offsetInSound);
			processedInputSamples += currNumOfSamples;
			g_stAudioMixer.pendingSamples = processedInputSamples;
			for (i = 0; i < g_stAudioMixer.numSources; ++i)
			{
				src = &g_stAudioMixer.sources[i];
				if (src->skip)
					continue;
				uint32_t samplesToGen = processedInputSamples;
				samplesToGen = samplesToGen * src->freq + src->numScaledSamplesToGenNextTime;
				src->numScaledSamplesToGenNextTime = samplesToGen % g_stAudioMixer.freq;
				samplesToGen /= g_stAudioMixer.freq;
				if (samplesToGen > src->in.size)
				{
					BE_Cross_LogMessage(
						BE_LOG_MSG_NORMAL,
						"BEL_ST_AudioMixerCallback: Overflow for source %d; Want %u, have %u\n",
						i, samplesToGen, src->in.size);
					samplesToGen = src->in.size;
				}
				if (samplesToGen > src->in.num)
				{
					src->genSamples(&src->in.buffer[src->in.num],
					                samplesToGen - src->in.num);
					src->in.num = samplesToGen;
				}
			}
			// We're done with current part for now
			g_stAudioMixer.offsetInSound += currNumOfSamples;
			samplesToGenerate -= currNumOfSamples;
			// End of part?
			if (g_stAudioMixer.offsetInSound >= g_stAudioMixer.samplesInCurrentPart)
			{
				g_stAudioMixer.offsetInSound = 0;
				if (++g_stAudioMixer.samplesPartNum == PC_PIT_RATE)
					g_stAudioMixer.samplesPartNum = 0;
				g_stAudioMixer.samplesInCurrentPart =
					(g_stAudioMixer.samplesPartNum + 1) * g_stAudioMixer.samplesPerPartTimesPITRate / PC_PIT_RATE -
					g_stAudioMixer.samplesPartNum * g_stAudioMixer.samplesPerPartTimesPITRate / PC_PIT_RATE;
			}
		}

		// Try to resample the data that we have
		uint32_t samplesToOutput = len;
		for (i = 0; i < g_stAudioMixer.numSources; ++i)
		{
			src = &g_stAudioMixer.sources[i];
			if (src->skip)
				continue;
			uint32_t consumed, produced;
			uint32_t maxSamplesToOutput = BE_Cross_TypedMin(
				uint32_t, len,
				src->out.size - src->out.num);
			if (g_stAudioMixer.freq != src->freq)
				BEL_ST_DoResample(&src->resamplingContext,
				                  &consumed, &produced,
				                  src->in.buffer,
				                  &src->out.buffer[src->out.num],
				                  src->in.num, maxSamplesToOutput);
			else
			{
				consumed = produced = BE_Cross_TypedMin(
					uint32_t, maxSamplesToOutput, src->in.num);
				memcpy(&src->out.buffer[src->out.num], src->in.buffer,
				       sizeof(BE_ST_SndSample_T) * consumed);
			}
			if ((consumed > 0) && (consumed < src->in.num))
				memmove(src->in.buffer, &src->in.buffer[consumed],
				        sizeof(BE_ST_SndSample_T) * (src->in.num - consumed));

			src->in.num -= consumed;
			src->out.num += produced;

			samplesToOutput = BE_Cross_TypedMin(uint32_t, samplesToOutput, src->out.num);
		}

		// Mix
		if (samplesToOutput > 0)
		{
			memset(stream, 0, sizeof(BE_ST_SndSample_T) * samplesToOutput);
			for (i = 0; i < g_stAudioMixer.numSources; ++i)
			{
				src = &g_stAudioMixer.sources[i];
				if (src->skip)
					continue;
				BE_ST_SndSample_T *ptr = stream;
				for (j = 0; j < samplesToOutput; ++j, ++ptr)
					*ptr = (i * (*ptr) + src->out.buffer[j]) / (i + 1);
				if (samplesToOutput < src->out.num)
				{
					memmove(src->out.buffer,
					        &src->out.buffer[samplesToOutput],
					        sizeof(BE_ST_SndSample_T) * (src->out.num - samplesToOutput));
					src->out.num -= samplesToOutput;
				}
				else
					src->out.num = 0;
			}
			stream += samplesToOutput;
			len -= samplesToOutput;
			if (g_stAudioMixer.pendingSamples < samplesToOutput)
				g_stAudioMixer.pendingSamples = 0;
			else
				g_stAudioMixer.pendingSamples -= samplesToOutput;
		}
		else
		{
			// Resampling may add some latency (audible or not),
			// so generate a few more input samples if required.
			// In case we're stuck with no change,
			// again we should try to generate more samples.
			samplesToGenerateNextTime += (uint64_t)g_stAudioMixer.freq / 1000; // ~1ms
		}
		samplesToGenerate = samplesToGenerateNextTime;
	}
	BE_ST_UnlockAudioRecursively();
}
