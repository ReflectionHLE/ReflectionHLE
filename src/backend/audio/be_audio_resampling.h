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

#ifndef BE_AUDIO_RESAMPLING_H
#define BE_AUDIO_RESAMPLING_H

#include "be_audio_private.h"

#ifdef REFKEEN_RESAMPLER_NONE
// Nothing to add here
#elif (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
#include <libavutil/channel_layout.h>
//#include <libavutil/error.h> // av_err2str requires libavutil/libavutil-ffmpeg
#include <libswresample/swresample.h>
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
#include <libresample.h>
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
#include <soxr.h>
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
#include <speex/speex_resampler.h>
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
#include <samplerate.h>
#else
#error "FATAL ERROR: No recognized Reflection Keen resampler macro is defined!"
#endif

typedef struct
{
	// Nearest-neighborhood sample rate conversion, used as
	// a simplistic alternative to any resampling library
	int *sampleRateConvTable;
	int sampleRateConvTableSize;
	// Current location in the conversion process;
	int sampleRateConvCurrIndex; // Index to sampleRateConvTable
	int sampleRateConvCounter; // Counter for current cell of g_sdlALSampleRateConvTable
	BE_ST_SndSample_T sampleRateConvLastValue; // Last input sample
#ifdef REFKEEN_RESAMPLER_NONE
	// Nothing to add here
#elif (defined REFKEEN_RESAMPLER_LIBSWRESAMPLE)
	struct SwrContext *swrContext;
#elif (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
	void *resampleHandle;
	double resampleFactor;
#elif (defined REFKEEN_RESAMPLER_LIBSOXR)
	soxr_t soxr;
#elif (defined REFKEEN_RESAMPLER_LIBSPEEXDSP)
	SpeexResamplerState *speexResamplerState;
#elif (defined REFKEEN_RESAMPLER_LIBSAMPLERATE)
	SRC_STATE *srcResampler;
	SRC_DATA srcData;
#endif
} BESDLResamplingContext;

void BEL_ST_InitResampling(BESDLResamplingContext *context,
                           int outSampleRate, int inSampleRate,
                           BE_ST_SndSample_T *dataInPtr);

void BEL_ST_ShutdownResampling(BESDLResamplingContext *context);

void BEL_ST_DoResample(BESDLResamplingContext *context,
                       uint32_t *outConsumed, uint32_t *outProduced,
                       BE_ST_SndSample_T *inPtr, BE_ST_SndSample_T *outPtr,
                       uint32_t numOfAvailInputSamples, uint32_t maxSamplesToOutput);

#endif
