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

#ifndef BE_AUDIO_MIXER
#define BE_AUDIO_MIXER

#include "be_audio_resampling.h"

typedef struct BE_ST_AudioMixerSource
{
	void (*genSamples)(BE_ST_SndSample_T *stream, int length);
	struct
	{
		BE_ST_SndSample_T *buffer;
		int size, num;
	} in, out;
	BESDLResamplingContext resamplingContext;
	int numScaledSamplesToGenNextTime;
	int freq;
	float vol[2];
	bool skip;
} BE_ST_AudioMixerSource;

void BEL_ST_AudioMixerInit(int freq, int channels);
void BEL_ST_AudioMixerShutdown(void);
void BEL_ST_AudioMixerCallback(BE_ST_SndSample_T *stream, int len);
void BEL_ST_AudioMixerUpdateFromPITRateWord(int_fast32_t rateVal);

// Returns a pointer to a new source, which is invalidated after mixer shutdown
BE_ST_AudioMixerSource *BEL_ST_AudioMixerAddSource(
	int freq, int maxNumOfOutSamples,
	void (*genSamples)(BE_ST_SndSample_T *stream, int len));

void BE_ST_AudioMixerSetSourceFreq(BE_ST_AudioMixerSource *src, int freq);

#endif
